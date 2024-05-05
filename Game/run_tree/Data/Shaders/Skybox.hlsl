cbuffer cbConstants : register(b0) {
    uint _placeholder1;
};

cbuffer cbPerObject : register(b1) {
    uint _placeholder2;
};

cbuffer cbPerFrame : register(b2) {
    float4x4 r_View;
    float4x4 r_Projection;

    uint r_ToneMap;
    uint r_GammaCorrect;
};

TextureCube  u_skybox  : register(t0);
SamplerState u_sampler : register(s0);

struct VertexIn
{
    float3 a_Position    : POSITION;
    //float3 a_Normal      : NORMAL;
    //float3 a_Tangent     : TANGENT;
    //float3 a_Bitangent   : BITANGENT;
    //float2 a_TexCoord    : TEXCOORD;
    //int4   a_BoneIndices : BONEIDX;
    //float4 a_BoneWeights : BONEWGT;
};

struct VertexOut
{
    float4 PosH     : SV_POSITION;
    //float3 Norm     : NORMAL;
    float3 TexCoord : TEXCOORD;
};

float3 ToneMap(float3 colorIn);

VertexOut VS(VertexIn vin)
{
    VertexOut vout;

    // Transform to homogeneous clip space.
    vout.PosH = mul(mul(r_Projection, r_View), float4(vin.a_Position, 1.0f));
    vout.TexCoord = vin.a_Position;

    return vout;
}

float4 PS(VertexOut pin) : SV_Target
{
    float4 tex_color = u_skybox.Sample(u_sampler, pin.TexCoord);
    float4 frag_color = float4(ToneMap(tex_color.rgb), 1.0);

    // Gamma correct
    float gamma = 2.2;
    if (r_GammaCorrect > 0.5)
    frag_color.rgb = pow(frag_color.rgb, float3(1.0/gamma, 1.0/gamma, 1.0/gamma));

    return frag_color;
}


// Tone Mapping Functions
static const float3x3 ACESInputMat = {
    {0.59719, 0.35458, 0.04823},
    {0.07600, 0.90834, 0.01566},
    {0.02840, 0.13383, 0.83777}
};

static const float3x3 ACESOutputMat = {
    { 1.60475, -0.53108, -0.07367},
    {-0.10208,  1.10813, -0.00605},
    {-0.00327, -0.07276,  1.07602}
};

float3 RRTAndODTFit(float3 v)
{
    float3 a = v * (v + 0.0245786f) - 0.000090537f;
    float3 b = v * (0.983729f * v + 0.4329510f) + 0.238081f;
    return a / b;
}

float3 ACESFitted(float3 color) {
    color = mul(transpose(ACESInputMat), color);

    // Apply RRT and ODT
    color = RRTAndODTFit(color);

    color = mul(transpose(ACESOutputMat), color);

    // Clamp to [0,1]
    color = clamp(color, 0.0, 1.0);

    return color;
}

float3 ToneMap(float3 colorIn) {
    //return colorIn;
    if (r_ToneMap > 0.5) {
        return ACESFitted(colorIn);
        //return reinhard_extended_luminance(colorIn, 5.0f);
        //return uncharted2_filmic(colorIn);
        //return ACES_approx(colorIn);
        //return MGSVToneMap(colorIn);
    } else {
        return colorIn;
    }
}