#include "light.hlsl"
#include "register.hlsl"

struct VS_IN
{
    float3 pos : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
};

struct VS_OUT
{
    float4 pos : SV_Position;
    float3 pos_l : POSITION;
};

VS_OUT VS(VS_IN input) {
    VS_OUT output;

    output.pos_l = input.pos;
    output.pos.xyz = camera_pos + input.pos;
    output.pos.w = 1.f;
    output.pos = mul(output.pos, vp).xyww;

    return output;
}

float4 PS(VS_OUT input) :SV_Target {
    return cube_map.Sample(sam_lw, input.pos_l);
}