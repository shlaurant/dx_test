#include "constants.h"

namespace directx_renderer {
    const CD3DX12_STATIC_SAMPLER_DESC
            sampler::POINT_WRAP(0, D3D12_FILTER_MIN_MAG_MIP_POINT,
                                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                D3D12_TEXTURE_ADDRESS_MODE_WRAP);
    const CD3DX12_STATIC_SAMPLER_DESC
            sampler::POINT_CLAMP(1, D3D12_FILTER_MIN_MAG_MIP_POINT,
                                 D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                 D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                 D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    const CD3DX12_STATIC_SAMPLER_DESC
            sampler::POINT_MIRROR(2, D3D12_FILTER_MIN_MAG_MIP_POINT,
                                  D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
                                  D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
                                  D3D12_TEXTURE_ADDRESS_MODE_MIRROR);

    const CD3DX12_STATIC_SAMPLER_DESC
            sampler::LINEAR_WRAP(3, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                 D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                 D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                 D3D12_TEXTURE_ADDRESS_MODE_WRAP);
    const CD3DX12_STATIC_SAMPLER_DESC
            sampler::LINEAR_CLAMP(4, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                  D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                  D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                  D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    const CD3DX12_STATIC_SAMPLER_DESC
            sampler::LINEAR_MIRROR(5, D3D12_FILTER_MIN_MAG_MIP_LINEAR,
                                   D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
                                   D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
                                   D3D12_TEXTURE_ADDRESS_MODE_MIRROR);

    const CD3DX12_STATIC_SAMPLER_DESC
            sampler::ANISOTROPIC_WRAP(6, D3D12_FILTER_ANISOTROPIC,
                                      D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                      D3D12_TEXTURE_ADDRESS_MODE_WRAP,
                                      D3D12_TEXTURE_ADDRESS_MODE_WRAP);
    const CD3DX12_STATIC_SAMPLER_DESC
            sampler::ANISOTROPIC_CLAMP(7, D3D12_FILTER_ANISOTROPIC,
                                       D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                       D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
                                       D3D12_TEXTURE_ADDRESS_MODE_CLAMP);
    const CD3DX12_STATIC_SAMPLER_DESC
            sampler::ANISOTROPIC_MIRROR(8, D3D12_FILTER_ANISOTROPIC,
                                        D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
                                        D3D12_TEXTURE_ADDRESS_MODE_MIRROR,
                                        D3D12_TEXTURE_ADDRESS_MODE_MIRROR);

    const CD3DX12_STATIC_SAMPLER_DESC
            sampler::SHADOW_PCF(9,
                                D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT,
                                D3D12_TEXTURE_ADDRESS_MODE_BORDER,
                                D3D12_TEXTURE_ADDRESS_MODE_BORDER,
                                D3D12_TEXTURE_ADDRESS_MODE_BORDER, 0.f, 16,
                                D3D12_COMPARISON_FUNC_LESS_EQUAL,
                                D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK);

    std::array<const CD3DX12_STATIC_SAMPLER_DESC, 10> sampler::samplers() {
        return {POINT_WRAP, POINT_CLAMP, POINT_MIRROR, LINEAR_WRAP,
                LINEAR_CLAMP, LINEAR_MIRROR, ANISOTROPIC_WRAP,
                ANISOTROPIC_CLAMP, ANISOTROPIC_MIRROR, SHADOW_PCF};
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC
    pipeline_state::default_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                                 ID3D12RootSignature *rs,
                                 const std::vector<uint8_t> &vs,
                                 const std::vector<uint8_t> &ps) {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC ret = {};
        ret.InputLayout = {ed, ed_cnt};
        ret.pRootSignature = rs;
        ret.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        ret.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        ret.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
        ret.DSVFormat = DSV_FORMAT;
        ret.SampleMask = UINT_MAX;
        ret.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        ret.NumRenderTargets = 1;
        ret.RTVFormats[0] = RTV_FORMAT;
        ret.SampleDesc.Count = 4;
        ret.SampleDesc.Quality = 0;
        ret.VS = {vs.data(), vs.size()};
        ret.PS = {ps.data(), ps.size()};
        return ret;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC
    pipeline_state::billboard_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                                   ID3D12RootSignature *rs,
                                   const std::vector<uint8_t> &vs,
                                   const std::vector<uint8_t> &ps,
                                   const std::vector<uint8_t> &gs) {
        auto ret = default_desc(ed, ed_cnt, rs, vs, ps);
        ret.GS = {gs.data(), gs.size()};
        ret.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        ret.BlendState.AlphaToCoverageEnable = true;
        return ret;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC
    pipeline_state::transparent_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                                     ID3D12RootSignature *rs,
                                     const std::vector<uint8_t> &vs,
                                     const std::vector<uint8_t> &ps) {
        auto ret = default_desc(ed, ed_cnt, rs, vs, ps);

        D3D12_RENDER_TARGET_BLEND_DESC transparent_blend_desc;
        transparent_blend_desc.BlendEnable = true;
        transparent_blend_desc.LogicOpEnable = false;
        transparent_blend_desc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
        transparent_blend_desc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        transparent_blend_desc.BlendOp = D3D12_BLEND_OP_ADD;
        transparent_blend_desc.SrcBlendAlpha = D3D12_BLEND_ONE;
        transparent_blend_desc.DestBlendAlpha = D3D12_BLEND_ZERO;
        transparent_blend_desc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
        transparent_blend_desc.LogicOp = D3D12_LOGIC_OP_NOOP;
        transparent_blend_desc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

        ret.BlendState.RenderTarget[0] = transparent_blend_desc;
        ret.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

        return ret;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC
    pipeline_state::mirror_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                                ID3D12RootSignature *rs,
                                const std::vector<uint8_t> &vs,
                                const std::vector<uint8_t> &ps) {
        auto ret = default_desc(ed, ed_cnt, rs, vs, ps);
        ret.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;
        D3D12_DEPTH_STENCIL_DESC ds_desc = {};
        ds_desc.DepthEnable = true;
        ds_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        ds_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        ds_desc.StencilEnable = true;
        ds_desc.StencilReadMask = 0xff;
        ds_desc.StencilWriteMask = 0xff;

        ds_desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        ds_desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
        ds_desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

        ds_desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        ds_desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
        ds_desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

        ret.DepthStencilState = ds_desc;

        return ret;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC
    pipeline_state::reflection_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                                    ID3D12RootSignature *rs,
                                    const std::vector<uint8_t> &vs,
                                    const std::vector<uint8_t> &ps) {
        auto ret = default_desc(ed, ed_cnt, rs, vs, ps);
        D3D12_DEPTH_STENCIL_DESC ref_dss = {};
        ref_dss.DepthEnable = true;
        ref_dss.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        ref_dss.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        ref_dss.StencilEnable = true;
        ref_dss.StencilReadMask = 0xff;
        ref_dss.StencilWriteMask = 0xff;

        ref_dss.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
        ref_dss.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        ref_dss.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        ref_dss.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

        ref_dss.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        ref_dss.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
        ref_dss.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        ref_dss.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

        ret.DepthStencilState = ref_dss;
        ret.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        ret.RasterizerState.FrontCounterClockwise = true;

        return ret;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC
    pipeline_state::shadow_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                                ID3D12RootSignature *rs,
                                const std::vector<uint8_t> &vs,
                                const std::vector<uint8_t> &ps) {
        auto ret = transparent_desc(ed, ed_cnt, rs, vs, ps);

        D3D12_DEPTH_STENCIL_DESC ds_desc = {};
        ds_desc.DepthEnable = true;
        ds_desc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
        ds_desc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        ds_desc.StencilEnable = true;
        ds_desc.StencilReadMask = 0xff;
        ds_desc.StencilWriteMask = 0xff;

        ds_desc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
        ds_desc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_INCR;
        ds_desc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        ds_desc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

        ds_desc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        ds_desc.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
        ds_desc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        ds_desc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

        ret.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;

        ret.DepthStencilState = ds_desc;

        return ret;
    }

    D3D12_COMPUTE_PIPELINE_STATE_DESC
    pipeline_state::blur_desc(ID3D12RootSignature *rs,
                              const std::vector<uint8_t> &cs) {
        D3D12_COMPUTE_PIPELINE_STATE_DESC ret = {};
        ret.pRootSignature = rs;
        ret.NodeMask = 0;
        ret.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        ret.CS = {cs.data(), cs.size()};

        return ret;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC
    pipeline_state::terrain_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                                 ID3D12RootSignature *rs,
                                 const std::vector<uint8_t> &vs,
                                 const std::vector<uint8_t> &hs,
                                 const std::vector<uint8_t> &ds,
                                 const std::vector<uint8_t> &ps) {
        auto ret = default_desc(ed, ed_cnt, rs, vs, ps);
        ret.HS = {hs.data(), hs.size()};
        ret.DS = {ds.data(), ds.size()};
        ret.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
        return ret;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC
    pipeline_state::terrain_ref_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                                     ID3D12RootSignature *rs,
                                     const std::vector<uint8_t> &vs,
                                     const std::vector<uint8_t> &hs,
                                     const std::vector<uint8_t> &ds,
                                     const std::vector<uint8_t> &ps) {
        auto ret = terrain_desc(ed, ed_cnt, rs, vs, hs, ds, ps);
        D3D12_DEPTH_STENCIL_DESC ref_dss = {};
        ref_dss.DepthEnable = true;
        ref_dss.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        ref_dss.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
        ref_dss.StencilEnable = true;
        ref_dss.StencilReadMask = 0xff;
        ref_dss.StencilWriteMask = 0xff;

        ref_dss.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_EQUAL;
        ref_dss.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        ref_dss.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        ref_dss.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

        ref_dss.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        ref_dss.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
        ref_dss.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        ref_dss.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;

        ret.DepthStencilState = ref_dss;
        ret.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
        ret.RasterizerState.FrontCounterClockwise = true;

        return ret;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC
    pipeline_state::skybox_desc(D3D12_INPUT_ELEMENT_DESC *ed, UINT ed_cnt,
                                ID3D12RootSignature *rs,
                                const std::vector<uint8_t> &vs,
                                const std::vector<uint8_t> &ps) {
        auto ret = default_desc(ed, ed_cnt, rs, vs, ps);
        ret.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
        ret.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
        return ret;
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC
    pipeline_state::dynamic_shadow_desc(D3D12_INPUT_ELEMENT_DESC *ed,
                                        UINT ed_cnt,
                                        ID3D12RootSignature *rs,
                                        const std::vector<uint8_t> &vs,
                                        const std::vector<uint8_t> &ps) {
        auto ret = default_desc(ed, ed_cnt, rs, vs, ps);
        ret.RasterizerState.DepthBias = 100000;
        ret.RasterizerState.DepthBiasClamp = 0.f;
        ret.RasterizerState.SlopeScaledDepthBias = 1.f;
        ret.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
        ret.SampleDesc.Count = 1;
        ret.NumRenderTargets = 0;
        return ret;
    }
}