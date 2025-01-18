#pragma once

namespace oxygen
{
	struct Entity;
	struct Component : ManagedObject
	{
		OXYGENOBJECT(Component, ManagedObject);

		auto GetEntity() const -> std::shared_ptr<Entity>
		{
			return m_entity.lock();
		}

		auto IsEnabled() const -> oxyBool
		{
			return m_enabled;
		}
		auto SetEnabled(oxyBool enabled) -> void
		{
			m_enabled = enabled;
		}

	  protected:
		virtual auto Update(oxyF32 deltaTimeSeconds) -> void{};
		virtual auto Render() const -> void{};

	  private:
		std::weak_ptr<Entity> m_entity{};
		oxyBool m_enabled{true};

		friend struct Entity;
	};
}; // namespace oxygen