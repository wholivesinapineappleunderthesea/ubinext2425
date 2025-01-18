#pragma once

#include "Singleton/Singleton.h"

namespace oxygen
{
	struct World;
	struct GameManager : SingletonBase<GameManager>
	{
		GameManager();

		auto Render() -> void;
		auto Update(float deltaTimeSeconds) -> void;

		auto HostSummonEntity(EntitySpawnType type, const oxyVec3& pos,
						 const oxyQuat& rot) -> std::shared_ptr<struct Entity>;

		auto HostGame(std::string worldName) -> void;

	  private:
		friend struct NetSystem;
		auto HandlePacket(struct NetConnection& conn, oxyU16 type,
						  std::span<const oxyU8> data) -> void;

		auto HostHandlePacket(struct NetConnection& conn, oxyU16 type,
							  std::span<const oxyU8> data) -> void;
		auto ClientHandlePacket(struct NetConnection& conn, oxyU16 type,
								std::span<const oxyU8> data) -> void;

		auto ClientDisconnectedFromHost() -> void;

		auto HostNewPeerConnected(struct NetConnection& conn) -> void;
		auto HostPeerDisconnected(struct NetConnection& conn) -> void;

		auto SpawnEntityInWorld(
			EntitySpawnType type, const oxyVec3& pos, const oxyQuat& rot,
			std::vector<oxyObjectID>& ids) -> std::shared_ptr<struct Entity>;

		auto SendPeerEntityHistory(struct NetConnection& conn) -> void;

		auto HostSendEntityTransforms() -> void;
		auto ClientSendEntityTransforms() -> void;

		auto InterpolateEntityTransforms(float deltaTimeSeconds) -> void;


		struct PeerData
		{
			oxyBool m_loadedIn{};
			std::weak_ptr<struct Entity> m_localPlayer;
		};
		std::unordered_map<oxyU64, PeerData> m_peers;

		struct InterpolateEntityTransformData
		{
			std::weak_ptr<struct Entity> m_entity;
			oxyVec3 m_latestPosition;
			oxyQuat m_latestRotation;
			oxyF32 m_timeSinceReceived{};


		};
		std::vector<InterpolateEntityTransformData>
			m_interpolateEntityTransforms;

		std::string m_worldName;
		std::shared_ptr<World> m_world;

		std::vector<std::tuple<EntitySpawnType, std::vector<oxyObjectID>, std::weak_ptr<struct Entity>>>
			m_entitySpawnHistory;

		oxyF32 m_timeUntilNextGolfclubSpawn{3.0f};
	};
}; // namespace oxygen