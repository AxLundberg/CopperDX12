#pragma once
#include "SpriteBatcher.h"
#include <Core/src/log/Log.h>
#include <Core/src/utl/String.h>
#include <Core/src/gfx/cmn/GraphicsError.h>
#include <d3dcompiler.h>
#include <Core/src/utl/Assert.h>

namespace CPR::GFX::D12
{
	using Microsoft::WRL::ComPtr;
	namespace rn = std::ranges;

	SpriteBatcher::SpriteBatcher(const SPA::DimensionsI& targetDimensions,
		std::shared_ptr<IDeviceChil> pDevice,
		std::shared_ptr<SpriteCodex> pSpriteCodex,
		UINT maxSpriteCount)
		:
		pDevice_{ std::move(pDevice) },
		outputDims_{ (SPA::DimensionsF)targetDimensions },
		pSpriteCodex_{ std::move(pSpriteCodex) },
		maxIndices_{ 6 * maxSpriteCount },
		maxVertices_{ 4 * maxSpriteCount },
		cameraTransform_{ DirectX::XMMatrixIdentity() }
	{
		auto pDeviceInterface = pDevice_->GetD3D12DeviceInterface();
		// root signature
		{
			// define root signature a table of sprite atlas textures
			// in future to reduce root signature binding this should just be merged into a global root descriptor
			// might want to use a bounded range, in which case the root signature will need to be updated when atlases are added
			D3D12_ROOT_PARAMETER rootParameters[2]{};
			// sprite codex
			D3D12_DESCRIPTOR_RANGE descRange{
				.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV,
				.NumDescriptors = UINT_MAX,
				.BaseShaderRegister = 0,
				.RegisterSpace = 0,
				.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND,
			};
			D3D12_ROOT_PARAMETER& spriteCodex = rootParameters[0];
			spriteCodex.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			spriteCodex.ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
			spriteCodex.DescriptorTable.NumDescriptorRanges = 1u;
			spriteCodex.DescriptorTable.pDescriptorRanges = &descRange;

			// camera transform
			D3D12_ROOT_PARAMETER& cameraTransform = rootParameters[1];
			cameraTransform.Constants.Num32BitValues = sizeof(DirectX::XMMATRIX) / sizeof(float);
			cameraTransform.Constants.RegisterSpace = 0;
			cameraTransform.Constants.ShaderRegister = 0;
			cameraTransform.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

			// Allow input layout and vertex shader and deny unnecessary access to certain pipeline stages.
			const D3D12_ROOT_SIGNATURE_FLAGS rootSignatureFlags =
				D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_MESH_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_AMPLIFICATION_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
				D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
			// define static sampler
			D3D12_STATIC_SAMPLER_DESC staticSampler = {};
			staticSampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
			staticSampler.AddressU = staticSampler.AddressV = staticSampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
			staticSampler.MipLODBias = 0.0f;
			staticSampler.MaxAnisotropy = 16;
			staticSampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
			staticSampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
			staticSampler.MinLOD = 0;
			staticSampler.MaxLOD = D3D12_FLOAT32_MAX;
			staticSampler.ShaderRegister = 0;
			staticSampler.RegisterSpace = 0;
			staticSampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
			// define root signature with transformation matrix
			D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
			rootSignatureDesc.NumParameters = static_cast<u32>(std::size(rootParameters));
			rootSignatureDesc.pParameters = rootParameters;
			rootSignatureDesc.NumStaticSamplers = 1u;
			rootSignatureDesc.pStaticSamplers = &staticSampler;
			rootSignatureDesc.Flags = rootSignatureFlags;

			// serialize root signature 
			ComPtr<ID3DBlob> signatureBlob;
			ComPtr<ID3DBlob> errorBlob;
			if (const auto hr = D3D12SerializeRootSignature(
				&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1,
				&signatureBlob, &errorBlob); FAILED(hr)) {
				if (errorBlob) {
					auto errorBufferPtr = static_cast<const char*>(errorBlob->GetBufferPointer());
					cprlog.Error(UTL::ToWide(errorBufferPtr)).No_Trace();
				}
				hr >> hrVerify;
			}
			// Create the root signature. 
			pDeviceInterface->CreateRootSignature(0, signatureBlob->GetBufferPointer(),
				signatureBlob->GetBufferSize(), IID_PPV_ARGS(&pRootSignature_)) >> hrVerify;
		}
		// pso (with shaders)
		{
			// static declaration of pso stream structure 
			struct PipelineStateStream
			{
				ID3D12RootSignature* RootSignature;
				D3D12_INPUT_LAYOUT_DESC InputLayout;
				D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
				D3D12_SHADER_BYTECODE VS;
				D3D12_SHADER_BYTECODE PS;
				D3D12_RT_FORMAT_ARRAY RTVFormats;
				DXGI_FORMAT DSVFormat;
			} pipelineStateStream;

			// define the Vertex input layout 
			const D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
				{ "POSITION",		0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TEXCOORD",		0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "TRANSLATION",	0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "SCALE",			0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "ROTATION",		0, DXGI_FORMAT_R32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
				{ "ATLASINDEX",		0, DXGI_FORMAT_R16_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			};

			pipelineStateStream.InputLayout.NumElements = ARRAYSIZE(inputLayout);
			pipelineStateStream.InputLayout.pInputElementDescs = inputLayout;
			// Load the vertex shader. 
			ComPtr<ID3DBlob> pVertexShaderBlob;
			D3DReadFileToBlob(L"VertexShader.cso", &pVertexShaderBlob) >> hrVerify;

			// Load the pixel shader. 
			ComPtr<ID3DBlob> pPixelShaderBlob;
			D3DReadFileToBlob(L"PixelShader.cso", &pPixelShaderBlob) >> hrVerify;

			pipelineStateStream.RootSignature = pRootSignature_.Get();
			pipelineStateStream.VS.pShaderBytecode = pVertexShaderBlob->GetBufferPointer();
			pipelineStateStream.VS.BytecodeLength = pVertexShaderBlob->GetBufferSize();
			pipelineStateStream.PS.pShaderBytecode = pPixelShaderBlob->GetBufferPointer();
			pipelineStateStream.PS.BytecodeLength = pPixelShaderBlob->GetBufferSize();
			pipelineStateStream.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			pipelineStateStream.RTVFormats.NumRenderTargets = 1;
			const DXGI_FORMAT rtvFormats[] = { DXGI_FORMAT_R8G8B8A8_UNORM };
			pipelineStateStream.RTVFormats.RTFormats[0] = rtvFormats[0];
			pipelineStateStream.DSVFormat = DXGI_FORMAT_D32_FLOAT;

			// building the pipeline state object 
			const D3D12_PIPELINE_STATE_STREAM_DESC pipelineStateStreamDesc = {
				sizeof(PipelineStateStream), &pipelineStateStream
			};
			pDeviceInterface->CreatePipelineState(&pipelineStateStreamDesc, IID_PPV_ARGS(&pPipelineState_)) >> hrVerify;
			// filling pso structure
			/*D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
			ZeroMemory(&psoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			psoDesc.pRootSignature = pRootSignature_.Get();
			psoDesc.VS.pShaderBytecode = pVertexShaderBlob->GetBufferPointer();
			psoDesc.VS.BytecodeLength = pVertexShaderBlob->GetBufferSize();
			psoDesc.PS.pShaderBytecode = pPixelShaderBlob->GetBufferPointer();
			psoDesc.PS.BytecodeLength = pPixelShaderBlob->GetBufferSize();
			psoDesc.SampleMask = u32(-1);
			psoDesc.InputLayout.NumElements = ARRAYSIZE(inputLayout);
			psoDesc.InputLayout.pInputElementDescs = inputLayout;
			psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
			psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;

			for (unsigned int i = 0; i < ARRAYSIZE(rtvFormats); ++i)
			{
				psoDesc.RTVFormats[i] = rtvFormats[i];
				D3D12_RENDER_TARGET_BLEND_DESC blendDesc{};
				blendDesc.BlendEnable = false;
				blendDesc.LogicOpEnable = false;
				blendDesc.SrcBlend = D3D12_BLEND_ONE;
				blendDesc.DestBlend = D3D12_BLEND_ZERO;
				blendDesc.BlendOp = D3D12_BLEND_OP_ADD;
				blendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
				blendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
				blendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
				blendDesc.LogicOp = D3D12_LOGIC_OP_NOOP;
				blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
				psoDesc.BlendState.RenderTarget[i] = blendDesc;
			}
			
			psoDesc.BlendState.AlphaToCoverageEnable = false;
			psoDesc.BlendState.IndependentBlendEnable = false;
			psoDesc.SampleDesc.Count = 1;
			psoDesc.SampleDesc.Quality = 0;
			psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

			D3D12_DEPTH_STENCIL_DESC depthDesc{};
			{
				depthDesc.DepthEnable = true;
				depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
				depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
				depthDesc.StencilEnable = false;
				depthDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
				depthDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
				depthDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
				depthDesc.BackFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
				depthDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
				depthDesc.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
				depthDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
				depthDesc.BackFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
				depthDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
				depthDesc.BackFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
			}
			D3D12_STREAM_OUTPUT_DESC streamOutDesc = {};
			{
				streamOutDesc.pSODeclaration = nullptr;
				streamOutDesc.NumEntries = 0;
				streamOutDesc.pBufferStrides = nullptr;
				streamOutDesc.NumStrides = 0;
				streamOutDesc.RasterizedStream = 0;
			}

			psoDesc.DepthStencilState = depthDesc;
			psoDesc.StreamOutput = streamOutDesc;*/
			
			// create index buffer resource
			{
				D3D12_HEAP_PROPERTIES heapProperties = {};
				heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
				heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
				heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
				heapProperties.CreationNodeMask = 0;
				heapProperties.VisibleNodeMask = 0;

				D3D12_RESOURCE_DESC resourceDesc = {};
				resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
				resourceDesc.Alignment = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
				resourceDesc.Width = sizeof(UINT) * maxIndices_;
				resourceDesc.Height = 1;
				resourceDesc.DepthOrArraySize = 1;
				resourceDesc.MipLevels = 1;
				resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
				resourceDesc.SampleDesc.Count = 1;
				resourceDesc.SampleDesc.Quality = 0;
				resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
				resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

				pDeviceInterface->CreateCommittedResource(
					&heapProperties,
					D3D12_HEAP_FLAG_NONE,
					&resourceDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr, IID_PPV_ARGS(&pIndexBuffer_)
				) >> hrVerify;
			}
			// index buffer view
			indexBufferView_ = {
				.BufferLocation = pIndexBuffer_->GetGPUVirtualAddress(),
				.SizeInBytes = (UINT)sizeof(UINT) * maxIndices_,
				.Format = DXGI_FORMAT_R32_UINT,
			};
		}
	}

	SpriteBatcher::~SpriteBatcher() = default;

	void SpriteBatcher::StartBatch(AllocatorListPair cmd, uint64_t frameFenceValue, uint64_t signaledFenceValue)
	{
		// command list/queue stuff
		cmd_ = std::move(cmd);
		frameFenceValue_ = frameFenceValue;
		signaledFenceValue_ = signaledFenceValue;
		// frame resource stuff
		currentFrameResource_ = GetFrameResource_(signaledFenceValue);
		//const auto mapReadRangeNone = CD3DX12_RANGE{ 0, 0 };
		// vertex buffer
		currentFrameResource_->pVertexBuffer->Map(0, nullptr,
			reinterpret_cast<void**>(&pVertexUpload_)) >> hrVerify;
		// write indices reset
		nVertices_ = 0;
		nIndices_ = 0;
	}

	void SpriteBatcher::SetCamera(const SPA::Vec2F& pos, float rot, float scale)
	{
		using namespace DirectX;

		// xform: rotate
		auto transform = XMMatrixRotationZ(rot);
		// xform: translate
		transform = transform * XMMatrixTranslation(pos.x, pos.y, 0.f);
		// xform: scale
		transform = transform * XMMatrixScaling(scale, scale, 1.f);
		// xform: to ndc
		transform = transform * XMMatrixScaling(2.f / outputDims_.width, 2.f / outputDims_.height, 1.f);
		// column major for
		cameraTransform_ = XMMatrixTranspose(transform);
	}

	void SpriteBatcher::Draw(size_t atlasIndex,
		const SPA::RectF& srcInTexcoords,
		const SPA::DimensionsF& destPixelDims,
		const SPA::Vec2F& pos,
		const float rot,
		const SPA::Vec2F& scale)
	{
		using namespace DirectX;

		cpr_assert(nVertices_ + 4 <= maxVertices_);
		cpr_assert(nIndices_ + 6 <= maxIndices_);

		// atlas index 32-bit
		const auto atlasIndex32 = (UINT)atlasIndex;

		// starting dest vertice vectors
		const DirectX::XMFLOAT3 posArray[4]{
			{ 0.f, 0.f, 0.f },
			{ destPixelDims.width, 0.f, 0.f },
			{ 0.f, -destPixelDims.height, 0.f },
			{ destPixelDims.width, -destPixelDims.height, 0.f },
		};

		// update index count
		nIndices_ += 6;

		// write vertex source coordinates
		pVertexUpload_[nVertices_ + 0].tc = { srcInTexcoords.left, srcInTexcoords.top };
		pVertexUpload_[nVertices_ + 1].tc = { srcInTexcoords.right, srcInTexcoords.top };
		pVertexUpload_[nVertices_ + 2].tc = { srcInTexcoords.left, srcInTexcoords.bottom };
		pVertexUpload_[nVertices_ + 3].tc = { srcInTexcoords.right, srcInTexcoords.bottom };

		// write vertex destination and atlas index
		for (int i = 0; i < 4; i++) {
			auto& vtx = pVertexUpload_[nVertices_ + i];
			vtx.position = posArray[i];
			vtx.rotation = rot;
			vtx.scale = { scale.x, scale.y };
			vtx.translation = { pos.x, pos.y };
			vtx.atlasIndex = atlasIndex32;
		}

		// increment vertex write index / count
		nVertices_ += 4;
	}

	AllocatorListPair SpriteBatcher::EndBatch()
	{
		cpr_assert(cmd_.pCommandAllocator);
		cpr_assert(cmd_.pCommandList);

		// fill index buffer if not already filled
		if (!indexBufferFilled_) {
			WriteIndexBufferFillCommands_(cmd_);
		}
		else if (pIndexUploadBuffer_ && signaledFenceValue_ >= indexBufferUploadFenceValue_) {
			// remove upload buffer when upload is finished
			pIndexUploadBuffer_.Reset();
		}
		// unmap upload vertex
		{
			const auto mapWrittenRange = D3D12_RANGE{ 0, nVertices_ * sizeof(Vertex_) };
			currentFrameResource_->pVertexBuffer->Unmap(0, &mapWrittenRange);
			pVertexUpload_ = nullptr;
		}

		// set pipeline state 
		cmd_.pCommandList->SetPipelineState(pPipelineState_.Get());
		cmd_.pCommandList->SetGraphicsRootSignature(pRootSignature_.Get());
		// configure IA 
		cmd_.pCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		cmd_.pCommandList->IASetVertexBuffers(0, 1, &currentFrameResource_->vertexBufferView);
		cmd_.pCommandList->IASetIndexBuffer(&indexBufferView_);
		// bind the heap containing the texture descriptor
		{
			ID3D12DescriptorHeap* heapArray[1] = { pSpriteCodex_->GetHeap() };
			cmd_.pCommandList->SetDescriptorHeaps(1, heapArray);
		}
		// bind the descriptor table containing the texture descriptor
		cmd_.pCommandList->SetGraphicsRootDescriptorTable(0, pSpriteCodex_->GetTableHandle());
		// bind the camera transform matrix
		cmd_.pCommandList->SetGraphicsRoot32BitConstants(1, sizeof(cameraTransform_) / 4, &cameraTransform_, 0);
		// draw vertices
		cmd_.pCommandList->DrawIndexedInstanced(nIndices_, 1, 0, 0, 0);

		// return frame resource to pool
		frameResourcePool_.PutResource(std::move(*currentFrameResource_), frameFenceValue_);
		currentFrameResource_.reset();

		// reliquish command list to be executed on a queue
		return std::move(cmd_);
	}



	// SpriteBatcher::FrameResource_
	// -----------------------------

	SpriteBatcher::FrameResource_ SpriteBatcher::GetFrameResource_(uint64_t frameFenceValue)
	{
		// get an existing buffer available from the pool
		if (auto fr = frameResourcePool_.GetResource(frameFenceValue)) {
			return std::move(*fr);
		}
		// create a new one if none available
		auto pDeviceInterface = pDevice_->GetD3D12DeviceInterface();
		FrameResource_ fr;
		// vertex buffer
		{
			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC resourceDesc = {};
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = sizeof(Vertex_) * maxVertices_;
			resourceDesc.Height = 1;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			pDeviceInterface->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr, IID_PPV_ARGS(&fr.pVertexBuffer)
			) >> hrVerify;
		}
		// vertex buffer view
		fr.vertexBufferView = {
			.BufferLocation = fr.pVertexBuffer->GetGPUVirtualAddress(),
			.SizeInBytes = (UINT)sizeof(Vertex_) * maxVertices_,
			.StrideInBytes = sizeof(Vertex_),
		};

		return fr;
	}

	void SpriteBatcher::WriteIndexBufferFillCommands_(AllocatorListPair& cmd)
	{
		// create array of index data
		std::vector<UINT> indexData(maxIndices_);
		{
			UINT baseVertexIndex_ = 0;
			for (size_t i = 0; i < maxIndices_; i += 6) {
				indexData[i + 0] = baseVertexIndex_ + 0;
				indexData[i + 1] = baseVertexIndex_ + 1;
				indexData[i + 2] = baseVertexIndex_ + 2;
				indexData[i + 3] = baseVertexIndex_ + 1;
				indexData[i + 4] = baseVertexIndex_ + 3;
				indexData[i + 5] = baseVertexIndex_ + 2;
				baseVertexIndex_ += 4;
			}
		}
		// create committed resource for cpu upload of index data
		{
			D3D12_HEAP_PROPERTIES heapProps = {};
			heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
			heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
			heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
			heapProps.CreationNodeMask = 1;
			heapProps.VisibleNodeMask = 1;

			D3D12_RESOURCE_DESC resourceDesc = {};
			resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
			resourceDesc.Alignment = 0;
			resourceDesc.Width = indexData.size() * sizeof(UINT);
			resourceDesc.Height = 1;
			resourceDesc.DepthOrArraySize = 1;
			resourceDesc.MipLevels = 1;
			resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
			resourceDesc.SampleDesc.Count = 1;
			resourceDesc.SampleDesc.Quality = 0;
			resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
			resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

			pDevice_->GetD3D12DeviceInterface()->CreateCommittedResource(
				&heapProps,
				D3D12_HEAP_FLAG_NONE,
				&resourceDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr, IID_PPV_ARGS(&pIndexUploadBuffer_)
			) >> hrVerify;
		}
		// copy array of index data to upload buffer  
		{
			UINT* mappedIndexData = nullptr;
			pIndexUploadBuffer_->Map(0, nullptr, reinterpret_cast<void**>(&mappedIndexData)) >> hrVerify;
			rn::copy(indexData, mappedIndexData);
			pIndexUploadBuffer_->Unmap(0, nullptr);
		}
		// copy upload buffer to index buffer  
		cmd.pCommandList->CopyResource(pIndexBuffer_.Get(), pIndexUploadBuffer_.Get());
		// transition index buffer to index buffer state 
		{
			D3D12_RESOURCE_BARRIER barrier = {};
			barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			barrier.Transition.pResource = pIndexBuffer_.Get();
			barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
			barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_INDEX_BUFFER;

			cmd.pCommandList->ResourceBarrier(1, &barrier);
		}

		// set index buffer filled flag
		indexBufferFilled_ = true;
		// set fence value for upload complete
		indexBufferUploadFenceValue_ = frameFenceValue_;
	}



	// sprite frame
	// ------------

	SpriteFrame::SpriteFrame(const SPA::RectF& frameInPixels, size_t atlasIndex, std::shared_ptr<SpriteCodex> pCodex)
		:
		atlasIndex_{ atlasIndex },
		pCodex_{ std::move(pCodex) }
	{
		atlasDimensions_ = pCodex_->GetAtlasDimensions(atlasIndex_);
		frameInTexcoords_ = {
			.left = frameInPixels.left / atlasDimensions_.width,
			.top = frameInPixels.top / atlasDimensions_.height,
			.right = frameInPixels.right / atlasDimensions_.width,
			.bottom = frameInPixels.bottom / atlasDimensions_.height,
		};
	}

	SpriteFrame::SpriteFrame(const SPA::DimensionsI& cellGridDimensions, const SPA::Vec2I& cellCoordinates, size_t atlasIndex, std::shared_ptr<SpriteCodex> pCodex)
		:
		atlasIndex_{ atlasIndex },
		pCodex_{ std::move(pCodex) }
	{
		atlasDimensions_ = pCodex_->GetAtlasDimensions(atlasIndex_);
		const auto cellWidth = atlasDimensions_.width / float(cellGridDimensions.width);
		const auto cellHeight = atlasDimensions_.height / float(cellGridDimensions.height);
		const auto frameInPixels = SPA::RectF{
			.left = cellWidth * cellCoordinates.x,
			.top = cellHeight * cellCoordinates.y,
			.right = cellWidth * (cellCoordinates.x + 1),
			.bottom = cellHeight * (cellCoordinates.y + 1),
		};
		frameInTexcoords_ = {
			.left = frameInPixels.left / atlasDimensions_.width,
			.top = frameInPixels.top / atlasDimensions_.height,
			.right = frameInPixels.right / atlasDimensions_.width,
			.bottom = frameInPixels.bottom / atlasDimensions_.height,
		};
	}

	void SpriteFrame::DrawToBatch(ISpriteBatcher& batch, const SPA::Vec2F& pos, float rotation, const SPA::Vec2F& scale) const
	{
		// deriving dest in pixel coordinates from texcoord source frame and source atlas dimensions
		const auto destPixelDims = frameInTexcoords_.GetDimensions() * atlasDimensions_;
		batch.Draw(atlasIndex_, frameInTexcoords_, destPixelDims, pos, rotation, scale);
	}



	// sprite codex
	// ------------

	SpriteCodex::SpriteCodex(std::shared_ptr<IDeviceChil> pDevice, UINT maxNumAtlases)
		:
		pDevice_{ std::move(pDevice) },
		maxNumAtlases_{ maxNumAtlases }
	{
		// descriptor heap for srvs
		{
			const D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc{
				.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV,
				.NumDescriptors = (UINT)maxNumAtlases,
				.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE,
			};
			pDevice_->GetD3D12DeviceInterface()->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&pSrvHeap_)) >> hrVerify;
		}
		// size of descriptors used for index calculation
		descriptorSize_ = pDevice_->GetD3D12DeviceInterface()->GetDescriptorHandleIncrementSize(
			D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	void SpriteCodex::AddSpriteAtlas(std::shared_ptr<ITexture> pTexture)
	{
		cpr_assert(curNumAtlases_ < maxNumAtlases_);

		// get handle to the destination descriptor
		auto descriptorHandle = pSrvHeap_->GetCPUDescriptorHandleForHeapStart();
		descriptorHandle.ptr += SIZE_T(descriptorSize_) * SIZE_T(curNumAtlases_);
		// write into descriptor
		pTexture->WriteDescriptor(pDevice_->GetD3D12DeviceInterface().Get(), descriptorHandle);
		// store in atlas array
		spriteAtlases_.push_back(std::make_unique<SpriteAtlas_>(descriptorHandle, std::move(pTexture)));
		// update number of atlases stored
		curNumAtlases_++;
	}

	ID3D12DescriptorHeap* SpriteCodex::GetHeap() const
	{
		return pSrvHeap_.Get();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE SpriteCodex::GetTableHandle() const
	{
		return pSrvHeap_->GetGPUDescriptorHandleForHeapStart();
	}

	SPA::DimensionsI SpriteCodex::GetAtlasDimensions(size_t atlasIndex) const
	{
		cpr_assert(atlasIndex < curNumAtlases_);

		return spriteAtlases_[atlasIndex]->pTexture_->GetDimensions();
	}
}