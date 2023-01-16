#include "directx_12.h"
#include "debug.h"
#include "dx_util.h"
#include "ReadData.h"

using namespace Microsoft::WRL;
using namespace DirectX::SimpleMath;

namespace fuse::directx {
    void directx_12::init(const window_info &info) {
        {
            ComPtr<ID3D12Debug> debugController;
            if (SUCCEEDED(
                    D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
                debugController->EnableDebugLayer();
            }
        }

        init_base(info);
        init_cmds();
        init_swap_chain(info);
        init_rtv(info);
        init_dsv(info);

        init_global_buf();
        init_camera_buf();
        init_light_buf();
        init_resources();
        _blur.init(_device, info.width, info.height);
        _shadow_map.init(_device, info.width, info.height, shadow_handle());

        init_root_signature();
        init_shader();

        //window resize;
        RECT rect = {0, 0, info.width, info.height};
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
        SetWindowPos(info.hwnd, 0, 100, 100, info.width, info.height, 0);
    }

    void directx_12::set_main_camera(std::shared_ptr<camera> p) {
        _main_camera = std::move(p);
    }

    void directx_12::update_camera(const camera_buf &cam) {
        update_const_buffer<camera_buf>(_vp_buffer, &cam, 0);
    }

    void directx_12::update_lights(const light_info &info) {
        update_const_buffer<light_info>(_light_buffer, &info, 0);
    }

    void
    directx_12::init_renderees(std::vector<std::shared_ptr<renderee>> vec) {
        _renderees.clear();
        _renderees.resize(static_cast<uint8_t>(renderee_type::count));

        int id = 0;
        for (auto e: vec) {
            e->id = id;
            _renderees[static_cast<uint8_t>(e->type)].emplace_back(e);
            ++id;
        }

        for (auto &typed_renderees: _renderees) {
            for (auto &ptr: typed_renderees) {
                uint32_t tid;
                switch (ptr->type) {
                    case renderee_type::opaque:
                        tid = type_id<vertex>();
                        break;
                    case renderee_type::translucent:
                        tid = type_id<vertex>();
                        break;
                    case renderee_type::billboard:
                        tid = type_id<vertex_billboard>();
                        break;
                    case renderee_type::terrain:
                        tid = type_id<vertex>();
                        break;
                    case renderee_type::count:
                        //do nothing
                        break;
                    case renderee_type::skybox:
                        tid = type_id<vertex>();
                        break;
                    case renderee_type::opaque_skinned:
                        tid = type_id<vertex>();
                        break;
                }
                if (_geo_infos[tid].count(ptr->geometry) < 1) {
                    tid = tid;
                }
                ptr->geo = _geo_infos[tid][ptr->geometry];

                for (auto i = 0; i < _countof(ptr->texture); ++i) {
                    if (!ptr->texture[i].empty())
                        bind_texture(ptr->id, ptr->texture[i], i);
                }

                if (!ptr->material.empty()) ptr->constants.mat_id = _mat_ids[ptr->material];
                ptr->constants.position = ptr->tr.position;
                ptr->constants.world_matrix = ptr->tr.world_matrix();

                update_const_buffer(_obj_const_buffer, &(ptr->constants),
                                    ptr->id);

                if (ptr->type == renderee_type::opaque_skinned)
                    update_const_buffer(_skin_metrics_buf,
                                        &(ptr->skin_matrices), ptr->id);
            }
        }
    }

    void directx_12::render() {
        _cmd_list->IASetVertexBuffers(0, 1,
                                      &(_vertex_buffers[type_id<vertex_billboard>()].second));
        _cmd_list->IASetIndexBuffer(
                &(_index_buffers[type_id<vertex_billboard>()].second));
        _cmd_list->IASetPrimitiveTopology(
                D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        _cmd_list->SetPipelineState(
                _pso_list[static_cast<uint8_t>(layer::billboard)].Get());

        for (const auto &e: _renderees[static_cast<uint8_t>(renderee_type::billboard)]) {
            render(e);
        }

        _cmd_list->IASetVertexBuffers(0, 1,
                                      &(_vertex_buffers[type_id<vertex>()].second));
        _cmd_list->IASetIndexBuffer(
                &(_index_buffers[type_id<vertex>()].second));
        _cmd_list->IASetPrimitiveTopology(
                D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
        _cmd_list->SetPipelineState(
                _pso_list[static_cast<uint8_t>(layer::terrain)].Get());

        for (const auto &e: _renderees[static_cast<uint8_t>(renderee_type::terrain)]) {
            render(e);
        }


        _cmd_list->IASetPrimitiveTopology(
                D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        _cmd_list->SetPipelineState(
                _pso_list[static_cast<uint8_t>(layer::skybox)].Get());
        for (const auto &e: _renderees[static_cast<uint8_t>(renderee_type::skybox)]) {
            render(e);
        }

        _cmd_list->SetPipelineState(
                _pso_list[static_cast<uint8_t>(layer::opaque)].Get());

        for (const auto &e: _renderees[static_cast<uint8_t>(renderee_type::opaque)]) {
            render(e);
        }

        _cmd_list->SetPipelineState(
                _pso_list[static_cast<uint8_t>(layer::skinned)].Get());

        for (const auto &e: _renderees[static_cast<uint8_t>(renderee_type::opaque_skinned)]) {
            render(e);
        }

        _cmd_list->SetPipelineState(
                _pso_list[static_cast<uint8_t>(layer::transparent)].Get());

        for (const auto &e: _renderees[static_cast<uint8_t>(renderee_type::translucent)]) {
            render(e);
        }
    }

    void directx_12::load_texture(const std::string &name,
                                  const std::wstring &path) {
        ThrowIfFailed(_cmd_alloc->Reset());
        ThrowIfFailed(_cmd_list->Reset(_cmd_alloc.Get(), nullptr));

        DirectX::ScratchImage image;
        ComPtr<ID3D12Resource> ub;
        ComPtr<ID3D12Resource> buf = load_texture(path, image, ub);

        _cmd_list->Close();
        execute_cmd_list();
        wait_cmd_queue_sync();

        D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
        auto &meta = image.GetMetadata();
        desc.Format = meta.format;

        if (meta.IsCubemap()) {
            desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            desc.TextureCube.MipLevels = meta.mipLevels;
            desc.TextureCube.MostDetailedMip = 0;

            auto handle = _res_desc_heap->GetCPUDescriptorHandleForHeapStart();
            handle.ptr += group_size() * OBJ_CNT;
            _device->CreateShaderResourceView(buf.Get(), &desc, handle);
        } else if (meta.arraySize == 1) {
            desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
            desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            desc.Texture2D.MipLevels = meta.mipLevels;
            desc.Texture2D.MostDetailedMip = 0;
        } else {
            desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            desc.Texture2DArray.MipLevels = meta.mipLevels;
            desc.Texture2DArray.MostDetailedMip = 0;
            desc.Texture2DArray.ArraySize = meta.arraySize;
        }


        _textures[name] = std::make_pair(desc, buf);
    }

    ComPtr<ID3D12Resource> directx_12::load_texture(const std::wstring &path,
                                                    DirectX::ScratchImage &image,
                                                    ComPtr<ID3D12Resource> &upload_buffer) {
        auto ext = std::filesystem::path(path).extension();

        if (ext == L".dds" || ext == L".DDS") {
            ThrowIfFailed(LoadFromDDSFile(path.c_str(), DirectX::DDS_FLAGS_NONE,
                                          nullptr, image))
        } else if (ext == L".tga" || ext == L".TGA") {
            ThrowIfFailed(
                    LoadFromTGAFile(path.c_str(), nullptr, image))
        } else {
            ThrowIfFailed(
                    LoadFromWICFile(path.c_str(), DirectX::WIC_FLAGS_NONE,
                                    nullptr, image))
        }

        ComPtr<ID3D12Resource> tmp;
        ThrowIfFailed(
                DirectX::CreateTexture(_device.Get(), image.GetMetadata(),
                                       &tmp));

        std::vector<D3D12_SUBRESOURCE_DATA> sub_reses;
        ThrowIfFailed(DirectX::PrepareUpload(_device.Get(), image.GetImages(),
                                             image.GetImageCount(),
                                             image.GetMetadata(), sub_reses));

        upload_buffer = create_upload_buffer(1, GetRequiredIntermediateSize(
                                                     tmp.Get(), 0, static_cast<uint32_t>(sub_reses.size())),
                                             _device);
        UpdateSubresources(_cmd_list.Get(), tmp.Get(), upload_buffer.Get(),
                           0, 0, static_cast<uint32_t>(sub_reses.size()),
                           sub_reses.data());
        return tmp;
    }

    void
    directx_12::bind_texture(int obj, const std::string &texture, int regi) {
        auto handle = _res_desc_heap->GetCPUDescriptorHandleForHeapStart();
        auto handle_sz = _device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        handle.ptr += obj * group_size();
        handle.ptr += handle_sz * (regi + 2);
        auto texture_res = _textures[texture].second;
        auto desc = _textures[texture].first;
        _device->CreateShaderResourceView(texture_res.Get(), &desc, handle);
    }

    void directx_12::load_material(const std::vector<std::string> &names,
                                   const std::vector<material> &mat) {
        for (auto i = 0; i < names.size(); ++i) {
            update_upload_buffer(_mat_buffer, &(mat[i]), i, sizeof(material));
            _mat_ids[names[i]] = i;
        }
    }

    void directx_12::render_begin() {
        camera_buf cam_buf;
        cam_buf.position = _main_camera->tr.position;
        cam_buf.vp = _main_camera->view() * _main_camera->projection();
        update_const_buffer<camera_buf>(_vp_buffer, &cam_buf, 0);

        ThrowIfFailed(_cmd_alloc->Reset())
        ThrowIfFailed(_cmd_list->Reset(_cmd_alloc.Get(), nullptr))
        _cmd_list->SetGraphicsRootSignature(
                _signatures[shader_type::general].Get());
        _cmd_list->SetGraphicsRootConstantBufferView(0,
                                                     _global_buffer->GetGPUVirtualAddress());
        _cmd_list->SetGraphicsRootConstantBufferView(1,
                                                     _vp_buffer->GetGPUVirtualAddress());
        _cmd_list->SetGraphicsRootConstantBufferView(2,
                                                     _light_buffer->GetGPUVirtualAddress());
        _cmd_list->SetGraphicsRootShaderResourceView(3,
                                                     _mat_buffer->GetGPUVirtualAddress());

        ID3D12DescriptorHeap *heaps[] = {_res_desc_heap.Get()};
        _cmd_list->SetDescriptorHeaps(_countof(heaps), heaps);
        auto skybox_handle = _res_desc_heap->GetGPUDescriptorHandleForHeapStart();
        skybox_handle.ptr += group_size() * OBJ_CNT;
        _cmd_list->SetGraphicsRootDescriptorTable(4, skybox_handle);

        {//draw shadow map
            _cmd_list->RSSetViewports(1, &(_shadow_map.viewport));
            _cmd_list->RSSetScissorRects(1, &(_shadow_map.scissors_rect));

            auto b0 = CD3DX12_RESOURCE_BARRIER::Transition(
                    _shadow_map.depth_map.Get(),
                    D3D12_RESOURCE_STATE_GENERIC_READ,
                    D3D12_RESOURCE_STATE_DEPTH_WRITE);
            _cmd_list->ResourceBarrier(1, &b0);

            _cmd_list->ClearDepthStencilView(
                    _shadow_map.dsv_heap->GetCPUDescriptorHandleForHeapStart(),
                    D3D12_CLEAR_FLAG_DEPTH, 1.f, 0, 0, nullptr);
            auto dsv = _shadow_map.dsv_heap->GetCPUDescriptorHandleForHeapStart();
            _cmd_list->IASetPrimitiveTopology(
                    D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            _cmd_list->IASetVertexBuffers(0, 1,
                                          &(_vertex_buffers[type_id<vertex>()].second));
            _cmd_list->IASetIndexBuffer(
                    &(_index_buffers[type_id<vertex>()].second));
            _cmd_list->OMSetRenderTargets(0, nullptr, false, &dsv);
            Vector3 center = Vector3::Zero;
            Vector3 light_vec = Vector3(0.f, -1.f, 1.f);
            light_vec.Normalize();
            camera light_cam;
            light_cam.tr.position = center + (-light_vec * 2) * 10.f;
            light_cam.tr.rotation.x = DirectX::XM_PI / 4.f;
            auto view = light_cam.view();
            auto proj = DirectX::XMMatrixOrthographicLH(80.f, 80.f, 1.f, 100.f);
            Matrix ndc_to_uv = {.5f, .0f, .0f, .0f,
                                .0f, -.5f, .0f, .0f,
                                .0f, .0f, 1.f, 0.f,
                                .5f, .5f, .0f, 1.f};
            _global.light_vp = view * proj;
            _global.shadow_uv_matrix = _global.light_vp * ndc_to_uv;
            update_const_buffer(_global_buffer, &_global, 0);
            _cmd_list->SetPipelineState(
                    _pso_list[static_cast<uint8_t>(layer::dynamic_shadow)].Get());
            for (const auto &e: _renderees[static_cast<uint8_t>(renderee_type::opaque)]) {
                render(e);
            }

            for (const auto &e: _renderees[static_cast<uint8_t>(renderee_type::opaque_skinned)]) {
                render(e);
            }

            for (const auto &e: _renderees[static_cast<uint8_t>(renderee_type::translucent)]) {
                render(e);
            }

            auto b1 = CD3DX12_RESOURCE_BARRIER::Transition(
                    _shadow_map.depth_map.Get(),
                    D3D12_RESOURCE_STATE_DEPTH_WRITE,
                    D3D12_RESOURCE_STATE_GENERIC_READ);
            _cmd_list->ResourceBarrier(1, &b1);
        }

        _cmd_list->RSSetViewports(1, &_view_port);
        _cmd_list->RSSetScissorRects(1, &_scissors_rect);

        auto barrier0 = CD3DX12_RESOURCE_BARRIER::Transition(
                _rtv_buffer[_back_buffer].Get(), D3D12_RESOURCE_STATE_PRESENT,
                D3D12_RESOURCE_STATE_RESOLVE_DEST);
        _cmd_list->ResourceBarrier(1, &barrier0);

        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
                _msaa_render_buffer.Get(), D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
                D3D12_RESOURCE_STATE_RENDER_TARGET);
        _cmd_list->ResourceBarrier(1, &barrier1);


        auto msaa_handle = _msaa_render_buffer_heap->GetCPUDescriptorHandleForHeapStart();
        _cmd_list->ClearRenderTargetView(msaa_handle,
                                         DirectX::Colors::Aqua, 0, nullptr);
        _cmd_list->ClearDepthStencilView(_dsv_handle, D3D12_CLEAR_FLAG_DEPTH |
                                                      D3D12_CLEAR_FLAG_STENCIL,
                                         1.f, 0, 0, nullptr);
        _cmd_list->OMSetRenderTargets(1, &msaa_handle, FALSE,
                                      &_dsv_handle);
    }

    void directx_12::render_end() {
        auto barrier1 = CD3DX12_RESOURCE_BARRIER::Transition(
                _msaa_render_buffer.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET,
                D3D12_RESOURCE_STATE_RESOLVE_SOURCE);
        _cmd_list->ResourceBarrier(1, &barrier1);

        _cmd_list->ResolveSubresource(_rtv_buffer[_back_buffer].Get(), 0,
                                      _msaa_render_buffer.Get(), 0,
                                      RTV_FORMAT);

//        _blur.blur_texture(_cmd_list, _rtv_buffer[_back_buffer],
//                           _pso_list[static_cast<uint8_t>(layer::blur_h)],
//                           _pso_list[static_cast<uint8_t>(layer::blur_v)],
//                           _signatures[shader_type::blur]);

        auto barrier0 = CD3DX12_RESOURCE_BARRIER::Transition(
                _rtv_buffer[_back_buffer].Get(),
                D3D12_RESOURCE_STATE_RESOLVE_DEST,
                D3D12_RESOURCE_STATE_PRESENT);

        _cmd_list->ResourceBarrier(1, &barrier0);
        ThrowIfFailed(_cmd_list->Close())

        execute_cmd_list();
        _swap_chain->Present(0, 0);
        _back_buffer = (_back_buffer + 1) % SWAP_CHAIN_BUFFER_COUNT;

        wait_cmd_queue_sync();
    }

    void directx_12::render(const std::vector<render_info> &infos) {
        _cmd_list->IASetVertexBuffers(0, 1,
                                      &(_vertex_buffers[type_id<vertex_billboard>()].second));
        _cmd_list->IASetIndexBuffer(
                &(_index_buffers[type_id<vertex_billboard>()].second));
        _cmd_list->IASetPrimitiveTopology(
                D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
        _cmd_list->SetPipelineState(
                _pso_list[static_cast<uint8_t>(layer::billboard)].Get());

        for (const auto &e: infos) {
            if (e.is_billboard) {
                render(e);
            }
        }

        _cmd_list->IASetVertexBuffers(0, 1,
                                      &(_vertex_buffers[type_id<vertex>()].second));
        _cmd_list->IASetIndexBuffer(
                &(_index_buffers[type_id<vertex>()].second));
        _cmd_list->IASetPrimitiveTopology(
                D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
        _cmd_list->SetPipelineState(
                _pso_list[static_cast<uint8_t>(layer::terrain)].Get());

        for (const auto &e: infos) {
            if (e.is_terrain) render(e);
        }

        _cmd_list->IASetPrimitiveTopology(
                D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        std::vector<size_t> mirrors;
        std::vector<size_t> trans;
        std::vector<size_t> reflects;
        std::vector<size_t> shadows;

        _cmd_list->SetPipelineState(
                _pso_list[static_cast<uint8_t>(layer::opaque)].Get());
        for (auto i = 0; i < infos.size(); ++i) {
            if (infos[i].is_billboard || infos[i].is_terrain) continue;

            if (infos[i].do_reflect) {
                reflects.push_back(i);
            }

            if (infos[i].do_shadow) {
                shadows.push_back(i);
            }

            if (infos[i].is_mirror) {
                mirrors.push_back(i);
                trans.push_back(i);
            } else if (infos[i].is_transparent) {
                trans.push_back(i);
            } else {
                render(infos[i]);
            }
        }

        if (mirrors.size() > 0 && reflects.size() > 0) {
            _cmd_list->OMSetStencilRef(1);
            _cmd_list->SetPipelineState(
                    _pso_list[static_cast<uint8_t>(layer::mirror)].Get());

            int k = 0;
            for (auto i: mirrors) {
                render(infos[i]);
                _global.reflection_matrix[k]
                        = Matrix::CreateReflection(infos[i].mirror_plane);
                ++k;
            }
            _global.reflection_count = mirrors.size();
            update_const_buffer(_global_buffer, &_global, 0);


            _cmd_list->SetPipelineState(
                    _pso_list[static_cast<uint8_t>(layer::reflection)].Get());

            for (auto j: reflects) {
                render(infos[j]);
            }

            _cmd_list->OMSetStencilRef(0);
        }

        if (trans.size() > 0) {
            _cmd_list->SetPipelineState(
                    _pso_list[static_cast<uint8_t>(layer::transparent)].Get());
            for (auto i: trans) render(infos[i]);
        }

        if (shadows.size() > 0) {
            _cmd_list->OMSetStencilRef(0);
            _cmd_list->SetPipelineState(
                    _pso_list[static_cast<uint8_t>(layer::shadow)].Get());
            for (auto i: shadows) render(infos[i]);
        }
    }

    void directx_12::render(const render_info &info) {
        auto handle = _res_desc_heap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += group_size() * info.object_index;
        _cmd_list->SetGraphicsRootDescriptorTable(4, handle);

        _cmd_list->DrawIndexedInstanced(info.index_count, 1, info.index_offset,
                                        info.vertex_offset, 0);
    }

    void directx_12::render(const std::shared_ptr<renderee> &r) {
        auto handle = _res_desc_heap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += group_size() * r->id;
        _cmd_list->SetGraphicsRootDescriptorTable(5, handle);

        _cmd_list->DrawIndexedInstanced(r->geo.index_count, 1,
                                        r->geo.index_offset,
                                        r->geo.vertex_offset, 0);
    }

    void directx_12::init_base(const window_info &info) {
        CreateDXGIFactory1(IID_PPV_ARGS(&_factory));
        D3D12CreateDevice(nullptr, D3D_FEATURE_LEVEL_11_0,
                          IID_PPV_ARGS(&_device));
        _view_port = {0, 0, static_cast<float>(info.width),
                      static_cast<float>(info.height), 0, 1};
        _scissors_rect = CD3DX12_RECT{0, 0, info.width, info.height};

        D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS quality_levels;
        quality_levels.Format = RTV_FORMAT;
        quality_levels.SampleCount = msaa_sample_count;
        quality_levels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
        quality_levels.NumQualityLevels = 0;
        ThrowIfFailed(_device->CheckFeatureSupport(
                D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
                &quality_levels, sizeof(quality_levels)));
        msaa_quality_levels = quality_levels.NumQualityLevels;
    }

    void directx_12::init_cmds() {
        D3D12_COMMAND_QUEUE_DESC queue_desc = {D3D12_COMMAND_LIST_TYPE_DIRECT,
                                               0, D3D12_COMMAND_QUEUE_FLAG_NONE,
                                               0};
        _device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&_cmd_queue));
        _device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
                                        IID_PPV_ARGS(&_cmd_alloc));
        _device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                                   _cmd_alloc.Get(), nullptr,
                                   IID_PPV_ARGS(&_cmd_list));
        _cmd_list->Close();
        _device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&_fence));
    }

    void directx_12::init_swap_chain(const window_info &info) {
        _swap_chain.Reset();
        DXGI_SWAP_CHAIN_DESC swap_desc;
        swap_desc.BufferDesc.Width = static_cast<uint32_t>(info.width);
        swap_desc.BufferDesc.Height = static_cast<uint32_t>(info.height);
        swap_desc.BufferDesc.RefreshRate.Numerator = 60;
        swap_desc.BufferDesc.RefreshRate.Denominator = 1;
        swap_desc.BufferDesc.Format = RTV_FORMAT;
        swap_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        swap_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        swap_desc.SampleDesc.Count = 1;
        swap_desc.SampleDesc.Quality = 0;
        swap_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swap_desc.BufferCount = SWAP_CHAIN_BUFFER_COUNT;
        swap_desc.OutputWindow = info.hwnd;
        swap_desc.Windowed = info.windowed;
        swap_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swap_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
        ThrowIfFailed(_factory->CreateSwapChain(_cmd_queue.Get(), &swap_desc,
                                                &_swap_chain))
    }

    void directx_12::init_rtv(const window_info &info) {
        auto rtv_heap_size = _device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
        D3D12_DESCRIPTOR_HEAP_DESC rtv_dh_desc;
        rtv_dh_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtv_dh_desc.NumDescriptors = SWAP_CHAIN_BUFFER_COUNT;
        rtv_dh_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtv_dh_desc.NodeMask = 0;
        _device->CreateDescriptorHeap(&rtv_dh_desc, IID_PPV_ARGS(&_rtv_heap));
        auto rtv_dh_begin = _rtv_heap->GetCPUDescriptorHandleForHeapStart();

        for (auto i = 0; i < SWAP_CHAIN_BUFFER_COUNT; ++i) {
            _rtv_handle[i] = CD3DX12_CPU_DESCRIPTOR_HANDLE(rtv_dh_begin,
                                                           rtv_heap_size * i);
            _swap_chain->GetBuffer(i, IID_PPV_ARGS(&_rtv_buffer[i]));
            _device->CreateRenderTargetView(_rtv_buffer[i].Get(), nullptr,
                                            _rtv_handle[i]);
        }

        D3D12_DESCRIPTOR_HEAP_DESC msaa_desc_heap_desc;
        msaa_desc_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        msaa_desc_heap_desc.NumDescriptors = 1;
        msaa_desc_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        msaa_desc_heap_desc.NodeMask = 0;
        ThrowIfFailed(_device->CreateDescriptorHeap(&msaa_desc_heap_desc,
                                                    IID_PPV_ARGS(
                                                            &_msaa_render_buffer_heap)));

        auto msaa_buf_desc = CD3DX12_RESOURCE_DESC::Tex2D(
                RTV_FORMAT,
                info.width, info.height, 1,
                1, msaa_sample_count,
                msaa_quality_levels - 1);
        msaa_buf_desc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
        D3D12_CLEAR_VALUE clear_value;
        clear_value.Format = RTV_FORMAT;
        memcpy(clear_value.Color, DirectX::Colors::Aqua, sizeof(float) * 4);
        auto msaa_heap_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        ThrowIfFailed(
                _device->CreateCommittedResource(&msaa_heap_prop,
                                                 D3D12_HEAP_FLAG_NONE,
                                                 &msaa_buf_desc,
                                                 D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
                                                 &clear_value, IID_PPV_ARGS(
                                &_msaa_render_buffer)));


        _device->CreateRenderTargetView(_msaa_render_buffer.Get(),
                                        nullptr,
                                        _msaa_render_buffer_heap->GetCPUDescriptorHandleForHeapStart());
    }

    void directx_12::init_dsv(const window_info &info) {
        D3D12_DESCRIPTOR_HEAP_DESC dh_desc = {};
        dh_desc.NumDescriptors = 1;
        dh_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        dh_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        dh_desc.NodeMask = 0;

        ThrowIfFailed(_device->CreateDescriptorHeap(&dh_desc, IID_PPV_ARGS(
                &_dsv_desc_heap)));
        _dsv_handle = _dsv_desc_heap->GetCPUDescriptorHandleForHeapStart();

        auto r_desc = CD3DX12_RESOURCE_DESC::Tex2D(DSV_FORMAT,
                                                   info.width, info.height, 1,
                                                   1, msaa_sample_count,
                                                   msaa_quality_levels - 1);
        r_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

        auto heap_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        auto clear_value = CD3DX12_CLEAR_VALUE(DSV_FORMAT, 1.f, 0);

        ThrowIfFailed(_device->CreateCommittedResource(&heap_prop,
                                                       D3D12_HEAP_FLAG_NONE,
                                                       &r_desc,
                                                       D3D12_RESOURCE_STATE_DEPTH_WRITE,
                                                       &clear_value,
                                                       IID_PPV_ARGS(
                                                               &_dsv_buffer)));


        _device->CreateDepthStencilView(_dsv_buffer.Get(), nullptr,
                                        _dsv_handle);
    }

    void directx_12::init_global_buf() {
        _global_buffer = create_const_buffer<global>(1, _device);
        auto plane = Plane(Vector3(0.f, 0.001f, 0.f), Vector3::Up);
        auto n = Vector4(0.f, -1.f, 1.f, 0.f);
        n.Normalize();
        n = -n;
        _global.shadow_matrix = DirectX::XMMatrixShadow(plane, n);
        update_const_buffer(_global_buffer, &_global, 0);
    }

    void directx_12::init_camera_buf() {
        _vp_buffer = create_const_buffer<camera_buf>(1, _device);
    }

    void directx_12::init_light_buf() {
        _light_buffer = create_const_buffer<light_info>(2, _device);
    }

    void directx_12::init_resources() {
        _obj_const_buffer = create_const_buffer<object_constant>(OBJ_CNT,
                                                                 _device);
        _skin_metrics_buf = create_const_buffer<skin_matrix>(OBJ_CNT, _device);
        _mat_buffer = create_upload_buffer(OBJ_CNT, sizeof(material), _device);

        D3D12_DESCRIPTOR_HEAP_DESC h_desc = {};
        h_desc.NodeMask = 0;
        h_desc.NumDescriptors = OBJ_CNT * TABLE_SIZE + 2;//1 is for cube map;
        h_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        h_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
        _device->CreateDescriptorHeap(&h_desc, IID_PPV_ARGS(&_res_desc_heap));

        for (auto i = 0; i < OBJ_CNT; ++i) {
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
            cbv_desc.SizeInBytes = size_of_256<object_constant>();
            cbv_desc.BufferLocation =
                    _obj_const_buffer->GetGPUVirtualAddress() +
                    size_of_256<object_constant>() * i;
            auto handle = _res_desc_heap->GetCPUDescriptorHandleForHeapStart();
            handle.ptr += group_size() * i;
            _device->CreateConstantBufferView(&cbv_desc, handle);
        }

        for (auto i = 0; i < OBJ_CNT; ++i) {
            D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
            cbv_desc.SizeInBytes = size_of_256<skin_matrix>();
            cbv_desc.BufferLocation =
                    _skin_metrics_buf->GetGPUVirtualAddress() +
                    size_of_256<skin_matrix>() * i;
            auto handle = _res_desc_heap->GetCPUDescriptorHandleForHeapStart();
            handle.ptr += group_size() * i +
                          _device->GetDescriptorHandleIncrementSize(
                                  D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
            _device->CreateConstantBufferView(&cbv_desc, handle);
        }
    }

    void directx_12::init_root_signature() {
        init_default_signature();
        init_blur_signature();
    }

    void directx_12::init_shadow_signature() {
        CD3DX12_ROOT_PARAMETER param[1];
        param[1].InitAsConstantBufferView(0);
        auto rs_desc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param, 0,
                                                   nullptr);

        ComPtr<ID3DBlob> blob_signature;
        ComPtr<ID3DBlob> blob_error;
        D3D12SerializeRootSignature(&rs_desc, D3D_ROOT_SIGNATURE_VERSION_1,
                                    &blob_signature, &blob_error);
        ThrowIfFailed(_device->CreateRootSignature(0,
                                                   blob_signature->GetBufferPointer(),
                                                   blob_signature->GetBufferSize(),
                                                   IID_PPV_ARGS(
                                                           &_signatures[shader_type::shadow])));
    }

    void directx_12::init_terrain_signature() {
        CD3DX12_DESCRIPTOR_RANGE ranges[] = {
                CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 3),
                CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0)
        };
        CD3DX12_ROOT_PARAMETER param[5];
        param[0].InitAsConstantBufferView(static_cast<uint32_t>(0));//global
        param[1].InitAsConstantBufferView(static_cast<uint32_t>(1));//camera
        param[2].InitAsConstantBufferView(static_cast<uint32_t>(2));//lights
        param[3].InitAsShaderResourceView(0, 1);//mats
        param[4].InitAsDescriptorTable(_countof(ranges), ranges);//object const

        auto sampler_arr = sampler::samplers();

        auto rs_desc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param,
                                                   sampler_arr.size(),
                                                   sampler_arr.data());
        rs_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> blob_signature;
        ComPtr<ID3DBlob> blob_error;
        D3D12SerializeRootSignature(&rs_desc, D3D_ROOT_SIGNATURE_VERSION_1,
                                    &blob_signature, &blob_error);
        ThrowIfFailed(_device->CreateRootSignature(0,
                                                   blob_signature->GetBufferPointer(),
                                                   blob_signature->GetBufferSize(),
                                                   IID_PPV_ARGS(
                                                           &_signatures[shader_type::terrain])));
    }

    void directx_12::init_blur_signature() {
        CD3DX12_DESCRIPTOR_RANGE blur_ranges[] = {
                CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0),
                CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0)
        };

        CD3DX12_ROOT_PARAMETER blur_param[2];
        blur_param[0].InitAsConstants(12, 0);
        blur_param[1].InitAsDescriptorTable(_countof(blur_ranges), blur_ranges);

        auto blur_rs_desc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(blur_param),
                                                        blur_param);
        blur_rs_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> blob_signature0;
        ComPtr<ID3DBlob> blob_error0;
        ThrowIfFailed(D3D12SerializeRootSignature(&blur_rs_desc,
                                                  D3D_ROOT_SIGNATURE_VERSION_1,
                                                  &blob_signature0,
                                                  &blob_error0));
        ThrowIfFailed(_device->CreateRootSignature(0,
                                                   blob_signature0->GetBufferPointer(),
                                                   blob_signature0->GetBufferSize(),
                                                   IID_PPV_ARGS(
                                                           &_signatures[shader_type::blur])));
    }
    void directx_12::init_default_signature() {
        CD3DX12_DESCRIPTOR_RANGE t0[] = {
                CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 0),
        };

        CD3DX12_DESCRIPTOR_RANGE t1[] = {
                CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 2, 3),
                CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 2, 2)
        };

        CD3DX12_ROOT_PARAMETER param[6];
        param[0].InitAsConstantBufferView(0);//global
        param[1].InitAsConstantBufferView(1);//camera
        param[2].InitAsConstantBufferView(2);//lights
        param[3].InitAsShaderResourceView(0, 1);//mats
        param[4].InitAsDescriptorTable(_countof(t0), t0);//skybox, shadow
        param[5].InitAsDescriptorTable(_countof(t1), t1);//object const

        auto sampler_arr = sampler::samplers();

        auto rs_desc = CD3DX12_ROOT_SIGNATURE_DESC(_countof(param), param,
                                                   sampler_arr.size(),
                                                   sampler_arr.data());
        rs_desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> blob_signature;
        ComPtr<ID3DBlob> blob_error;
        D3D12SerializeRootSignature(&rs_desc, D3D_ROOT_SIGNATURE_VERSION_1,
                                    &blob_signature, &blob_error);
        ThrowIfFailed(_device->CreateRootSignature(0,
                                                   blob_signature->GetBufferPointer(),
                                                   blob_signature->GetBufferSize(),
                                                   IID_PPV_ARGS(
                                                           &_signatures[shader_type::general])));
    }

    void directx_12::init_shader() {
#if defined(_DEBUG)
        UINT compile_flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
        UINT compile_flags = 0;
#endif

        _pso_list.resize(static_cast<uint8_t>(layer::end));

        auto vs_data = DX::ReadData(L"shader\\vs.cso");
        auto vs_ref_data = DX::ReadData(L"shader\\vs_ref.cso");
        auto vs_sha_data = DX::ReadData(L"shader\\vs_sha.cso");
        auto ps_data = DX::ReadData(L"shader\\ps.cso");
        auto ps_sha_data = DX::ReadData(L"shader\\ps_sha.cso");

        D3D12_INPUT_ELEMENT_DESC ie_desc[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"WEIGHTS",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 44, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"INDICES",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 60, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
        };

        auto opaque_desc = pipeline_state::default_desc(ie_desc,
                                                        _countof(ie_desc),
                                                        _signatures[shader_type::general].Get(),
                                                        vs_data,
                                                        ps_data);

        ThrowIfFailed(_device->CreateGraphicsPipelineState
                (&opaque_desc, IID_PPV_ARGS(
                        &_pso_list[static_cast<uint8_t>(layer::opaque)])));


        auto trans_pso = pipeline_state::transparent_desc(ie_desc,
                                                          _countof(ie_desc),
                                                          _signatures[shader_type::general].Get(),
                                                          vs_data,
                                                          ps_data);

        ThrowIfFailed(_device->CreateGraphicsPipelineState
                (&trans_pso, IID_PPV_ARGS(
                        &_pso_list[static_cast<uint8_t>(layer::transparent)])));


        auto mirror_pso = pipeline_state::mirror_desc(ie_desc,
                                                      _countof(ie_desc),
                                                      _signatures[shader_type::general].Get(),
                                                      vs_data,
                                                      ps_data);
        ThrowIfFailed(_device->CreateGraphicsPipelineState
                (&mirror_pso, IID_PPV_ARGS(
                        &_pso_list[static_cast<uint8_t>(layer::mirror)])));

        auto ref_pso = pipeline_state::reflection_desc(ie_desc,
                                                       _countof(ie_desc),
                                                       _signatures[shader_type::general].Get(),
                                                       vs_ref_data,
                                                       ps_data);
        ThrowIfFailed(_device->CreateGraphicsPipelineState
                (&ref_pso, IID_PPV_ARGS(
                        &_pso_list[static_cast<uint8_t>(layer::reflection)])));

        auto shadow_pso = pipeline_state::shadow_desc(ie_desc,
                                                      _countof(ie_desc),
                                                      _signatures[shader_type::general].Get(),
                                                      vs_sha_data,
                                                      ps_sha_data);

        ThrowIfFailed(_device->CreateGraphicsPipelineState
                (&shadow_pso, IID_PPV_ARGS(
                        &_pso_list[static_cast<uint8_t>(layer::shadow)])));

        D3D12_INPUT_ELEMENT_DESC ie_desc_bill[] = {
                {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
                {"SIZE",     0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };

        auto vs_bill_data = DX::ReadData(L"shader\\vs_bill.cso");
        auto ps_bill_data = DX::ReadData(L"shader\\ps_bill.cso");
        auto gs_bill_data = DX::ReadData(L"shader\\gs_bill.cso");
        auto billboard_pso = pipeline_state::billboard_desc(ie_desc_bill,
                                                            _countof(
                                                                    ie_desc_bill),
                                                            _signatures[shader_type::general].Get(),
                                                            vs_bill_data,
                                                            ps_bill_data,
                                                            gs_bill_data);

        ThrowIfFailed(_device->CreateGraphicsPipelineState
                (&billboard_pso, IID_PPV_ARGS(
                        &_pso_list[static_cast<uint8_t>(layer::billboard)])));

        auto cs_blur_h_data = DX::ReadData(L"shader\\cs_blur_h.cso");
        auto cs_blur_v_data = DX::ReadData(L"shader\\cs_blur_v.cso");
        auto blur_h_pso = pipeline_state::blur_desc(
                _signatures[shader_type::blur].Get(),
                cs_blur_h_data);
        auto blur_v_pso = pipeline_state::blur_desc(
                _signatures[shader_type::blur].Get(),
                cs_blur_v_data);
        ThrowIfFailed(_device->CreateComputePipelineState(&blur_h_pso,
                                                          IID_PPV_ARGS(
                                                                  &_pso_list[static_cast<uint8_t>(layer::blur_h)])));
        ThrowIfFailed(_device->CreateComputePipelineState(&blur_v_pso,
                                                          IID_PPV_ARGS(
                                                                  &_pso_list[static_cast<uint8_t>(layer::blur_v)])));

        auto vs_terrain = DX::ReadData(L"shader\\vs_terrain.cso");
        auto hs_terrain = DX::ReadData(L"shader\\hs_terrain.cso");
        auto ds_terrain = DX::ReadData(L"shader\\ds_terrain.cso");
        auto ps_terrain = DX::ReadData(L"shader\\ps_terrain.cso");

        auto
                terrain_pso = pipeline_state::terrain_desc(ie_desc,
                                                           _countof(ie_desc),
                                                           _signatures[shader_type::general].Get(),
                                                           vs_terrain,
                                                           hs_terrain,
                                                           ds_terrain,
                                                           ps_terrain);
        ThrowIfFailed(_device->CreateGraphicsPipelineState(&terrain_pso,
                                                           IID_PPV_ARGS(
                                                                   &_pso_list[static_cast<uint8_t>(layer::terrain)])));

        auto vs_skybox = DX::ReadData(L"shader\\vs_skybox.cso");
        auto ps_skybox = DX::ReadData(L"shader\\ps_skybox.cso");
        auto skybox_pso = pipeline_state::skybox_desc(ie_desc,
                                                      _countof(ie_desc),
                                                      _signatures[shader_type::general].Get(),
                                                      vs_skybox, ps_skybox);
        ThrowIfFailed(_device->CreateGraphicsPipelineState(&skybox_pso,
                                                           IID_PPV_ARGS(
                                                                   &_pso_list[static_cast<uint8_t>(layer::skybox)])));

        auto vs_shadow = DX::ReadData(L"shader\\vs_shadow.cso");
        auto ps_shadow = DX::ReadData(L"shader\\ps_shadow.cso");
        auto d_shadow_pso = pipeline_state::dynamic_shadow_desc(ie_desc,
                                                                _countof(
                                                                        ie_desc),
                                                                _signatures[shader_type::general].Get(),
                                                                vs_shadow,
                                                                ps_shadow);
        ThrowIfFailed(_device->CreateGraphicsPipelineState(&d_shadow_pso,
                                                           IID_PPV_ARGS(
                                                                   &_pso_list[static_cast<uint8_t>(layer::dynamic_shadow)])));

        auto vs_skin = DX::ReadData(L"shader\\vs_skin.cso");
        auto skin_pso = pipeline_state::default_desc(ie_desc,
                                                     _countof(ie_desc),
                                                     _signatures[shader_type::general].Get(),
                                                     vs_skin,
                                                     ps_data);
        ThrowIfFailed(_device->CreateGraphicsPipelineState(&skin_pso,
                                                           IID_PPV_ARGS(
                                                                   &_pso_list[static_cast<uint8_t>(layer::skinned)])));
    }

    void directx_12::execute_cmd_list() {
        ID3D12CommandList *cmd_list_arr[] = {_cmd_list.Get()};
        _cmd_queue->ExecuteCommandLists(_countof(cmd_list_arr), cmd_list_arr);
    }

    void directx_12::wait_cmd_queue_sync() {
        ++_fence_value;
        _cmd_queue->Signal(_fence.Get(), _fence_value);
        if (_fence->GetCompletedValue() < _fence_value) {
            auto fence_event = CreateEventEx(nullptr, nullptr, false,
                                             EVENT_ALL_ACCESS);
            _fence->SetEventOnCompletion(_fence_value, fence_event);
            WaitForSingleObject(fence_event, INFINITE);
            CloseHandle(fence_event);
        }
    }

    UINT directx_12::group_size() {
        return _device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) * TABLE_SIZE;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE directx_12::shadow_handle() {
        auto ret = _res_desc_heap->GetCPUDescriptorHandleForHeapStart();
        ret.ptr += OBJ_CNT * group_size();
        ret.ptr += _device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        return ret;
    }
}