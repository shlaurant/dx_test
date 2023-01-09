#ifndef GUARD_LIGHT
#define GUARD_LIGHT

#define LIGHT_COUNT 50

struct material {
    float4 diffuse_albedo;
    float3 fresnel_r0;
    float roughness;
};

struct light {
    int type;
    float3 color;
    float fo_start;
    float3 direction;
    float fo_end;
    float3 position;
    float spot_pow;
    float3 pad0;
};

float attenuation_linear(float d, float s, float e){
    return saturate((e - d)/(e - s));
}

float3 fresnel_shlick(float3 r0, float3 normal, float3 light){
    float cos = saturate(dot(normal, light));
    float t = 1.0f - cos;
    float3 ret = r0 + (1.0f - r0)*(t*t*t*t*t);
    return ret;
}

float3 blinn_phong(float3 light, float3 light_v, float3 normal, float3 to_eye,
 material mat){
    const float m = 256.0f * saturate(1.0 - mat.roughness);
    float3 half_vec = normalize(to_eye + light_v);

    float roughness_factor = (m+8.0f) * pow(max(dot(half_vec, normal), 0.0f), m);
    float3 fresnel_factor = fresnel_shlick(mat.fresnel_r0, half_vec, light_v);
    float3 specular_albedo = roughness_factor * fresnel_factor;
    specular_albedo = specular_albedo / (1.0f + specular_albedo);

    return (mat.diffuse_albedo.rgb + specular_albedo) * light;
}

float3 directional_light(light l, material mat, float3 normal, float3 to_eye){
    float3 light_v = -l.direction;
    float3 light_color = l.color * max(dot(light_v, normal), 0.0f);
    return blinn_phong(light_color, light_v, normal, to_eye, mat);
}

float3 point_light(light l, material mat, float3 pos, float3 normal, float3 to_eye){
    float3 light_v = l.position - pos;
    float distance = length(light_v);
    float3 ret;

    if(distance > l.fo_end)
    {
        ret = float3(0.f, 0.f, 0.f);
    } else
    {
        light_v /= distance;
        float3 light_color = l.color * max(dot(light_v, normal), 0.0f);
        light_color *= attenuation_linear(distance, l.fo_start, l.fo_end);
        ret = blinn_phong(light_color, light_v, normal, to_eye, mat);
    }

    return ret;
}

float3 spot_light(light l, material mat, float3 pos, float3 normal, float3 to_eye){
    float3 light_v = l.position - pos;
    float distance = length(light_v);
    float3 ret;

    if(distance > l.fo_end)
    {
        ret = float3(0.f, 0.f, 0.f);
    } else
    {
        light_v /= distance;
        float3 light_color = l.color * max(dot(light_v, normal), 0.0f);
        light_color *= attenuation_linear(distance, l.fo_start, l.fo_end);

        light_color *= pow(max(dot(-light_v, l.direction), 0.0f), l.spot_pow);

        ret = blinn_phong(light_color, light_v, normal, to_eye, mat);
    }

    return ret;
}

float4 calc_light(light lights[LIGHT_COUNT], int active_cnt, material mat, float3 pos, float3 normal, float3 to_eye){
    float3 ret;

    for(int i = 0; i < active_cnt; ++i){
        light l = lights[i];
        if(l.type == 0) ret += directional_light(l, mat, normal, to_eye);
        if(l.type == 1) ret += point_light(l, mat, pos, normal, to_eye);
        if(l.type == 2) ret += spot_light(l, mat, pos, normal, to_eye);
        if(l.type == 3) ret += l.color * mat.diffuse_albedo.rgb;
    }

    return float4(ret, 1.0f);
}

#endif