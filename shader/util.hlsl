#ifndef _UTIL_
#define _UTIL_

float calc_shadow_factor(float4 shadow_pos) {
    shadow_pos.xyz /= shadow_pos.w;

    float depth = shadow_pos.z;
    uint width, height, mips;
    shadow_map.GetDimensions(0, width, height, mips);

    float dx = 1.0f/(float)width;
    float percent_lit = 0.f;
    const float2 offsets[9] = {
        float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    };

    [unroll]
    for(int i = 0; i < 9; ++i)
    {
           percent_lit += shadow_map.SampleCmpLevelZero(sam_sh, shadow_pos.xy + offsets[i], depth).r;
    }

    return percent_lit / 9.f;
}

float3 calc_w_normal(float3 normal_uv, float3 normal_unit, float3 tangent){
    float3 normal_tb = 2.f * normal_uv - 1.f;
    float3 t = normalize(tangent - dot(tangent, normal_unit) * normal_unit);
    float3 b = cross(normal_unit, t);
    float3x3 tbn = float3x3(t, b, normal_unit);

    return mul(normal_tb, tbn);
}

#endif