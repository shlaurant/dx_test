#include "light.hlsl"
#include "register.hlsl"

Texture2DArray tex_arr : register(t2);

struct VS_IN
{
    float3 position : POSITION;
    float2 size : SIZE;
};

struct VS_OUT
{
    float3 position:POSITION;
    float2 size : SIZE;
};

struct GS_OUT
{
    float4 position:SV_POSITION;
    float4 pos_w:POSITION;
    float3 normal:NORMAL;
    float2 uv:TEXCOORD;
    uint id : SV_PrimitiveID;
};

VS_OUT VS_Main(VS_IN input)
{
    VS_OUT output;
    output.position = input.position;
    output.size = input.size;
    return output;
}

[maxvertexcount(4)]
void GS_Main(point VS_OUT input[1], uint id:SV_PrimitiveID, inout TriangleStream<GS_OUT> stream)
{
    float3 up = float3(0.f, 1.f, 0.f);
    float3 look = camera_pos - obj_position;
    look.y = 0.f;
    look = normalize(look);
    float3 left = cross(up, look);

    float4x4 wvp = mul(w, vp);

    GS_OUT gout;

    gout.position = float4(input[0].position + (input[0].size.x / 2) * left, 1.f);
    gout.pos_w = mul(gout.position, w);
    gout.position = mul(gout.position, wvp);
    gout.normal = look;
    gout.uv = float2(0.f, 1.f);
    gout.id = id;
    stream.Append(gout);

    gout.position = float4(input[0].position + (input[0].size.x / 2) * left + input[0].size.y * up, 1.f);
    gout.position = mul(gout.position, wvp);
    gout.pos_w = gout.position;
    gout.normal = look;
    gout.uv = float2(0.f, 0.f);
    gout.id = id;
    stream.Append(gout);

    gout.position = float4(input[0].position - (input[0].size.x / 2) * left, 1.f);
    gout.position = mul(gout.position, wvp);
    gout.pos_w = gout.position;
    gout.normal = look;
    gout.uv = float2(1.f, 1.f);
    gout.id = id;
    stream.Append(gout);

    gout.position = float4(input[0].position - (input[0].size.x / 2) * left + input[0].size.y * up, 1.f);
    gout.position = mul(gout.position, wvp);
    gout.pos_w = gout.position;
    gout.normal = look;
    gout.uv = float2(1.f, 0.f);
    gout.id = id;
    stream.Append(gout);
}

float4 PS_Main(GS_OUT input):SV_Target
{
    float3 uvw = float3(input.uv, input.id % 3);
    float4 diffuse_albedo = tex_arr.Sample(sam_aw, uvw) * materials[obj_pad0].diffuse_albedo;
    clip(diffuse_albedo.w * materials[obj_pad0].diffuse_albedo.w - 0.1f);

#ifdef SHADOW
    float4 color = float4(0.f, 0.f, 0.f, 0.5f);
#else
    input.normal = normalize(input.normal);

    material new_mat = {diffuse_albedo, materials[obj_pad0].fresnel_r0, materials[obj_pad0].roughness};
    float3 to_eye = normalize(camera_pos - input.pos_w.xyz);
    float4 light_color = calc_light(lights, active_light_counts, new_mat, input.pos_w.xyz, input.normal, to_eye);
    float4 color = light_color;
    color.w = diffuse_albedo.w;
#endif

    return color;
}