#include "TextureManagerD11.h"
#include "../cmn/GraphicsError.h"

namespace CPR::GFX::D11
{
	TextureManagerD11::TextureManagerD11(std::shared_ptr<IDevice> device)
		:
		device(std::move(device))
	{}

	TextureManagerD11::~TextureManagerD11()
	{
		for (auto& texture : textures)
		{
			texture.interfacePtr->Release();

			if (texture.views.srv != nullptr)
				texture.views.srv->Release();
			if (texture.views.uav != nullptr)
				texture.views.uav->Release();
			if (texture.views.rtv != nullptr)
				texture.views.rtv->Release();
			if (texture.views.dsv != nullptr)
				texture.views.dsv->Release();
		}
	}

	bool TextureManagerD11::TranslateFormatInfo(const FormatInfo& formatInfo,
		DXGI_FORMAT& toSet)
	{
		if (formatInfo.componentCount == TexelComponentCount::SINGLE &&
			formatInfo.componentSize == TexelComponentSize::WORD)
		{
			switch (formatInfo.componentType)
			{
			case TexelComponentType::DEPTH:
				toSet = DXGI_FORMAT_D32_FLOAT;
				break;
			case TexelComponentType::FLOAT:
				toSet = DXGI_FORMAT_R32_FLOAT;
				break;
			default:
				return false;
			}
		}
		else if (formatInfo.componentCount == TexelComponentCount::QUAD &&
			formatInfo.componentSize == TexelComponentSize::WORD)
		{
			switch (formatInfo.componentType)
			{
			case TexelComponentType::FLOAT:
				toSet = DXGI_FORMAT_R32G32B32A32_FLOAT;
				break;
			default:
				return false;
			}
		}
		else if (formatInfo.componentCount == TexelComponentCount::QUAD &&
			formatInfo.componentSize == TexelComponentSize::BYTE)
		{
			switch (formatInfo.componentType)
			{
			case TexelComponentType::UNORM:
				toSet = DXGI_FORMAT_R8G8B8A8_UNORM;
				break;
			default:
				return false;
			}
		}
		else
		{
			return false;
		}

		return true;
	}

	D3D11_USAGE TextureManagerD11::DetermineUsage(unsigned int bindingFlags)
	{
		if (bindingFlags & TextureBinding::UNORDERED_ACCESS ||
			bindingFlags & TextureBinding::RENDER_TARGET ||
			bindingFlags & TextureBinding::DEPTH_STENCIL)
		{
			return D3D11_USAGE_DEFAULT;
		}
		else
		{
			return D3D11_USAGE_IMMUTABLE;
		}
	}

	UINT TextureManagerD11::TranslateBindFlags(unsigned int bindingFlags)
	{
		UINT toReturn = 0;

		if (bindingFlags & TextureBinding::SHADER_RESOURCE)
			toReturn |= D3D11_BIND_SHADER_RESOURCE;
		if (bindingFlags & TextureBinding::UNORDERED_ACCESS)
			toReturn |= D3D11_BIND_UNORDERED_ACCESS;
		if (bindingFlags & TextureBinding::RENDER_TARGET)
			toReturn |= D3D11_BIND_RENDER_TARGET;
		if (bindingFlags & TextureBinding::DEPTH_STENCIL)
			toReturn |= D3D11_BIND_DEPTH_STENCIL;

		return toReturn;
	}

	bool TextureManagerD11::CreateDescription(const TextureInfo& textureInfo,
		D3D11_TEXTURE2D_DESC& toSet)
	{
		toSet.Width = textureInfo.baseTextureWidth;
		toSet.Height = textureInfo.baseTextureHeight;
		toSet.MipLevels = textureInfo.mipLevels;
		toSet.ArraySize = 1;
		toSet.SampleDesc.Count = 1;
		toSet.SampleDesc.Quality = 0;
		toSet.Usage = DetermineUsage(textureInfo.bindingFlags);
		toSet.BindFlags = TranslateBindFlags(textureInfo.bindingFlags);
		toSet.CPUAccessFlags = 0;
		toSet.MiscFlags = 0;

		return TranslateFormatInfo(textureInfo.format, toSet.Format);
	}

	void TextureManagerD11::CreateResourceViews(ID3D11Texture2D* texture,
		unsigned int bindingFlags, TextureViews& toSet)
	{
		if (bindingFlags & TextureBinding::SHADER_RESOURCE)
		{
			device->GetD3D11Device()->CreateShaderResourceView(texture, nullptr, &toSet.srv) >> hrVerify;
		}
		if (bindingFlags & TextureBinding::UNORDERED_ACCESS)
		{
			device->GetD3D11Device()->CreateUnorderedAccessView(texture, nullptr, &toSet.uav) >> hrVerify;
		}
		if (bindingFlags & TextureBinding::RENDER_TARGET)
		{
			device->GetD3D11Device()->CreateRenderTargetView(texture, nullptr, &toSet.rtv) >> hrVerify;
		}
		if (bindingFlags & TextureBinding::DEPTH_STENCIL)
		{
			device->GetD3D11Device()->CreateDepthStencilView(texture, nullptr, &toSet.dsv) >> hrVerify;
		}
	}

	ResourceIndex TextureManagerD11::AddTexture(void* textureData,
		const TextureInfo& textureInfo)
	{
		D3D11_TEXTURE2D_DESC desc;
		bool result = CreateDescription(textureInfo, desc);
		if (result == false)
			return ResourceIndex(-1);

		D3D11_SUBRESOURCE_DATA resourceData;
		resourceData.pSysMem = textureData;
		resourceData.SysMemPitch = textureInfo.baseTextureWidth;
		resourceData.SysMemPitch *= textureInfo.format.componentCount ==
			TexelComponentCount::QUAD ? 4 : 1;
		resourceData.SysMemPitch *= textureInfo.format.componentSize ==
			TexelComponentSize::WORD ? 4 : 1;

		ID3D11Texture2D* interfacePtr = nullptr;
		device->GetD3D11Device()->CreateTexture2D(&desc, &resourceData, &interfacePtr);

		TextureViews views;
		CreateResourceViews(interfacePtr, textureInfo.bindingFlags, views);

		textures.push_back({ interfacePtr, views });
		return ResourceIndex(textures.size() - 1);
	}

	ID3D11ShaderResourceView* TextureManagerD11::GetSRV(ResourceIndex index)
	{
		return textures[index].views.srv;
	}
}
