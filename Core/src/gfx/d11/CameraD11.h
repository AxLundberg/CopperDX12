#pragma once
#include <DirectXMath.h>

#include "BufferManagerD11.h"

namespace CPR::GFX::D11
{
	class CameraD11
	{
	public:
		CameraD11(BufferManagerD11& bufferManager, float minDepth, float maxDepth, float aspectRatio);
		~CameraD11() = default;

		void MoveZ(float amount);
		void MoveY(float amount);
		void MoveX(float amount);
		void RotateY(float radians);

		ResourceIndex GetVP(BufferManagerD11& bufferManager);
		ResourceIndex GetPosition(BufferManagerD11& bufferManager);

	private:
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 forward;
		DirectX::XMFLOAT3 up;
		DirectX::XMFLOAT3 right;

		DirectX::XMFLOAT4X4 projectionMatrix;

		ResourceIndex vpBufferIndex = ResourceIndex(-1);
		ResourceIndex cameraPosBufferIndex = ResourceIndex(-1);

		void CreateProjectionMatrix(float minDepth, float maxDepth, float aspectRatio);

	};

}