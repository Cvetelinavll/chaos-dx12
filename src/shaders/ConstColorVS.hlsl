struct VSInput {
    float2 position : POSITION;
};

struct PSInput {
    float4 position : SV_POSITION;
};

PSInput VSMain(VSInput inputVertex) {
    return PSInput(float4(inputVertex.position, 0.0, 1.0));
}