#pragma once
#include <vector>
#include <memory>

#include "cmn/D11Headers.h"
#include "DeviceD11.h"
#include "../ITextureManager.h"


namespace CPR::GFX::D11
{
	class ITextureManager : public GFX::ITextureManager
	{
	public:
		virtual ID3D11ShaderResourceView* GetSRV(ResourceIndex index) = 0;
	};

	class TextureManagerD11 : public ITextureManager
	{
	private:
		struct TextureViews
		{
			ID3D11ShaderResourceView* srv = nullptr;
			ID3D11UnorderedAccessView* uav = nullptr;
			ID3D11RenderTargetView* rtv = nullptr;
			ID3D11DepthStencilView* dsv = nullptr;
		};

	public:
		TextureManagerD11(std::shared_ptr<IDevice>);
		~TextureManagerD11();

		ResourceIndex AddTexture(void* textureData, const TextureInfo& textureInfo) override;

		ID3D11ShaderResourceView* GetSRV(ResourceIndex index) override;

	private:
		bool TranslateFormatInfo(const FormatInfo& formatInfo, DXGI_FORMAT& toSet);
		D3D11_USAGE DetermineUsage(unsigned int bindingFlags);
		UINT TranslateBindFlags(unsigned int bindingFlags);
		bool CreateDescription(const TextureInfo& textureInfo, D3D11_TEXTURE2D_DESC& toSet);
		void CreateResourceViews(ID3D11Texture2D* texture, unsigned int bindingFlags, TextureViews& toSet);

	private:
		struct StoredTexture
		{
			ID3D11Texture2D* interfacePtr = nullptr;
			TextureViews views;
		};

		std::shared_ptr<IDevice> device;
		std::vector<StoredTexture> textures;
	};
}
