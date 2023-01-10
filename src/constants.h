#pragma once

#include "d3dx12.h"

namespace fuse::directx {
    static const DXGI_FORMAT RTV_FORMAT = DXGI_FORMAT_R8G8B8A8_UNORM;
    static const DXGI_FORMAT DSV_FORMAT = DXGI_FORMAT_D24_UNORM_S8_UINT;
    static const DXGI_FORMAT SHADOW_FORMAT = DXGI_FORMAT_R24G8_TYPELESS;
    static const DXGI_FORMAT SHADOW_SRV_FORMAT = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

    class sampler {
    public:
        static const CD3DX12_STATIC_SAMPLER_DESC POINT_WRAP;
        static const CD3DX12_STATIC_SAMPLER_DESC POINT_CLAMP;
        static const CD3DX12_STATIC_SAMPLER_DESC POINT_MIRROR;

        static const CD3DX12_STATIC_SAMPLER_DESC LINEAR_WRAP;
        static const CD3DX12_STATIC_SAMPLER_DESC LINEAR_CLAMP;
        static const CD3DX12_STATIC_SAMPLER_DESC LINEAR_MIRROR;

        static const CD3DX12_STATIC_SAMPLER_DESC ANISOTROPIC_WRAP;
        static const CD3DX12_STATIC_SAMPLER_DESC ANISOTROPIC_CLAMP;
        static const CD3DX12_STATIC_SAMPLER_DESC ANISOTROPIC_MIRROR;

        static const CD3DX12_STATIC_SAMPLER_DESC SHADOW_PCF;

        static std::array<const CD3DX12_STATIC_SAMPLER_DESC, 10> samplers();
    };

    class pipeline_state {
    public:
        static D3D12_GRAPHICS_PIPELINE_STATE_DESC
        default_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                     ID3D12RootSignature *rs,
                     const std::vector<uint8_t> &vs,
                     const std::vector<uint8_t> &ps);

        static D3D12_GRAPHICS_PIPELINE_STATE_DESC
        billboard_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                       ID3D12RootSignature *rs,
                       const std::vector<uint8_t> &vs,
                       const std::vector<uint8_t> &ps,
                       const std::vector<uint8_t> &gs);

        static D3D12_GRAPHICS_PIPELINE_STATE_DESC
        transparent_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                         ID3D12RootSignature *rs,
                         const std::vector<uint8_t> &vs,
                         const std::vector<uint8_t> &ps);

        static D3D12_GRAPHICS_PIPELINE_STATE_DESC
        mirror_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                    ID3D12RootSignature *rs,
                    const std::vector<uint8_t> &vs,
                    const std::vector<uint8_t> &ps);

        static D3D12_GRAPHICS_PIPELINE_STATE_DESC
        reflection_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                        ID3D12RootSignature *rs,
                        const std::vector<uint8_t> &vs,
                        const std::vector<uint8_t> &ps);

        static D3D12_GRAPHICS_PIPELINE_STATE_DESC
        shadow_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                    ID3D12RootSignature *rs,
                    const std::vector<uint8_t> &vs,
                    const std::vector<uint8_t> &ps);

        static D3D12_COMPUTE_PIPELINE_STATE_DESC
        blur_desc(ID3D12RootSignature *rs, const std::vector<uint8_t> &cs);

        static D3D12_GRAPHICS_PIPELINE_STATE_DESC
        terrain_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                     ID3D12RootSignature *rs,
                     const std::vector<uint8_t> &vs,
                     const std::vector<uint8_t> &hs,
                     const std::vector<uint8_t> &ds,
                     const std::vector<uint8_t> &ps);

        static D3D12_GRAPHICS_PIPELINE_STATE_DESC
        skybox_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                    ID3D12RootSignature *rs,
                    const std::vector<uint8_t> &vs,
                    const std::vector<uint8_t> &ps);

        static D3D12_GRAPHICS_PIPELINE_STATE_DESC
        dynamic_shadow_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                            ID3D12RootSignature *rs,
                            const std::vector<uint8_t> &vs,
                            const std::vector<uint8_t> &ps);
    };
}
