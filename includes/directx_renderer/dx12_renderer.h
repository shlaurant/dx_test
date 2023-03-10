#pragma once

#include "d3dx12.h"
#include "SimpleMath.h"

#include "common.h"
#include "resource.h"
#include "constants.h"
#include "dx_util.h"
#include "debug.h"
#include "typeid.h"
#include "blur.h"
#include "renderee.h"
#include "shadow_map.h"
#include "frame_resource.h"

using namespace Microsoft::WRL;

namespace directx_renderer {
    struct window_info {
        HWND hwnd;
        int32_t width;
        int32_t height;
        bool windowed;
    };

    const uint32_t OPTION_BLUR = 1 << 0;

    class dx12_renderer {
    public:
        const static int OBJ_CNT = 20;
        const static int SKIN_CNT = 3;

        void init(const window_info &);

        template<typename T>
        void load_geometries(std::vector<geometry<T>> &geometries) {
            ThrowIfFailed(_cmd_alloc->Reset());
            ThrowIfFailed(_cmd_list->Reset(_cmd_alloc.Get(), nullptr));

            ComPtr<ID3D12Resource> u_buffer_v;
            std::vector<T> vertices;
            size_t index_v = 0;

            ComPtr<ID3D12Resource> u_buffer_i;
            std::vector<uint16_t> indices;
            size_t index_i = 0;

            for (auto &e: geometries) {
                e.vertex_offset = index_v;
                std::copy(e.vertices.begin(), e.vertices.end(),
                          std::back_inserter(vertices));
                index_v += e.vertices.size();

                for (auto i = 0; i < e.names.size(); ++i) {
                    std::copy(e.indices[i].begin(), e.indices[i].end(),
                              std::back_inserter(indices));
                    e.index_offset = index_i;
                    index_i += e.indices[i].size();
                    _geo_infos[type_id<T>()][e.names[i]] = {
                            (UINT) e.vertex_offset,
                            (UINT) e.index_offset,
                            (UINT) e.indices[i].size()};
                }
            }

            auto vert_byte_size = sizeof(T) * vertices.size();
            std::copy(vertices.begin(), vertices.end(), vertices.data());

            auto vb = create_default_buffer(vertices.data(), vert_byte_size,
                                            u_buffer_v, _device, _cmd_list);
            D3D12_VERTEX_BUFFER_VIEW vbv;
            vbv.BufferLocation = vb->GetGPUVirtualAddress();
            vbv.StrideInBytes = sizeof(T);
            vbv.SizeInBytes = vert_byte_size;
            _vertex_buffers[type_id<T>()] = std::make_pair(vb, vbv);

            auto ib = create_default_buffer(indices.data(),
                                            sizeof(uint16_t) *
                                            indices.size(),
                                            u_buffer_i, _device,
                                            _cmd_list);

            D3D12_INDEX_BUFFER_VIEW ibv;
            ibv.BufferLocation = ib->GetGPUVirtualAddress();
            ibv.Format = DXGI_FORMAT_R16_UINT;
            ibv.SizeInBytes = sizeof(uint16_t) * indices.size();

            _index_buffers[type_id<T>()] = std::make_pair(ib, ibv);

            _cmd_list->Close();
            execute_cmd_list();
            wait_cmd_queue_sync();
        }
        void load_texture(const std::string &name, const std::wstring &path);
        void load_material(const std::vector<std::string> &names,
                           const std::vector<material> &mat);
        void init_renderees(const std::vector<std::shared_ptr<renderee>> &);
        void update_frame(const frame_globals &,
                          const std::vector<object_constant> &,
                          const std::vector<skin_matrix> &);

        void render(uint32_t option = 0);

    private:
        enum class root_param : uint8_t {
            g_scene,
            g_frame,
            obj_const,
            skin_matrix,
            g_texture,
            obj_texture,
            material
        };

        enum class layer : uint8_t {
            opaque,
            skinned,
            transparent,
            mirror,
            ref_opaque,
            ref_transparent,
            ref_skinned,
            ref_terrain,
            shadow,
            billboard,
            blur_h,
            blur_v,
            terrain,
            skybox,
            dynamic_shadow,
            dynamic_shadow_skinned,
            end
        };

        enum class shader_type {
            general, blur, terrain, shadow
        };

        const static int SWAP_CHAIN_BUFFER_COUNT = 2;
        const static int FRAME_RESOURCE_BUFFER_SIZE = 3;
        static const int TABLE_SIZE = 4;
        static const int TEX_PER_OBJ = 2;
        static const int TEX_GLOBAL_CNT = 2;

        ComPtr<IDXGIFactory> _factory;
        ComPtr<ID3D12Device> _device;
        D3D12_VIEWPORT _view_port;
        D3D12_RECT _scissors_rect;
        UINT msaa_sample_count = 4;
        UINT msaa_quality_levels;


        //cmd queue
        ComPtr<ID3D12CommandQueue> _cmd_queue;
        ComPtr<ID3D12CommandAllocator> _cmd_alloc;
        ComPtr<ID3D12GraphicsCommandList> _cmd_list;
        ComPtr<ID3D12Fence> _fence;
        uint32_t _fence_value = 0;

        //swap chain & rtv & dsv
        ComPtr<IDXGISwapChain> _swap_chain;
        uint32_t _back_buffer = 0;
        ComPtr<ID3D12Resource> _rtv_buffer[SWAP_CHAIN_BUFFER_COUNT];
        ComPtr<ID3D12DescriptorHeap> _rtv_heap;
        D3D12_CPU_DESCRIPTOR_HANDLE _rtv_handle[SWAP_CHAIN_BUFFER_COUNT];
        ComPtr<ID3D12Resource> _msaa_render_buffer;
        ComPtr<ID3D12DescriptorHeap> _msaa_render_buffer_heap;

        ComPtr<ID3D12Resource> _dsv_buffer;
        ComPtr<ID3D12DescriptorHeap> _dsv_desc_heap;
        D3D12_CPU_DESCRIPTOR_HANDLE _dsv_handle;

        //root sig
        std::unordered_map<shader_type, ComPtr<ID3D12RootSignature>> _signatures;

        //resource
        std::unique_ptr<frame_resource_buffer> _fres_buffer;
        std::unordered_map<uint32_t, std::unordered_map<std::string, geo_info>> _geo_infos;
        std::unordered_map<uint32_t, std::pair<ComPtr<
                ID3D12Resource>, D3D12_VERTEX_BUFFER_VIEW>> _vertex_buffers;
        std::unordered_map<uint32_t, std::pair<ComPtr<
                ID3D12Resource>, D3D12_INDEX_BUFFER_VIEW>> _index_buffers;
        global _global;

        ComPtr<ID3D12Resource> _global_buffer;//globals set automatically.
        ComPtr<ID3D12Resource> _mat_buffer;

        std::unordered_map<std::string, int> _mat_ids;
        std::unordered_map<std::string, std::pair<D3D12_SHADER_RESOURCE_VIEW_DESC,
                ComPtr<ID3D12Resource>>> _textures;
        ComPtr<ID3D12DescriptorHeap> _tex_desc_heap;


        //shader
        std::vector<ComPtr<ID3D12PipelineState>> _pso_list;

        //cs
        blur _blur;
        shadow_map _shadow_map;

        //user modifiable
        std::vector<std::vector<std::shared_ptr<renderee>>> _renderees = std::vector<std::vector<std::shared_ptr<renderee>>>(
                static_cast<uint8_t>(renderee_type::count));

        void init_base(const window_info &info);
        void init_cmds();
        void init_swap_chain(const window_info &info);
        void init_rtv(const window_info &);
        void init_dsv(const window_info &);

        void init_global_buf();
        void init_resources();
        void bind_texture(int obj, const std::string &texture, int regi);

        void init_root_signature();
        void init_shader();

        void execute_cmd_list();
        void wait_cmd_queue_sync();

        void render_begin();
        void render_end(uint32_t option);
        void render(const std::shared_ptr<renderee> &);

        UINT group_size();

        ComPtr<ID3D12Resource>
        load_texture(const std::wstring &path, DirectX::ScratchImage &image,
                     ComPtr<ID3D12Resource> &upload_buffer);
        void init_default_signature();
        void init_blur_signature();
        void init_terrain_signature();
        void init_shadow_signature();
        D3D12_CPU_DESCRIPTOR_HANDLE shadow_handle();
        UINT cbv_handle_size();
    };
}
