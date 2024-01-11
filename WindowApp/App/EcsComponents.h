#pragma once
#include <DirectXMath.h>
#include <SimpleMath.h>

#include "typedefs.h"

namespace CPR::APP
{
	struct TransformComponent
	{
		TransformComponent(const DirectX::SimpleMath::Vector3& position = { 0.0f, 0.0f, 0.0f },
			const DirectX::SimpleMath::Vector3& rotation = { 0.0f, 0.0f, 0.0f },
			const DirectX::SimpleMath::Vector3& scale = { 1.0f, 1.0f, 1.0f }) noexcept;
		TransformComponent& SetPosition(const DirectX::SimpleMath::Vector3& position) noexcept;
		TransformComponent& SetRotation(const DirectX::SimpleMath::Vector3& rotation) noexcept;
		TransformComponent& SetRotation(const DirectX::SimpleMath::Matrix& rotationMatrix) noexcept;
		TransformComponent& SetScale(const DirectX::SimpleMath::Vector3& scale) noexcept;
		DirectX::SimpleMath::Vector3 GetPosition() const noexcept;
		DirectX::SimpleMath::Matrix GetRotation() const noexcept;
		DirectX::SimpleMath::Vector3 GetForward() const noexcept { return DirectX::SimpleMath::Vector3(worldMatrix._31, worldMatrix._32, worldMatrix._33); }
		DirectX::SimpleMath::Vector3 GetUp() const noexcept { return DirectX::SimpleMath::Vector3(worldMatrix._21, worldMatrix._22, worldMatrix._23); }
		DirectX::SimpleMath::Vector3 GetRight() const noexcept { return -worldMatrix.Right(); }
		DirectX::SimpleMath::Vector3 GetScale() const noexcept;

		TransformComponent& RotateW(const DirectX::SimpleMath::Vector3& rotation) noexcept;
		TransformComponent& RotateW(const DirectX::SimpleMath::Matrix& rotation) noexcept;
		TransformComponent& RotateL(const DirectX::SimpleMath::Vector3& rotation) noexcept;
		TransformComponent& RotateL(const DirectX::SimpleMath::Matrix& rotation) noexcept;

		TransformComponent& RotateForwardTo(const DirectX::SimpleMath::Vector3& target) noexcept;

		operator const DirectX::SimpleMath::Matrix& () const { return worldMatrix; }
		operator DirectX::SimpleMath::Matrix& () { return worldMatrix; }

		DirectX::SimpleMath::Matrix worldMatrix = DirectX::SimpleMath::Matrix::Identity;
	};
	struct CoordinateComponent
	{
		i32 x;
		i32 y;
	};
	struct SurfacePropertyComponent
	{
		ResourceIndex diffuse;
		ResourceIndex specular;
		ResourceIndex sampler;
	};
	struct TransformBufferComponent
	{
		ResourceIndex transformBuffer;
	};
	struct MeshComponent
	{
		ResourceIndex vertexBuffer;
		ResourceIndex indexBuffer;
	};
}