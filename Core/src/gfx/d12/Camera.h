#pragma once
#include <DirectXMath.h>

#include "Camera.h"
#include "BufferManager.h"

namespace CPR::GFX::D12
{
	class Camera
	{
	private:
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 forward;
		DirectX::XMFLOAT3 up;
		DirectX::XMFLOAT3 right;

		DirectX::XMFLOAT4X4 projectionMatrix;

		ResourceIndex vpBufferIndex = ResourceIndex(-1);
		ResourceIndex cameraPosBufferIndex = ResourceIndex(-1);

		void CreateProjectionMatrix(float minDepth, float maxDepth, float aspectRatio);

	public:
		Camera(BufferManager& bufferManager, float minDepth,
			float maxDepth, float aspectRatio);
		~Camera() = default;

		void MoveZ(float amount);
		void MoveY(float amount);
		void MoveX(float amount);
		void RotateY(float radians);

		ResourceIndex GetVP(BufferManager& bufferManager);
		ResourceIndex GetPosition(BufferManager& bufferManager);
	};
}