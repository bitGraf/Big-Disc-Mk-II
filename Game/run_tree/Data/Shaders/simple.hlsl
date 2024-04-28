// #define BATCH_AMOUNT 1024 // <- set outside shader

cbuffer cbConstants : register(b0) {
    uint  batch_idx;
    uint  tex_diffuse;
    float alpha;
    float tex_scale;
};

cbuffer cbPerObject : register(b1) {
    float4x4 r_Transform;
    float3   u_color;
};

cbuffer cbPerFrame : register(b2) {
    float4x4 r_VP;
};

//Texture2D    tex_map : register(t0);
//SamplerState sam     : register(s0);

struct VertexIn
{
    float3 a_Position    : POSITION;
    float3 a_Normal      : NORMAL;
    float3 a_Tangent     : TANGENT;
    float3 a_Bitangent   : BITANGENT;
    float2 a_TexCoord    : TEXCOORD;
    //int4   a_BoneIndices : BONEIDX;
    //float4 a_BoneWeights : BONEWGT;
};

struct VertexOut
{
    float4 PosH     : SV_POSITION;
    float3 Norm     : NORMAL;
    float2 TexCoord : TEXCOORD;
};

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    // Transform to homogeneous clip space.
    vout.PosH = mul(mul(r_VP, r_Transform), float4(vin.a_Position, 1.0f));

    // Just pass vertex color into the pixel shader.
    //vout.Color = float4(vin.Color, 1.0f);
    vout.Norm = vin.a_Normal;

    vout.TexCoord = vin.a_TexCoord;

    return vout;
}

float sRGB(float x) {
    if (x <= 0.0031308) {
        return 12.92 * x;
    } else {
        return 1.055*pow(x,(1.0 / 2.4) ) - 0.055;
    }
}

float4 sRGB_float4(float4 v) {
    // alpha is already linear?
    return float4(
        sRGB(v.r),
        sRGB(v.g),
        sRGB(v.b),
        v.a
    );
}

float4 PS(VertexOut pin) : SV_Target
{
    //float4 tex_color = tex_map.Sample(sam, pin.TexCoord);
    //clip(tex_color.a - 0.5f);
    //tex_color = sRGB_float4(tex_color);
    //return pin.Color * tex_color;
    //return pin.Color;
    //float4 mix_color = (pin.Color * (1 - tex_diffuse)) + (tex_color * tex_diffuse);

    //return float4(mix_color.rgb, mix_color.a*alpha);
    //return float4(per_obj[batch_idx].u_color, 1.0f);
    //return float4(1.0, 0.5, 0.3, 1.0f);
    return float4(u_color * dot(pin.Norm, float3(0.0f, 1.0f, 1.0f)), 1.0f);
}