cbuffer global : register(b0){
    int radius;

    float w0;
    float w1;
    float w2;
    float w3;
    float w4;
    float w5;
    float w6;
    float w7;
    float w8;
    float w9;
    float w10;
}

Texture2D input : register(t0);
RWTexture2D<float4> output : register(u0);

groupshared float4 cache[266];

[numthreads(256, 1, 1)]
void CS_Blur_H(int3 gtid:SV_GroupThreadID, int3 dtid: SV_DispatchThreadID){
    float weights[11] = {w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10};

    cache[gtid.x + 5] = input[min(dtid.xy, input.Length.xy-1)];
    if(gtid.x < 5){
        int x = max(0, dtid.x-5);
        cache[gtid.x] = input[int2(x, dtid.y)];
    }

    if(gtid.x >= 251){
        int x = min(input.Length.x-1, dtid.x+5);
        cache[gtid.x + 10] = input[int2(x, dtid.y)];
    }

    GroupMemoryBarrierWithGroupSync();

    float4 blurred_color = float4(0, 0, 0, 0);
    for(int i = - 5; i <= 5; ++i){
        blurred_color += weights[i+5] * cache[i + gtid.x + 5];

        output[dtid.xy] = blurred_color;
    }
}

[numthreads(1, 256, 1)]
void CS_Blur_V(int3 gtid:SV_GroupThreadID, int3 dtid: SV_DispatchThreadID){
    float weights[11] = {w0, w1, w2, w3, w4, w5, w6, w7, w8, w9, w10};

    cache[gtid.y + 5] = input[min(dtid.xy, input.Length.xy-1)];
    if(gtid.y < 5){
        int y = max(0, dtid.y-5);
        cache[gtid.y] = input[int2(dtid.x, y)];
    }

    if(gtid.y >= 251){
        int y = min(input.Length.y-1, dtid.y+5);
        cache[gtid.y + 10] = input[int2(dtid.x, y)];
    }

    GroupMemoryBarrierWithGroupSync();

    float4 blurred_color = float4(0, 0, 0, 0);
    for(int i = - 5; i <= 5; ++i){
        blurred_color += weights[i+5] * cache[i + gtid.y + 5];

        output[dtid.xy] = blurred_color;
    }
}