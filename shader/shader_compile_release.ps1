fxc "shaders.hlsl"  /T vs_5_1 /E "VS_Main" /Fo "vs.cso"
fxc "shaders.hlsl" /D SKINNED  /T vs_5_1 /E "VS_Main" /Fo "vs_skin.cso"
fxc "shaders.hlsl" /D REFLECTION  /T vs_5_1 /E "VS_Main" /Fo "vs_ref.cso"
fxc "shaders.hlsl" /D SHADOW  /T vs_5_1 /E "VS_Main" /Fo "vs_sha.cso"

fxc "shaders.hlsl"  /T ps_5_1 /E "PS_Main" /Fo "ps.cso"
fxc "shaders.hlsl" /D SHADOW  /T ps_5_1 /E "PS_Main" /Fo "ps_sha.cso"

fxc "billboard.hlsl"  /T vs_5_1 /E "VS_Main" /Fo "vs_bill.cso"
fxc "billboard.hlsl"  /T gs_5_1 /E "GS_Main" /Fo "gs_bill.cso"
fxc "billboard.hlsl"  /T ps_5_1 /E "PS_Main" /Fo "ps_bill.cso"

fxc "blur.hlsl"  /T cs_5_1 /E "CS_Blur_H" /Fo "cs_blur_h.cso"
fxc "blur.hlsl"  /T cs_5_1 /E "CS_Blur_V" /Fo "cs_blur_v.cso"

fxc "terrain.hlsl"  /T vs_5_1 /E "VS" /Fo "vs_terrain.cso"
fxc "terrain.hlsl"  /T hs_5_1 /E "CPHS" /Fo "hs_terrain.cso"
fxc "terrain.hlsl"  /T ds_5_1 /E "DS" /Fo "ds_terrain.cso"
fxc "terrain.hlsl"  /T ps_5_1 /E "PS" /Fo "ps_terrain.cso"

fxc "skybox.hlsl"  /T vs_5_1 /E "VS" /Fo "vs_skybox.cso"
fxc "skybox.hlsl"  /T ps_5_1 /E "PS" /Fo "ps_skybox.cso"

fxc "shadow.hlsl"  /T vs_5_1 /E "VS" /Fo "vs_shadow.cso"
fxc "shadow.hlsl"  /T ps_5_1 /E "PS" /Fo "ps_shadow.cso"