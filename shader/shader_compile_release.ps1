fxc "shaders.hlsl"  /T vs_5_1 /E "VS_Main" /Fo "release/vs.cso"
fxc "shaders.hlsl" /D SKINNED /T vs_5_1 /E "VS_Main" /Fo "release/vs_skin.cso"
fxc "shaders.hlsl" /D REFLECTION /T vs_5_1 /E "VS_Main" /Fo "release/vs_ref.cso"
fxc "shaders.hlsl" /D SKINNED /D REFLECTION /T vs_5_1 /E "VS_Main" /Fo "release/vs_skin_ref.cso"
fxc "shaders.hlsl" /D SHADOW  /T vs_5_1 /E "VS_Main" /Fo "release/vs_sha.cso"

fxc "shaders.hlsl"  /T ps_5_1 /E "PS_Main" /Fo "release/ps.cso"
fxc "shaders.hlsl" /D SHADOW  /T ps_5_1 /E "PS_Main" /Fo "release/ps_sha.cso"

fxc "billboard.hlsl"  /T vs_5_1 /E "VS_Main" /Fo "release/vs_bill.cso"
fxc "billboard.hlsl"  /T gs_5_1 /E "GS_Main" /Fo "release/gs_bill.cso"
fxc "billboard.hlsl"  /T ps_5_1 /E "PS_Main" /Fo "release/ps_bill.cso"

fxc "blur.hlsl"  /T cs_5_1 /E "CS_Blur_H" /Fo "release/cs_blur_h.cso"
fxc "blur.hlsl"  /T cs_5_1 /E "CS_Blur_V" /Fo "release/cs_blur_v.cso"

fxc "terrain.hlsl"  /T vs_5_1 /E "VS" /Fo "release/vs_terrain.cso"
fxc "terrain.hlsl"  /T hs_5_1 /E "CPHS" /Fo "release/hs_terrain.cso"
fxc "terrain.hlsl"  /T ds_5_1 /E "DS" /Fo "release/ds_terrain.cso"
fxc "terrain.hlsl"  /T ps_5_1 /E "PS" /Fo "release/ps_terrain.cso"

fxc "skybox.hlsl"  /T vs_5_1 /E "VS" /Fo "release/vs_skybox.cso"
fxc "skybox.hlsl"  /T ps_5_1 /E "PS" /Fo "release/ps_skybox.cso"

fxc "shadow.hlsl"  /T vs_5_1 /E "VS" /Fo "release/vs_shadow.cso"
fxc "shadow.hlsl" /D SKINNED /T vs_5_1 /E "VS" /Fo "release/vs_shadow_skin.cso"
fxc "shadow.hlsl"  /T ps_5_1 /E "PS" /Fo "release/ps_shadow.cso"