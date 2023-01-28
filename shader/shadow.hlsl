#include "light.hlsl"
#include "register.hlsl"

Texture2D tex : register(t2);

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float4 weights : WEIGHTS;
    float4 indices : INDICES;
};

struct VS_OUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

VS_OUT VS(VS_IN input) {
    VS_OUT output;

    #ifdef SKINNED
        float3 pos_t = float3(0.f, 0.f, 0.f);

        [unroll]
        for(int i = 0; i < 4; ++i){
            pos_t += input.weights[i] * mul(float4(input.pos, 1.f) ,final_matrices[input.indices[i]]).xyz;
        }

        input.pos = pos_t;
    #endif

    float4x4 wvp = mul(w, light_vp);
    output.pos = mul(float4(input.pos, 1.f), wvp);
    output.uv = input.uv;

    return output;
}

void PS(VS_OUT input) {
    float4 diffuse = tex.Sample(sam_lw, input.uv) * materials[obj_pad0].diffuse_albedo;
    clip(diffuse.w - .1f);
}