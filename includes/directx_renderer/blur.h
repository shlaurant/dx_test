#pragma once

using namespace Microsoft::WRL;

namespace fuse::directx {
    class blur {
    public:
        void init(ComPtr<ID3D12Device>, int32_t w, int32_t h);

        void blur_texture(ComPtr<ID3D12GraphicsCommandList> cmd_list,
                          ComPtr<ID3D12Resource> target,
                          ComPtr<ID3D12PipelineState> h_pso,
                          ComPtr<ID3D12PipelineState> v_pso,
                          ComPtr<ID3D12RootSignature> rs);


    private:
        ComPtr<ID3D12Device> _device;
        ComPtr<ID3D12Resource> _tex_A;
        ComPtr<ID3D12Resource> _tex_B;
        ComPtr<ID3D12DescriptorHeap> _tex_desc_heap;

        int32_t width;
        int32_t height;

        std::vector<float> gauss_weights(float sigma);
        void
        do_blur(ComPtr<ID3D12GraphicsCommandList> cmd_list,
                ComPtr<ID3D12Resource> target);
    };
}