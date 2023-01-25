#ifndef GUARD_REGISTER
#define GUARD_REGISTER

#define MAX_BONE_CNT 256

cbuffer globals_scene :register(b0) {
    row_major float4x4 reflection_matrix[10];
    int reflection_count;
    float3 pad0;
    row_major float4x4 shadow_matrix;
};

cbuffer globals_frame :register(b1) {
    row_major float4x4 vp;
    float3 camera_pos;
    int active_light_counts;
    light lights[LIGHT_COUNT];
    row_major float4x4 light_vp;
    row_major float4x4 shadow_uv_matrix;
};

cbuffer object_const :register(b2) {
    float3 obj_position;
    int obj_pad0;
    row_major float4x4 w;
};

cbuffer bone_matrix : register(b3) {
    row_major float4x4 final_matrices[MAX_BONE_CNT];
}

TextureCube cube_map : register(t0);
Texture2D shadow_map : register(t1);
StructuredBuffer<material> materials : register(t0, space1);

SamplerState sam_pw : register(s0);
SamplerState sam_pc : register(s1);
SamplerState sam_pm : register(s2);
SamplerState sam_lw : register(s3);
SamplerState sam_lc : register(s4);
SamplerState sam_lm : register(s5);
SamplerState sam_aw : register(s6);
SamplerState sam_ac : register(s7);
SamplerState sam_am : register(s8);
SamplerComparisonState sam_sh : register(s9);

#endif