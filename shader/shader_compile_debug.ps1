fxc "shaders.hlsl" /Od /Zi /T vs_5_1 /E "VS_Main" /Fo "debug/vs.cso"
fxc "shaders.hlsl" /D SKINNED /Od /Zi /T vs_5_1 /E "VS_Main" /Fo "debug/vs_skin.cso"
fxc "shaders.hlsl" /D REFLECTION /Od /Zi /T vs_5_1 /E "VS_Main" /Fo "debug/vs_ref.cso"
fxc "shaders.hlsl" /D SKINNED /D REFLECTION /Od /Zi /T vs_5_1 /E "VS_Main" /Fo "debug/vs_skin_ref.cso"
fxc "shaders.hlsl" /D SHADOW /Od /Zi /T vs_5_1 /E "VS_Main" /Fo "debug/vs_sha.cso"

fxc "shaders.hlsl" /Od /Zi /T ps_5_1 /E "PS_Main" /Fo "debug/ps.cso"
fxc "shaders.hlsl" /D SHADOW /Od /Zi /T ps_5_1 /E "PS_Main" /Fo "debug/ps_sha.cso"

fxc "billboard.hlsl" /Od /Zi /T vs_5_1 /E "VS_Main" /Fo "debug/vs_bill.cso"
fxc "billboard.hlsl" /Od /Zi /T gs_5_1 /E "GS_Main" /Fo "debug/gs_bill.cso"
fxc "billboard.hlsl" /Od /Zi /T ps_5_1 /E "PS_Main" /Fo "debug/ps_bill.cso"

fxc "blur.hlsl" /Od /Zi /T cs_5_1 /E "CS_Blur_H" /Fo "debug/cs_blur_h.cso"
fxc "blur.hlsl" /Od /Zi /T cs_5_1 /E "CS_Blur_V" /Fo "debug/cs_blur_v.cso"

fxc "terrain.hlsl" /Od /Zi /T vs_5_1 /E "VS" /Fo "debug/vs_terrain.cso"
fxc "terrain.hlsl" /Od /Zi /T hs_5_1 /E "CPHS" /Fo "debug/hs_terrain.cso"
fxc "terrain.hlsl" /Od /Zi /T ds_5_1 /E "DS" /Fo "debug/ds_terrain.cso"
fxc "terrain.hlsl"  /D REFLECTION /Od /Zi /T ds_5_1 /E "DS" /Fo "debug/ds_terrain_ref.cso"
fxc "terrain.hlsl" /Od /Zi /T ps_5_1 /E "PS" /Fo "debug/ps_terrain.cso"

fxc "skybox.hlsl" /Od /Zi /T vs_5_1 /E "VS" /Fo "debug/vs_skybox.cso"
fxc "skybox.hlsl" /Od /Zi /T ps_5_1 /E "PS" /Fo "debug/ps_skybox.cso"

fxc "shadow.hlsl" /Od /Zi /T vs_5_1 /E "VS" /Fo "debug/vs_shadow.cso"
fxc "shadow.hlsl" /D SKINNED /Od /Zi /T vs_5_1 /E "VS" /Fo "debug/vs_shadow_skin.cso"
fxc "shadow.hlsl" /Od /Zi /T ps_5_1 /E "PS" /Fo "debug/ps_shadow.cso"