#include "CameraD11.h"

namespace CPR::GFX::D11
{
	using namespace DirectX;

	CameraD11::CameraD11(std::shared_ptr<IBufferManager> bufferManager, float minDepth,
		float maxDepth, float aspectRatio)
	{
		CreateProjectionMatrix(minDepth,
			maxDepth, aspectRatio);
		position = { 0.0f, 0.0f, -4.0f };
		forward = { 0.0f, 0.0f, 1.0f };
		up = { 0.0f, 1.0f, 0.0f };
		right = { 1.0f, 0.0f, 0.0f };


		vpBufferIdx = bufferManager->AddBuffer(nullptr,
			BufferInfo{
				.elementSize = sizeof(XMFLOAT4X4),
				.nrOfElements = 1,
				.rwPattern = PerFrameUsage::DYNAMIC,
				.bindingFlags = BufferBinding::CONSTANT_BUFFER,
			}
		);

		cameraPosBufferIdx = bufferManager->AddBuffer(nullptr,
			BufferInfo{
				.elementSize = sizeof(XMFLOAT3),
				.nrOfElements = 1,
				.rwPattern = PerFrameUsage::DYNAMIC,
				.bindingFlags = BufferBinding::CONSTANT_BUFFER,
			}
		);
	}

	CameraD11::CameraD11(std::shared_ptr<IBufferManager> bufferManager, float width, float height)
	{
		CreateProjectionMatrix(width, height);

		position = { width/2.f, height/2.f, 0.0f };
		forward = { 0.0f, 0.0f, 1.0f };
		up = { 0.0f, 1.0f, 0.0f };
		right = { 1.0f, 0.0f, 0.0f };


		vpBufferIdx = bufferManager->AddBuffer(nullptr,
			BufferInfo{
				.elementSize = sizeof(XMFLOAT4X4),
				.nrOfElements = 1,
				.rwPattern = PerFrameUsage::DYNAMIC,
				.bindingFlags = BufferBinding::CONSTANT_BUFFER,
			}
		);

		cameraPosBufferIdx = bufferManager->AddBuffer(nullptr,
			BufferInfo{
				.elementSize = sizeof(XMFLOAT3),
				.nrOfElements = 1,
				.rwPattern = PerFrameUsage::DYNAMIC,
				.bindingFlags = BufferBinding::CONSTANT_BUFFER,
			}
		);
	}

	void CameraD11::CreateProjectionMatrix(float width, float height)
	{
		XMMATRIX projection = DirectX::XMMatrixOrthographicLH(width, height, 0.1f, 1000.0f);
		//XMMATRIX projection = DirectX::XMMatrixOrthographicOffCenterLH(0.0f, width, height, 0.0f, 0.1f, 1000.0f);
		XMStoreFloat4x4(&projectionMatrix, projection);
	}

	void CameraD11::CreateProjectionMatrix(float minDepth,
		float maxDepth, float aspectRatio)
	{
		XMMATRIX projection = XMMatrixPerspectiveFovLH(XM_PIDIV2,
			aspectRatio, minDepth, maxDepth);
		XMStoreFloat4x4(&projectionMatrix, projection);
	}

	void CameraD11::MoveZ(float amount)
	{
		XMVECTOR currentPos = XMLoadFloat3(&position);
		XMVECTOR forwardVector = XMLoadFloat3(&forward);
		currentPos += forwardVector * amount;
		DirectX::XMStoreFloat3(&position, currentPos);
	}

	void CameraD11::MoveY(float amount)
	{
		XMVECTOR currentPos = XMLoadFloat3(&position);
		XMVECTOR upVector = XMLoadFloat3(&up);
		currentPos += upVector * amount;
		DirectX::XMStoreFloat3(&position, currentPos);
	}

	void CameraD11::MoveX(float amount)
	{
		XMVECTOR currentPos = XMLoadFloat3(&position);
		XMVECTOR rightVector = XMLoadFloat3(&right);
		currentPos += rightVector * amount;
		DirectX::XMStoreFloat3(&position, currentPos);
	}

	void CameraD11::RotateY(float radians)
	{
		XMVECTOR axis = XMLoadFloat3(&up);
		XMMATRIX rotationMatrix = XMMatrixRotationAxis(axis, radians);
		XMVECTOR forwardVector = XMLoadFloat3(&forward);
		XMVECTOR rightVector = XMLoadFloat3(&right);
		DirectX::XMStoreFloat3(&forward, XMVector3Transform(forwardVector, rotationMatrix));
		DirectX::XMStoreFloat3(&right, XMVector3Transform(rightVector, rotationMatrix));
	}

	XMFLOAT4X4 CameraD11::GetVP()
	{
		XMVECTOR eyePos = XMLoadFloat3(&position);
		XMVECTOR forwardVector = XMLoadFloat3(&forward);
		XMVECTOR upVector = XMLoadFloat3(&up);

		XMMATRIX view = XMMatrixLookAtLH(eyePos,
			XMVectorAdd(eyePos, forwardVector), upVector);
		XMMATRIX projection = XMLoadFloat4x4(&projectionMatrix);

		XMFLOAT4X4 toReturn;
		XMStoreFloat4x4(&toReturn, XMMatrixTranspose(view * projection));

		return toReturn;
	}
	ResourceIndex CameraD11::GetVPBufferIndex()
	{
		return vpBufferIdx;
	}
	DirectX::XMFLOAT3 CameraD11::GetPosition()
	{
		return position;
	}
	ResourceIndex CameraD11::GetPositionBufferIndex()
	{
		return cameraPosBufferIdx;
	}
}
