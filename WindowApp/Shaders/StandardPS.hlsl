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
	float4 background;
	float4 blueGround;
	float4 greenGround;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 diffuseMaterial = diffuseTexture.Sample(clampSampler, input.uv).xyz;
	float diffBackground = length(diffuseMaterial - backgroundColor);
	float diffBlueGround = length(diffuseMaterial - blueGroundColor);
	float diffGreenGround = length(diffuseMaterial - greenGroundColor);
	
	//if(diffBackground < diffBlueGround && diffBackground < diffGreenGround)
	//	return float4(background.x, background.y, background.z, 1.f);
	//else if(diffBlueGround < diffBackground && diffBlueGround < diffGreenGround)
	//	return float4(blueGround.x, blueGround.y, blueGround.z, 1.f);
	//else
	//	return float4(greenGround.x, greenGround.y, greenGround.z, 1.f);

	
	return float4(diffuseMaterial, 1.0f);
}