#include "blur.h"
#include "dx12_renderer.h"

namespace directx_renderer {
    void blur::init(ComPtr<ID3D12Device> device, int32_t w, int32_t h) {
        width = w;
        height = h;

        _device = std::move(device);
        auto tex_desc = CD3DX12_RESOURCE_DESC::Tex2D(RTV_FORMAT, w,
                                                     h);
        tex_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
        tex_desc.MipLevels = 1;
        auto heap_prop = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

        _device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
                                         &tex_desc, D3D12_RESOURCE_STATE_COMMON,
                                         nullptr, IID_PPV_ARGS(&_tex_A));

        _device->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
                                         &tex_desc, D3D12_RESOURCE_STATE_COMMON,
                                         nullptr, IID_PPV_ARGS(&_tex_B));

        D3D12_DESCRIPTOR_HEAP_DESC heap_desc;
        heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        heap_desc.NodeMask = 0;
        heap_desc.NumDescriptors = 4;
        heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

        _device->CreateDescriptorHeap(&heap_desc,
                                      IID_PPV_ARGS(&_tex_desc_heap));
        auto sz = _device->GetDescriptorHandleIncrementSize(
                D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

        D3D12_SHADER_RESOURCE_VIEW_DESC srvd = {};
        srvd.Format = RTV_FORMAT;
        srvd.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvd.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
        srvd.Texture2D.MostDetailedMip = 0;
        srvd.Texture2D.MipLevels = 1;

        D3D12_UNORDERED_ACCESS_VIEW_DESC uavd = {};
        uavd.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
        uavd.Format = RTV_FORMAT;
        uavd.Texture2D.MipSlice = 0;

        auto handle = _tex_desc_heap->GetCPUDescriptorHandleForHeapStart();
        _device->CreateShaderResourceView(_tex_A.Get(), &srvd, handle);
        handle.ptr += sz;
        _device->CreateUnorderedAccessView(_tex_B.Get(), nullptr, &uavd,
                                           handle);
        handle.ptr += sz;
        _device->CreateShaderResourceView(_tex_B.Get(), &srvd, handle);
        handle.ptr += sz;
        _device->CreateUnorderedAccessView(_tex_A.Get(), nullptr, &uavd,
                                           handle);
    }

    void blur::blur_texture(ComPtr<ID3D12GraphicsCommandList> cmd_list,
                            ComPtr<ID3D12Resource> target,
                            ComPtr<ID3D12PipelineState> h_pso,
                            ComPtr<ID3D12PipelineState> v_pso,
                            ComPtr<ID3D12RootSignature> rs) {
        auto weights = gauss_weights(2.5f);
        int radius = weights.size() / 2;

        ID3D12DescriptorHeap *heaps[] = {_tex_desc_heap.Get()};
        cmd_list->SetDescriptorHeaps(1, heaps);
        cmd_list->SetComputeRootSignature(rs.Get());
        cmd_list->SetComputeRoot32BitConstant(0, radius, 0);
        cmd_list->SetComputeRoot32BitConstants(0, weights.size(),
                                               weights.data(), 1);

        cmd_list->SetPipelineState(h_pso.Get());
        do_blur(cmd_list, target);
        cmd_list->SetPipelineState(v_pso.Get());
        do_blur(cmd_list, target);
    }
    void blur::do_blur(ComPtr<ID3D12GraphicsCommandList> cmd_list,
                       ComPtr<ID3D12Resource> target) {
        auto handle = _tex_desc_heap->GetGPUDescriptorHandleForHeapStart();
        cmd_list->SetComputeRootDescriptorTable(1, handle);

        CD3DX12_RESOURCE_BARRIER b0[] = {
                CD3DX12_RESOURCE_BARRIER::Transition(
                        target.Get(), D3D12_RESOURCE_STATE_RESOLVE_DEST,
                        D3D12_RESOURCE_STATE_COPY_SOURCE),
                CD3DX12_RESOURCE_BARRIER::Transition(
                        _tex_A.Get(), D3D12_RESOURCE_STATE_COMMON,
                        D3D12_RESOURCE_STATE_COPY_DEST)
        };
        cmd_list->ResourceBarrier(2, b0);

        cmd_list->CopyResource(_tex_A.Get(), target.Get());


        CD3DX12_RESOURCE_BARRIER b1[] = {
                CD3DX12_RESOURCE_BARRIER::Transition(
                        _tex_A.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                        D3D12_RESOURCE_STATE_GENERIC_READ),
                CD3DX12_RESOURCE_BARRIER::Transition(
                        _tex_B.Get(), D3D12_RESOURCE_STATE_COMMON,
                        D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        };
        cmd_list->ResourceBarrier(2, b1);

        cmd_list->Dispatch((UINT) ceil(((float) width) / 256.f), height, 1);

        auto sz = _device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
        handle.ptr += sz * 2;
        cmd_list->SetComputeRootDescriptorTable(1, handle);
        CD3DX12_RESOURCE_BARRIER b2[] = {
                CD3DX12_RESOURCE_BARRIER::Transition(
                        _tex_B.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                        D3D12_RESOURCE_STATE_GENERIC_READ),
                CD3DX12_RESOURCE_BARRIER::Transition(
                        _tex_A.Get(), D3D12_RESOURCE_STATE_GENERIC_READ,
                        D3D12_RESOURCE_STATE_UNORDERED_ACCESS)
        };
        cmd_list->ResourceBarrier(2, b2);

        cmd_list->Dispatch(width, (UINT) ceil(((float) height) / 256.f), 1);

        CD3DX12_RESOURCE_BARRIER b3[] = {
                CD3DX12_RESOURCE_BARRIER::Transition(
                        target.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE,
                        D3D12_RESOURCE_STATE_COPY_DEST),
                CD3DX12_RESOURCE_BARRIER::Transition(
                        _tex_A.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
                        D3D12_RESOURCE_STATE_COPY_SOURCE)
        };
        cmd_list->ResourceBarrier(2, b3);

        cmd_list->CopyResource(target.Get(), _tex_A.Get());

        CD3DX12_RESOURCE_BARRIER b4[] = {
                CD3DX12_RESOURCE_BARRIER::Transition(
                        target.Get(), D3D12_RESOURCE_STATE_COPY_DEST,
                        D3D12_RESOURCE_STATE_RESOLVE_DEST),
                CD3DX12_RESOURCE_BARRIER::Transition(
                        _tex_A.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE,
                        D3D12_RESOURCE_STATE_COMMON),
                CD3DX12_RESOURCE_BARRIER::Transition(
                        _tex_B.Get(), D3D12_RESOURCE_STATE_GENERIC_READ,
                        D3D12_RESOURCE_STATE_COMMON)
        };
        cmd_list->ResourceBarrier(3, b4);
    }

    std::vector<float> blur::gauss_weights(float sigma) {
        float twoSigma2 = 2.0f * sigma * sigma;

        // Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
        // For example, for sigma = 3, the width of the bell curve is
        int blurRadius = (int) ceil(2.0f * sigma);

        assert(blurRadius <= 5);

        std::vector<float> weights;
        weights.resize(2 * blurRadius + 1);

        float weightSum = 0.0f;

        for (int i = -blurRadius; i <= blurRadius; ++i) {
            float x = (float) i;

            weights[i + blurRadius] = expf(-x * x / twoSigma2);

            weightSum += weights[i + blurRadius];
        }

        // Divide by the sum so all the weights add up to 1.0.
        for (int i = 0; i < weights.size(); ++i) {
            weights[i] /= weightSum;
        }

        return weights;
    }
}