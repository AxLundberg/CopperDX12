#pragma once
#include <d3d11_4.h>
#include <vector>

#include "cmn/D11Headers.h"


namespace CPR::GFX::D11
{
	enum class TexelComponentCount
	{
		SINGLE,
		QUAD
	};

	enum class TexelComponentSize
	{
		BYTE,
		WORD
	};

	enum class TexelComponentType
	{
		FLOAT,
		UNORM,
		DEPTH,
	};

	struct FormatInfo
	{
		TexelComponentCount componentCount = TexelComponentCount::QUAD;
		TexelComponentSize componentSize = TexelComponentSize::BYTE;
		TexelComponentType componentType = TexelComponentType::UNORM;
	};

	enum TextureBinding
	{
		NONE = 0,
		SHADER_RESOURCE = 1,
		UNORDERED_ACCESS = 2,
		RENDER_TARGET = 4,
		DEPTH_STENCIL = 8
	};

	struct TextureInfo
	{
		unsigned int mipLevels = 1;
		unsigned int baseTextureWidth = 0;
		unsigned int baseTextureHeight = 0;
		FormatInfo format;
		unsigned int bindingFlags = TextureBinding::NONE;
	};


	class TextureManagerD11
	{
	private:
		struct TextureViews
		{
			ID3D11ShaderResourceView* srv = nullptr;
			ID3D11UnorderedAccessView* uav = nullptr;
			ID3D11RenderTargetView* rtv = nullptr;
			ID3D11DepthStencilView* dsv = nullptr;
		};

		struct StoredTexture
		{
			ID3D11Texture2D* interfacePtr = nullptr;
			TextureViews views;
		};

		ID3D11Device* device = nullptr;
		std::vector<StoredTexture> textures;

		bool TranslateFormatInfo(const FormatInfo& formatInfo,
			DXGI_FORMAT& toSet);
		D3D11_USAGE DetermineUsage(unsigned int bindingFlags);
		UINT TranslateBindFlags(unsigned int bindingFlags);
		bool CreateDescription(const TextureInfo& textureInfo,
			D3D11_TEXTURE2D_DESC& toSet);
		bool CreateResourceViews(ID3D11Texture2D* texture, unsigned int bindingFlags,
			TextureViews& toSet);

	public:
		void Initialise(ID3D11Device* deviceToUse);

		ResourceIndex AddTexture(void* textureData,
			const TextureInfo& textureInfo);

		ID3D11ShaderResourceView* GetSRV(ResourceIndex index);
	};
}
