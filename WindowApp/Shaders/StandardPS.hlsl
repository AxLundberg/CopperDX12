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
	float3 imguiPos;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	float3 diffuseMaterial = diffuseTexture.Sample(clampSampler, input.uv).xyz;
	
	if(imguiPos.x > 0.5f)
		return float4(imguiPos.x, imguiPos.y, imguiPos.z, 1.f);

	return float4(diffuseMaterial, 1.0f);
}