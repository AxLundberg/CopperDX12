struct PixelShaderInput
{
	float4 position : SV_POSITION;
	float4 worldPos : WORLD_POSITION;
	float2 uv : UV;
	float3 normal : NORMAL;
};

struct PointLight
{
	float3 position;
	float3 colour;
};

static const float3 backgroundColor = {0.f, 0.f, 0.f};
static const float3 blueGroundColor = {0.f, 0.65882f, 0.95294f};
static const float3 greenGroundColor = {0.0549f, 0.81961f, 0.27059f};

Texture2D diffuseTexture : register(t0);
Texture2D specularTexture : register(t1);

StructuredBuffer<PointLight> lights : register(t2);

sampler clampSampler : register(s0);

cbuffer CameraPos : register(b0)
{
	float3 cameraPos;
}

cbuffer Imgui : register(b1)
{
	float3 background;
	float3 blue;
	float3 green;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 diffuseMaterial = diffuseTexture.Sample(clampSampler, input.uv).xyz;
	float3 colorDifference = diffuseMaterial - blueGroundColor;
	const float threshold = 0.25f;

	if(length(colorDifference) < threshold){
		//return float4(blueGroundColor, 1.f);
		return float4(background.x, background.y, background.z, 1.f);
	}
	return float4(diffuseMaterial, 1.0f);
}