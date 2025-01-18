#pragma once

#include "Component/Component.h"

namespace oxygen
{
	struct CameraComponent final : Component
	{
		OXYGENOBJECT(CameraComponent, Component);

		auto GetCameraLocalOffset() const -> const oxyVec3&
		{
			return m_cameraLocalOffset;
		}
		auto GetEuler() const -> const oxyVec3&
		{
			return m_cameraEuler;
		}
		auto GetCameraForward() const -> const oxyVec3&
		{
			return m_cameraForward;
		}
		auto GetCameraUp() const -> const oxyVec3&
		{
			return m_cameraUp;
		}
		auto GetCameraRight() const -> const oxyVec3&
		{
			return m_cameraRight;
		}
		auto GetVerticalFov() const -> oxyF32
		{
			return m_verticalFovRad;
		}
		auto GetNearClip() const -> oxyF32
		{
			return m_nearClip;
		}
		auto GetFarClip() const -> oxyF32
		{
			return m_farClip;
		}
		auto GetViewMatrix() const -> const oxyMat4x4&
		{
			return m_viewMatrix;
		}
		auto GetProjectionMatrix() const -> const oxyMat4x4&
		{
			return m_projectionMatrix;
		}
		auto GetViewProjectionMatrix() const -> const oxyMat4x4&
		{
			return m_viewProjectionMatrix;
		}

		auto SetEuler(const oxyVec3& euler) -> void
		{
			m_cameraEuler = euler;
		}
		auto SetLocalOffset(const oxyVec3& localOffset) -> void
		{
			m_cameraLocalOffset = localOffset;
		}
		auto SetNearClip(oxyF32 nearClip) -> void
		{
			m_nearClip = nearClip;
		}
		auto SetFarClip(oxyF32 farClip) -> void
		{
			m_farClip = farClip;
		}

	  protected:
		auto Update(oxyF32 deltaTimeSeconds) -> void override;

	  private:
		oxyVec3 m_cameraLocalOffset{};
		oxyVec3 m_cameraEuler{};
		oxyVec3 m_cameraForward{1.f, 0.f, 0.f};
		oxyVec3 m_cameraUp{0.f, 0.f, 1.f};
		oxyVec3 m_cameraRight{0.f, 1.f, 0.f};
		oxyF32 m_verticalFovRad{90.0f * Math::k_degToRad};
		oxyF32 m_nearClip{4.f};
		oxyF32 m_farClip{4000.f};
		oxyMat4x4 m_viewMatrix{};
		oxyMat4x4 m_projectionMatrix{};
		oxyMat4x4 m_viewProjectionMatrix{};

		oxyBool m_updateEntityYaw{true};
	};
}; // namespace oxygen