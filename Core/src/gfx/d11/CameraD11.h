#pragma once
#include <DirectXMath.h>

#include "BufferManagerD11.h"

namespace CPR::GFX::D11
{
	class CameraD11
	{
	public:
		CameraD11(std::shared_ptr<IBufferManager> bufferManager, float width, float height);
		CameraD11(std::shared_ptr<IBufferManager> bufferManager, float minDepth, float maxDepth, float aspectRatio);
		~CameraD11() = default;

		void MoveZ(float amount);
		void MoveY(float amount);
		void MoveX(float amount);
		void RotateY(float radians);

		DirectX::XMFLOAT4X4 GetVP();
		ResourceIndex GetVPBufferIndex();
		DirectX::XMFLOAT3 GetPosition();
		ResourceIndex GetPositionBufferIndex();
		void ResetPosition();
	private:
		DirectX::XMFLOAT3 initialPosition;
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 forward;
		DirectX::XMFLOAT3 up;
		DirectX::XMFLOAT3 right;

		DirectX::XMFLOAT4X4 projectionMatrix;

		ResourceIndex vpBufferIdx = ResourceIndex(-1);
		ResourceIndex cameraPosBufferIdx = ResourceIndex(-1);

		void CreateProjectionMatrix(float minDepth, float maxDepth, float aspectRatio);
		void CreateProjectionMatrix(float width, float height);
	};

}