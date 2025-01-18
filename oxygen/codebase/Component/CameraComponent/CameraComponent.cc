#include "OxygenPCH.h"
#include "CameraComponent.h"
#include "Entity/Entity.h"

#include "Gfx/GfxRenderer.h"

namespace oxygen
{
	auto CameraComponent::Update(oxyF32 deltaTimeSeconds) -> void
	{
		const auto ent = GetEntity();
		// HACK: TODO:
		if (m_updateEntityYaw && ent->GetFlag(EntityFlags_IsLocalPlayer))
		{
			const auto entEuler =
				Math::QuatToEulerAngles(ent->GetWorldRotation());
			ent->SetWorldRotation(Math::EulerAnglesToQuat(
				{entEuler.x, entEuler.y, -m_cameraEuler.z}));
		}

		m_cameraForward = Math::EulerForward(m_cameraEuler);
		m_cameraRight = m_cameraForward.CrossProduct({0.f, 0.f, 1.f});
		m_cameraUp = m_cameraRight.CrossProduct(m_cameraForward);

		const auto worldPos = ent->GetWorldPosition() + m_cameraLocalOffset;
		m_viewMatrix =
			Math::LookAt(worldPos, worldPos + m_cameraForward, m_cameraUp);

		const auto aspect =
			static_cast<oxyF32>(GfxRenderer::GetInstance().GetWidth()) /
			GfxRenderer::GetInstance().GetHeight();

		m_projectionMatrix =
			Math::Perspective(m_verticalFovRad, aspect, m_nearClip, m_farClip);
		m_viewProjectionMatrix = m_viewMatrix * m_projectionMatrix;
	}
}; // namespace oxygen