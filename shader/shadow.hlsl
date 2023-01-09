#include "light.hlsl"
#include "register.hlsl"

Texture2D tex : register(t2);

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct VS_OUT
{
    float4 pos : SV_Position;
    float2 uv : TEXCOORD;
};

VS_OUT VS(VS_IN input) {
    VS_OUT output;

    float4x4 wvp = mul(w, vp);
    output.pos = mul(float4(input.pos, 1.f), wvp);
    output.uv = input.uv;

    return output;
}

void PS(VS_OUT input) {
    float4 diffuse = tex.Sample(sam_lw, input.uv) * materials[obj_pad0].diffuse_albedo;
    clip(diffuse.w = .1f);
}