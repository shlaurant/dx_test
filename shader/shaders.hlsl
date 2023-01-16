#include "light.hlsl"
#include "register.hlsl"
#include "util.hlsl"

Texture2D tex : register(t2);
Texture2D tex_n : register(t3);

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
    float4 pos_w : POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
};

VS_OUT VS_Main(VS_IN input)
{
    VS_OUT output;

#ifdef REFLECTION
    float4x4 world = mul(w, reflection_matrix[0]);
#elif SHADOW
    float4x4 world = mul(w, shadow_matrix);
#else
    float4x4 world = w;
#endif

#ifdef SKINNED
    // float3 pos_t = float3(0.f, 0.f, 0.f);
    // float3 normal_t = float3(0.f, 0.f, 0.f);
    // float3 tangent_t = float3(0.f, 0.f, 0.f);
    //
    // [unroll]
    // for(int i = 0; i < 4; ++i){
    //     pos_t += input.weights[i] * mul(float4(input.pos, 1.f) ,final_matrices[input.indices[i]]).xyz;
    //     normal_t += input.weights[i] * mul(input.normal, (float3x3) final_matrices[input.indices[i]]);
    //     tangent_t += input.weights[i] * mul(input.tangent, (float3x3) final_matrices[input.indices[i]]);
    // }

    float3 pos_t = mul(float4(input.pos, 1.f), final_matrices[0]).xyz;
    float3 normal_t = input.normal;
    float3 tangent_t = input.tangent;

    input.pos = pos_t;
    input.normal = normal_t;
    input.tangent = tangent_t;

#endif

    output.pos_w = mul(float4(input.pos, 1.0f), world);
    float4x4 wvp = mul(world, vp);
    output.pos = mul(float4(input.pos, 1.0f), wvp);
    output.uv = input.uv;
    output.normal = mul(input.normal, (float3x3)world);
    output.tangent = mul(input.tangent, (float3x3)world);

    return output;
}

float4 PS_Main(VS_OUT input) : SV_Target
{
    float4 diffuse_albedo = tex.Sample(sam_aw, input.uv) * materials[obj_pad0].diffuse_albedo;
    clip(diffuse_albedo.w * materials[obj_pad0].diffuse_albedo.w - 0.1f);

#ifdef SHADOW
    float4 color = float4(0.f, 0.f, 0.f, 0.5f);
#else
    input.normal = normalize(input.normal);
    float4 normal_sample = tex_n.Sample(sam_lw, input.uv);
    float3 normal = calc_w_normal((float3)normal_sample, input.normal, input.tangent);


    float4 shadow_pos = mul(input.pos_w, shadow_uv_matrix);
    float shadow_factor = calc_shadow_factor(shadow_pos);


    material new_mat = {diffuse_albedo, materials[obj_pad0].fresnel_r0, materials[obj_pad0].roughness};
    float3 to_eye = normalize(camera_pos - input.pos_w.xyz);
    float4 light_color = calc_light(lights, active_light_counts, new_mat, input.pos_w.xyz, normal, to_eye, shadow_factor);
    float4 color = light_color;


    float3 ref = reflect(-to_eye, normal);
    float4 cube_color = cube_map.Sample(sam_lw, ref);
    color.rgb += (1.f-new_mat.roughness) * fresnel_shlick(new_mat.fresnel_r0, normal, ref) * cube_color.rgb;


    color.w = diffuse_albedo.w;
#endif
    return color;
}