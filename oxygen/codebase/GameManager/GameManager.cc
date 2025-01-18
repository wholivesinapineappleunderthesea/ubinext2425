#include "OxygenPCH.h"
#include "GameManager.h"

#include "World/World.h"
#include "World/WorldLoader.h"

#include "Entity/Entity.h"

#include "Component/Pawn/Pawn.h"
#include "Component/AnimatedMeshComponent/AnimatedMeshComponent.h"
#include "Component/StaticMeshComponent/StaticMeshComponent.h"
#include "Component/WeaponComponent/WeaponComponent.h"
#include "Component/HullComponent/HullComponent.h"
#include "Component/CameraComponent/CameraComponent.h"
#include "Component/ProjectileComponent/ProjectileComponent.h"
#include "Component/HealthComponent/HealthComponent.h"

#include "Net/NetSystem.h"
#include "UI/UIManager.h"
#include "Platform/Platform.h"

namespace oxygen
{
	GameManager::GameManager()
	{
#if 0
		auto hostgame = false;

		// iterate args
		const auto args = GetLaunchArguments();
		for (auto i = 0; i < args.size(); ++i)
		{
			const auto& arg = args[i];
			if (arg.compare("-host") == 0)
			{
				hostgame = true;
			}
			else if (arg.compare("-connect") == 0)
			{
				if (i + 1 < args.size())
				{
					NetSystem::GetInstance().ConnectToHost(args[i + 1]);
				}
			}
		}
		if (args.size() < 2)
		{
			hostgame = true;
		}
		if (hostgame)
		{
			m_worldName = "oregon2";
			m_world = LoadWorld("oregon2");
			NetSystem::GetInstance().StartHost();

			//
			auto ids = std::vector<oxyObjectID>{};
			auto e = HostSummonEntity(EntitySpawnType_Player, {0.f, 0.f, 0.f},
									  {0.f, 0.f, 0.f});
			m_world->SetLocalPlayer(e);
			ids.clear();
			HostSummonEntity(EntitySpawnType_GrenadeLauncherWeapon,
							 {100.f, 0.f, 0.f}, {0.f, 0.f, 0.f});
		}
#endif
	}
	auto GameManager::Render() -> void
	{
		if (m_world)
		{
			m_world->SubmitBSPFacesToRenderQueue();
		}
	}
	auto GameManager::Update(float deltaTimeSeconds) -> void
	{
		if (m_world)
		{
			m_world->Update(deltaTimeSeconds);

			// repl ents
			if (NetSystem::GetInstance().IsHost())
			{
				HostSendEntityTransforms();


				// test if there is a golf club launcher valid in the entity history
				oxyBool found = false;
				for (const auto& enthist : m_entitySpawnHistory)
				{
					const auto& [type, ids, ent] = enthist;
					if (type == EntitySpawnType_GolfclubLauncher)
						if (ent.lock())
							found = true;
				}
				if (!found)
				{
					m_timeUntilNextGolfclubSpawn -= deltaTimeSeconds;
					if (m_timeUntilNextGolfclubSpawn < 0.f)
					{
						m_timeUntilNextGolfclubSpawn = 3.f;
						const auto pos = m_world->RandomPlayerSpawn();
						HostSummonEntity(EntitySpawnType_GolfclubLauncher, pos,
										 {0.f, 0.f, 0.f, 1.f});
					}
				}
			}
			else
			{
				ClientSendEntityTransforms();
			}

			InterpolateEntityTransforms(deltaTimeSeconds);
		}

	}
	auto
	GameManager::HostSummonEntity(EntitySpawnType type, const oxyVec3& pos,
								  const oxyQuat& rot) -> std::shared_ptr<Entity>
	{
		std::vector<oxyObjectID> idsout;
		auto ent = SpawnEntityInWorld(type, pos, rot, idsout);
		std::vector<oxyU8> buffer;
		buffer.resize(sizeof(EntitySpawnType) + sizeof(oxyVec3) +
					  sizeof(oxyQuat) + sizeof(oxyU16) +
					  (sizeof(oxyObjectID) * idsout.size()));
		*reinterpret_cast<EntitySpawnType*>(buffer.data()) = type;
		*reinterpret_cast<oxyVec3*>(buffer.data() + sizeof(EntitySpawnType)) =
			pos;
		*reinterpret_cast<oxyQuat*>(buffer.data() + sizeof(EntitySpawnType) +
									sizeof(oxyVec3)) = rot;
		*reinterpret_cast<oxyU16*>(buffer.data() + sizeof(EntitySpawnType) +
								   sizeof(oxyVec3) + sizeof(oxyQuat)) =
			static_cast<oxyU16>(idsout.size());
		auto idptr = reinterpret_cast<oxyObjectID*>(
			buffer.data() + sizeof(EntitySpawnType) + sizeof(oxyVec3) +
			sizeof(oxyQuat) + sizeof(oxyU16));
		for (auto i = 0; i < idsout.size(); ++i)
		{
			idptr[i] = idsout[i];
		}
		NetSystem::GetInstance().HostSendToAll(NetProtoMsgType_SrvEntitySpawn,
											   buffer);
		return ent;
	}
	auto GameManager::HostGame(std::string worldName) -> void
	{
		if (m_world)
			return; // huh
		m_world = LoadWorld(worldName);
		if (m_world)
		{
			m_worldName = std::move(worldName);
			NetSystem::GetInstance().StartHost();
			auto ids = std::vector<oxyObjectID>{};
			const auto pos = m_world->RandomPlayerSpawn();
			auto e =
				HostSummonEntity(EntitySpawnType_Player, pos,
									  {0.f, 0.f, 0.f, 1.f});
			m_world->SetLocalPlayer(e);

			HostSummonEntity(EntitySpawntype_GolfballLauncher, pos,
							 {0, 0, 0, 1});
		}
	}
	auto GameManager::HandlePacket(NetConnection& conn, oxyU16 type,
								   std::span<const oxyU8> data) -> void
	{
		const auto isHost = NetSystem::GetInstance().IsHost();
		if (isHost)
			HostHandlePacket(conn, type, data);
		else
			ClientHandlePacket(conn, type, data);
	}
	auto GameManager::HostHandlePacket(NetConnection& conn, oxyU16 type,
									   std::span<const oxyU8> data) -> void
	{
		auto& peerdata = m_peers[conn.GetUniqueID()];

		if (type == NetProtoMsgType_CliLocalPlayerEntityMove)
		{
			if (data.size() != sizeof(oxyVec3) + sizeof(oxyQuat))
				return;
			const auto pos = *reinterpret_cast<const oxyVec3*>(data.data());
			const auto rot = *reinterpret_cast<const oxyQuat*>(data.data() +
															   sizeof(oxyVec3));
			if (m_world)
			{
				const auto& peer = m_peers[conn.GetUniqueID()];
				const auto ent = peer.m_localPlayer.lock();
				if (ent)
				{
					// find/insert to m_interpolateEntityTransforms
					auto found = false;
					for (auto& interp : m_interpolateEntityTransforms)
					{
						if (interp.m_entity.lock() == ent)
						{
							found = true;
							interp.m_latestPosition = pos;
							interp.m_latestRotation = rot;
							interp.m_timeSinceReceived = 0.f;
							break;
						}
					}
					if (!found)
					{
						m_interpolateEntityTransforms.push_back(
							{ent, pos, rot, 0.f});
					}
				}
			}
		}
		else if (type == NetProtoMsgType_CliPawnDropWeapon)
		{
			if (data.size() != sizeof(oxyObjectID))
				return;
			const auto pawnid =
				*reinterpret_cast<const oxyObjectID*>(data.data());
			if (m_world)
			{
				auto pawn =
					ObjectManager::GetInstance().GetManagedRef<Pawn>(pawnid);
				if (pawn)
				{
					if (peerdata.m_localPlayer.lock() == pawn->GetEntity())
					{
						pawn->DropWeaponNetWrap();
					}
				}
			}
		}
		else if (type == NetProtoMsgType_CliLocalPlayerFireWeapon)
		{
			if (data.size() != sizeof(oxyVec3) + sizeof(oxyVec3) + sizeof(oxyU8))
				return;
			const auto euler = *reinterpret_cast<const oxyVec3*>(data.data());
			const auto pos = *reinterpret_cast<const oxyVec3*>(data.data() +
															   sizeof(oxyVec3));
			const auto rh = *reinterpret_cast<const oxyU8*>(
				data.data() + sizeof(oxyVec3) + sizeof(oxyVec3));
			if (m_world)
			{
				const auto& peer = m_peers[conn.GetUniqueID()];
				const auto ent = peer.m_localPlayer.lock();
				if (ent)
				{
					auto pawn = ent->GetComponent<Pawn>();
					if (pawn)
					{
						if (!rh)
						{
							const auto wpn = pawn->GetEquippedWeapon();
							if (wpn)
							{
								wpn->FireInDirectionFromPos(euler, pos);
							}
						}
						else
						{
							const auto wpn =
								pawn->GetEquippedRightHandedWeapon();
							if (wpn)
							{
								wpn->FireInDirectionFromPos(euler, pos);
							}
						}
						
					}
				}
			}
		}
	}
	auto GameManager::ClientHandlePacket(NetConnection& conn, oxyU16 type,
										 std::span<const oxyU8> data) -> void
	{
		if (type == NetProtoMsgType_SrvChangeLevel)
		{
			const auto worldName = std::string_view(
				reinterpret_cast<const char*>(data.data()), data.size());
			m_worldName = worldName;
			m_world = LoadWorld(worldName);
		}
		else if (type == NetProtoMsgType_SrvEntitySpawn)
		{
			// handle ent spawn
			// oxyU16 : EntitySpawnType
			// oxyVec3: position
			// oxyQuat: rotation
			if (data.size() < sizeof(EntitySpawnType) + sizeof(oxyVec3) +
								  sizeof(oxyQuat) + sizeof(oxyU16))
				return;
			const auto countids = *reinterpret_cast<const oxyU16*>(
				data.data() + sizeof(EntitySpawnType) + sizeof(oxyVec3) +
				sizeof(oxyQuat));
			if (data.size() != sizeof(EntitySpawnType) + sizeof(oxyVec3) +
								   sizeof(oxyQuat) + sizeof(oxyU16) +
								   (sizeof(oxyObjectID) * countids))
				return;

			const auto entType =
				*reinterpret_cast<const EntitySpawnType*>(data.data());
			const auto pos = *reinterpret_cast<const oxyVec3*>(
				data.data() + sizeof(EntitySpawnType));
			const auto rot = *reinterpret_cast<const oxyQuat*>(
				data.data() + sizeof(EntitySpawnType) + sizeof(oxyVec3));

			std::vector<oxyObjectID> ids;
			const auto idptr = reinterpret_cast<const oxyObjectID*>(
				data.data() + sizeof(EntitySpawnType) + sizeof(oxyVec3) +
				sizeof(oxyQuat) + sizeof(oxyU16));
			for (auto i = 0; i < countids; ++i)
			{
				ids.push_back(idptr[i]);
			}

			SpawnEntityInWorld(entType, pos, rot, ids);
		}
		else if (type == NetProtoMsgType_SrvEntityDestroy)
		{
			if (data.size() != sizeof(oxyObjectID))
				return;
			const auto entid =
				*reinterpret_cast<const oxyObjectID*>(data.data());
			if (m_world)
			{
				auto ref =
					ObjectManager::GetInstance().GetManagedRef<Entity>(entid);
				if (ref)
				{
					ref->Destroy();
				}
			}
		}
		else if (type == NetProtoMsgType_SrvSetLocalPlayer)
		{
			if (data.size() != sizeof(oxyObjectID))
				return;
			const auto entid =
				*reinterpret_cast<const oxyObjectID*>(data.data());
			if (m_world)
			{
				auto ref =
					ObjectManager::GetInstance().GetManagedRef<Entity>(entid);
				if (ref)
				{
					m_world->SetLocalPlayer(std::move(ref));
				}
			}
		}
		else if (type == NetProtoMsgType_SrvEntityTransformRepl)
		{
			// oxyObjectID, oxyVec3, oxyQuat, oxyVec3
			if (data.size() <
				sizeof(oxyObjectID) + sizeof(oxyVec3) + sizeof(oxyQuat))
				return;
			const auto entid =
				*reinterpret_cast<const oxyObjectID*>(data.data());
			const auto pos = *reinterpret_cast<const oxyVec3*>(
				data.data() + sizeof(oxyObjectID));
			const auto rot = *reinterpret_cast<const oxyQuat*>(
				data.data() + sizeof(oxyObjectID) + sizeof(oxyVec3));
			if (m_world)
			{
				auto ref =
					ObjectManager::GetInstance().GetManagedRef<Entity>(entid);
				if (ref)
				{
					// find/insert to m_interpolateEntityTransforms
					auto found = false;
					for (auto& interp : m_interpolateEntityTransforms)
					{
						if (interp.m_entity.lock() == ref)
						{
							found = true;
							interp.m_latestPosition = pos;
							interp.m_latestRotation = rot;
							interp.m_timeSinceReceived = 0.f;
							break;
						}
					}
					if (!found)
					{
						m_interpolateEntityTransforms.push_back(
							{ref, pos, rot, 0.f});
					}
				}
			}
		}
		else if (type == NetProtoMsgType_SrvPawnPickupWeapon)
		{
			if (data.size() != sizeof(oxyObjectID) + sizeof(oxyObjectID))
				return;
			const auto pawnid =
				*reinterpret_cast<const oxyObjectID*>(data.data());
			const auto weaponid = *reinterpret_cast<const oxyObjectID*>(
				data.data() + sizeof(oxyObjectID));
			if (m_world)
			{
				auto pawn =
					ObjectManager::GetInstance().GetManagedRef<Pawn>(pawnid);
				auto weapon =
					ObjectManager::GetInstance().GetManagedRef<WeaponComponent>(
						weaponid);
				if (pawn && weapon)
				{
					pawn->PickupWeapon(std::move(weapon));
				}
			}
		}
		else if (type == NetProtoMsgType_SrvPawnDropWeapon)
		{
			if (data.size() != sizeof(oxyObjectID))
				return;
			const auto pawnid =
				*reinterpret_cast<const oxyObjectID*>(data.data());
			if (m_world)
			{
				auto pawn =
					ObjectManager::GetInstance().GetManagedRef<Pawn>(pawnid);
				if (pawn)
				{
					pawn->DropWeaponImpl();
				}
			}
		}
		else if (type == NetProtoMsgType_SrvHealhComponentChange)
		{
			if (data.size() != sizeof(oxyObjectID) + sizeof(oxyS32) +
								   sizeof(oxyS32) + sizeof(oxyU8))
				return;
			const auto healthcompid =
				*reinterpret_cast<const oxyObjectID*>(data.data());
			const auto health = *reinterpret_cast<const oxyS32*>(
				data.data() + sizeof(oxyObjectID));
			const auto maxhealth = *reinterpret_cast<const oxyS32*>(
				data.data() + sizeof(oxyObjectID) + sizeof(oxyS32));
			const auto state = *reinterpret_cast<const oxyU8*>(
				data.data() + sizeof(oxyObjectID) + sizeof(oxyS32) +
				sizeof(oxyS32));
			if (m_world)
			{
				auto healthcomp =
					ObjectManager::GetInstance().GetManagedRef<HealthComponent>(
						healthcompid);
				if (healthcomp)
				{
					healthcomp->ClientReceiveHealthStateChange(
						health, maxhealth, static_cast<HealthState>(state));
				}
			}
		}
	}
	auto GameManager::ClientDisconnectedFromHost() -> void
	{
		m_world.reset();
		m_peers.clear();
		m_entitySpawnHistory.clear();
		m_interpolateEntityTransforms.clear();
		UIManager::GetInstance().DisplayPopup("Disconnected from host");
	}
	auto GameManager::HostNewPeerConnected(NetConnection& conn) -> void
	{
		m_peers[conn.GetUniqueID()] = {};

		// send NetProtoMsgType_SrvWelcome
		{
			std::vector<oxyU8> buffer;
			buffer.resize(sizeof(oxyU64));
			auto* id = reinterpret_cast<oxyU64*>(buffer.data());
			*id = conn.GetUniqueID();
			conn.WriteData(NetProtoMsgType_SrvWelcome, buffer);
		}

		// send NetProtoMsgType_SrvChangeLevel
		{
			std::vector<oxyU8> buffer;
			buffer.resize(m_worldName.size());
			std::memcpy(buffer.data(), m_worldName.data(), m_worldName.size());
			conn.WriteData(NetProtoMsgType_SrvChangeLevel, buffer);
		}

		SendPeerEntityHistory(conn);

		// send (to all) NetProtoMsgType_SrvEntitySpawn
		{
			const auto pos = m_world->RandomPlayerSpawn();
			auto ent =
				HostSummonEntity(EntitySpawnType_Player, pos,
										{0.f, 0.f, 0.f, 1.f});

			HostSummonEntity(EntitySpawntype_GolfballLauncher, pos,
							 {0, 0, 0, 1});

			m_peers[conn.GetUniqueID()].m_localPlayer = ent;

			// tell the client to possess it (NetProtoMsgType_SrvSetLocalPlayer)
			{
				const auto entid = ent->GetObjectID();
				std::vector<oxyU8> buffer;
				buffer.resize(sizeof(oxyObjectID));
				auto* id = reinterpret_cast<oxyObjectID*>(buffer.data());
				*id = entid;
				conn.WriteData(NetProtoMsgType_SrvSetLocalPlayer, buffer);
			}
		}
	}
	auto GameManager::HostPeerDisconnected(NetConnection& conn) -> void
	{
		m_peers.erase(conn.GetUniqueID());
	}
	auto GameManager::SendPeerEntityHistory(NetConnection& conn) -> void
	{
		for (auto it = m_entitySpawnHistory.begin();
			 it != m_entitySpawnHistory.end();)
		{
			const auto& [type, ids, ent] = *it;
			if (ent.expired())
			{
				it = m_entitySpawnHistory.erase(it);
				continue;
			}
			auto ref = ent.lock();
			if (ref)
			{
				std::vector<oxyU8> buffer;
				buffer.resize(sizeof(EntitySpawnType) + sizeof(oxyVec3) +
							  sizeof(oxyQuat) + sizeof(oxyU16) +
							  (sizeof(oxyObjectID) * ids.size()));
				*reinterpret_cast<EntitySpawnType*>(buffer.data()) = type;
				*reinterpret_cast<oxyVec3*>(buffer.data() +
											sizeof(EntitySpawnType)) =
					ref->GetWorldPosition();
				*reinterpret_cast<oxyQuat*>(
					buffer.data() + sizeof(EntitySpawnType) + sizeof(oxyVec3)) =
					ref->GetWorldRotation();
				*reinterpret_cast<oxyU16*>(
					buffer.data() + sizeof(EntitySpawnType) + sizeof(oxyVec3) +
					sizeof(oxyQuat)) = static_cast<oxyU16>(ids.size());
				auto* idptr = reinterpret_cast<oxyObjectID*>(
					buffer.data() + sizeof(EntitySpawnType) + sizeof(oxyVec3) +
					sizeof(oxyQuat) + sizeof(oxyU16));
				for (auto i = 0; i < ids.size(); ++i)
				{
					idptr[i] = ids[i];
				}
				conn.WriteData(NetProtoMsgType_SrvEntitySpawn, buffer);
			}
			++it;
		}
	}
	auto GameManager::HostSendEntityTransforms() -> void
	{
		for (auto it = m_entitySpawnHistory.begin();
			 it != m_entitySpawnHistory.end();)
		{
			const auto& [type, ids, ent] = *it;
			const auto ref = ent.lock();
			if (!ref)
			{
				it = m_entitySpawnHistory.erase(it);
				continue;
			}

			if (!ref->GetFlag(EntityFlags_EnableTransformReplication))
			{
				++it;
				continue;
			}

			// these might be getting interpolated, if so use the latest
			// position/rotation
			auto worldPos = ref->GetLocalPosition();
			auto worldRot = ref->GetLocalRotation();
			oxyBool needsrepl = true;
			for (const auto& interp : m_interpolateEntityTransforms)
			{
				if (interp.m_entity.lock() == ref)
				{
					if ((worldPos - interp.m_latestPosition)
								.MagnitudeSquared() < 0.01f &&
						worldRot.DotProduct(interp.m_latestRotation) > 0.99f)
					{
						needsrepl = false;
						break;
					}
					worldPos = interp.m_latestPosition;
					worldRot = interp.m_latestRotation;
					break;
				}
			}
			if (!needsrepl)
			{
				++it;
				continue;
			}

			std::vector<oxyU8> buffer;
			buffer.resize(sizeof(oxyObjectID) + sizeof(oxyVec3) +
						  sizeof(oxyQuat));
			*reinterpret_cast<oxyObjectID*>(buffer.data()) = ref->GetObjectID();
			*reinterpret_cast<oxyVec3*>(buffer.data() + sizeof(oxyObjectID)) =
				worldPos;
			*reinterpret_cast<oxyQuat*>(buffer.data() + sizeof(oxyObjectID) +
										sizeof(oxyVec3)) = worldRot;

			oxyBool foundpeer = false;
			oxyU64 peerid = 0;
			for (const auto& peer : m_peers)
			{
				const auto& peerdata = peer.second;
				if (peerdata.m_localPlayer.lock() == ref)
				{
					foundpeer = true;
					peerid = peer.first;
					break;
				}
			}
			if (foundpeer)
			{
				NetSystem::GetInstance().HostSendToAllExcept(
					peerid, NetProtoMsgType_SrvEntityTransformRepl, buffer);
			}
			else
			{
				NetSystem::GetInstance().HostSendToAll(
					NetProtoMsgType_SrvEntityTransformRepl, buffer);
			}

			++it;
		}
	}
	auto GameManager::ClientSendEntityTransforms() -> void
	{
		if (m_world)
		{
			const auto lp = m_world->GetLocalPlayer();
			const auto ref = lp.lock();
			if (ref)
			{
				const auto pos = ref->GetLocalPosition();
				const auto rot = ref->GetLocalRotation();
				std::vector<oxyU8> buffer;
				buffer.resize(sizeof(oxyVec3) + sizeof(oxyQuat));
				*reinterpret_cast<oxyVec3*>(buffer.data()) = pos;
				*reinterpret_cast<oxyQuat*>(buffer.data() + sizeof(oxyVec3)) =
					rot;
				NetSystem::GetInstance().CliSendToHost(
					NetProtoMsgType_CliLocalPlayerEntityMove, buffer);
			}
		}
	}
	void GameManager::InterpolateEntityTransforms(float deltaTimeSeconds)
	{
		for (auto it = m_interpolateEntityTransforms.begin();
			 it != m_interpolateEntityTransforms.end();)
		{
			auto& interp = *it;
			const auto ent = interp.m_entity.lock();
			if (!ent)
			{
				it = m_interpolateEntityTransforms.erase(it);
				continue;
			}
			if (!ent->GetFlag(EntityFlags_EnableTransformReplication))
			{
				++it;
				continue;
			}
			if (ent->GetFlag(EntityFlags_EnableTransformInterpolation))
			{
				const auto pos = ent->GetLocalPosition();
				const auto rot = ent->GetLocalRotation();
				const float interpolationWindow = 0.1f;

				auto alpha = deltaTimeSeconds / interpolationWindow;
				alpha = std::clamp(alpha, 0.0f, 1.0f);

				const auto newPos =
					pos + (interp.m_latestPosition - pos) * alpha;
				const auto newRot =
					Math::Slerp(interp.m_latestRotation, rot, alpha);

				ent->SetLocalPosition(newPos);
				ent->SetLocalRotation(newRot);
			}
			else
			{
				ent->SetLocalPosition(interp.m_latestPosition);
				ent->SetLocalRotation(interp.m_latestRotation);
			}
			interp.m_timeSinceReceived += deltaTimeSeconds;

			++it;
		}
	}
	auto GameManager::SpawnEntityInWorld(
		EntitySpawnType type, const oxyVec3& pos, const oxyQuat& rot,
		std::vector<oxyObjectID>& ids) -> std::shared_ptr<struct Entity>
	{
		const auto shouldOutputIds = ids.size() == 0;
		const auto GetIDAtIndex = [&](auto idx) -> oxyObjectID {
			if (shouldOutputIds)
				return NetSystem::GetInstance().GetNewNetObjID();
			return ids[idx];
		};
		const auto OutputID = [&](const auto& managedObj) {
			if (shouldOutputIds)
			{
				ids.push_back(managedObj.GetObjectID());
			}
		};
		if (m_world)
		{
			auto ent = m_world->SpawnEntity(GetIDAtIndex(0));
			OutputID(*ent);
			ent->SetWorldPosition(pos);
			ent->SetLocalRotation(rot);
			ent->SetLocalScale({1.f, 1.f, 1.f});

			if (type == EntitySpawnType_Player)
			{
				ent->SetFlag(EntityFlags_Dynamic, true);
				ent->SetFlag(EntityFlags_HasHull, true);
				ent->SetFlag(EntityFlags_Renderable, true);
				ent->SetFlag(EntityFlags_EnableTransformReplication, true);
				ent->SetFlag(EntityFlags_EnableTransformInterpolation, true);
				ent->SetRenderOcclusionMin({-64.f, -64.f, -64.f});
				ent->SetRenderOcclusionMax({64.f, 64.f, 64.f});

				auto amc =
					ent->AddComponent<AnimatedMeshComponent>(GetIDAtIndex(1));
				OutputID(*amc);
				amc->LoadByName("Player3RD");
				amc->SetLocalOffset({0.f, 0.f, -32.f});
				amc->SetLocalRotation(Math::EulerAnglesToQuat(
					{0.f, 0.f, 90.0f * Math::k_degToRad}));

				auto hull = ent->AddComponent<HullComponent>(GetIDAtIndex(2));
				OutputID(*hull);
				hull->SetHull(CollisionHull_Player);
				hull->SetGravityPerSecond(700.f);
				hull->SetDrag(15.5f);
				hull->SetResponse(CollisionResponseType_Slide);

				auto pawn = ent->AddComponent<Pawn>();
				pawn->m_thirdPersonMesh = amc;

				auto camera =
					ent->AddComponent<CameraComponent>(GetIDAtIndex(3));
				OutputID(*camera);
				camera->SetLocalOffset(oxyVec3{0.f, 0.f, 32.f});

				auto health =
					ent->AddComponent<HealthComponent>(GetIDAtIndex(4));
				OutputID(*health);
				health->SetHealth(100);
				health->SetMaxHealth(100);
			}
			else if (type == EntitySpawnType_Golfclub)
			{
				ent->SetFlag(EntityFlags_Dynamic, true);
				ent->SetFlag(EntityFlags_Renderable, true);
				ent->SetFlag(EntityFlags_HasHull, true);
				ent->SetFlag(EntityFlags_EnableTransformReplication, true);
				ent->SetFlag(EntityFlags_EnableTransformInterpolation, true);
				ent->SetRenderOcclusionMin({-128, -128, -128});
				ent->SetRenderOcclusionMax({128, 128, 128});
				auto smc =
					ent->AddComponent<StaticMeshComponent>(GetIDAtIndex(1));
				OutputID(*smc);
				smc->LoadByName("Club");
				auto hull = ent->AddComponent<HullComponent>(GetIDAtIndex(2));
				OutputID(*hull);
				hull->SetHull(CollisionHull_Player);
				hull->SetGravityPerSecond(400.f);
				hull->SetDrag(0.07f);
				hull->SetResponse(CollisionResponseType_Bounce);
				hull->SetSolidToOtherHulls(false);
				auto proj =
					ent->AddComponent<ProjectileComponent>(GetIDAtIndex(3));
				OutputID(*proj);
				proj->SetBouncesLeft(0);
				proj->SetDamage(128.f);
				proj->SetDamageRadius(64.f);
				proj->SetHull(hull);
			}
			else if (type == EntitySpawnType_GolfclubLauncher)
			{
				ent->SetFlag(EntityFlags_Dynamic, true);
				ent->SetFlag(EntityFlags_Renderable, true);
				ent->SetFlag(EntityFlags_HasHull, true);
				ent->SetFlag(EntityFlags_EnableTransformReplication, true);
				ent->SetFlag(EntityFlags_EnableTransformInterpolation, true);
				ent->SetRenderOcclusionMin({-128, -128, -128});
				ent->SetRenderOcclusionMax({128, 128, 128});
				auto smc =
					ent->AddComponent<StaticMeshComponent>(GetIDAtIndex(1));
				OutputID(*smc);
				smc->LoadByName("Club");
				auto hull = ent->AddComponent<HullComponent>(GetIDAtIndex(2));
				OutputID(*hull);
				hull->SetHull(CollisionHull_Player);
				hull->SetGravityPerSecond(700.f);
				hull->SetDrag(15.5f);
				hull->SetResponse(CollisionResponseType_Slide);
				hull->SetSolidToOtherHulls(false);

				auto weapon =
					ent->AddComponent<WeaponComponent>(GetIDAtIndex(3));
				OutputID(*weapon);
				weapon->SetInfiniteReserve(true);
				weapon->SetInfiniteClip(true);
				weapon->SetClipAmmo(1);
				weapon->SetReserveAmmo(1);
				weapon->SetBulletsPerShot(1);
				weapon->SetRPM(140);
				weapon->SetTimeToReload(2.f);
				weapon->SetSpreadRadians({0.1f, 0.1f});
				weapon->SetFireType(WeaponFireType_GolfClub);
				weapon->SetDestroyOnFire(true);
				weapon->SetCanDrop(false);
			}
			else if (type == EntitySpawnType_Golfball)
			{
				ent->SetFlag(EntityFlags_Dynamic, true);
				ent->SetFlag(EntityFlags_Renderable, true);
				ent->SetFlag(EntityFlags_HasHull, true);
				ent->SetFlag(EntityFlags_EnableTransformReplication, true);
				ent->SetFlag(EntityFlags_EnableTransformInterpolation, true);
				ent->SetRenderOcclusionMin({-32.f, -32.f, -32.f});
				ent->SetRenderOcclusionMax({32.f, 32.f, 32.f});
				auto smc =
					ent->AddComponent<StaticMeshComponent>(GetIDAtIndex(1));
				OutputID(*smc);
				smc->LoadByName("Golfball");
				auto hull = ent->AddComponent<HullComponent>(GetIDAtIndex(2));
				OutputID(*hull);
				hull->SetHull(CollisionHull_Grenade);
				hull->SetGravityPerSecond(400.f);
				hull->SetDrag(0.07f);
				hull->SetResponse(CollisionResponseType_Bounce);
				hull->SetSolidToOtherHulls(false);
				auto proj =
					ent->AddComponent<ProjectileComponent>(GetIDAtIndex(3));
				OutputID(*proj);
				proj->SetBouncesLeft(0);
				proj->SetDamage(12.f);
				proj->SetDamageRadius(64.f);
				proj->SetHull(hull);
			}
			else if (type == EntitySpawntype_GolfballLauncher)
			{
				ent->SetFlag(EntityFlags_Dynamic, true);
				ent->SetFlag(EntityFlags_HasHull, true);
				ent->SetFlag(EntityFlags_EnableTransformReplication, false);
				ent->SetFlag(EntityFlags_EnableTransformInterpolation, false);
				ent->SetRenderOcclusionMin({-128, -128, -128});
				ent->SetRenderOcclusionMax({128, 128, 128});
				auto hull = ent->AddComponent<HullComponent>(GetIDAtIndex(1));
				OutputID(*hull);
				hull->SetHull(CollisionHull_Player);
				hull->SetGravityPerSecond(700.f);
				hull->SetDrag(15.5f);
				hull->SetResponse(CollisionResponseType_Slide);
				hull->SetSolidToOtherHulls(false);

				auto weapon =
					ent->AddComponent<WeaponComponent>(GetIDAtIndex(2));
				OutputID(*weapon);
				weapon->SetInfiniteReserve(true);
				weapon->SetInfiniteClip(true);
				weapon->SetClipAmmo(1);
				weapon->SetReserveAmmo(1);
				weapon->SetBulletsPerShot(1);
				weapon->SetRPM(140);
				weapon->SetTimeToReload(2.f);
				weapon->SetSpreadRadians({0.1f, 0.1f});
				weapon->SetFireType(WeaponFireType_GolfBall);
				weapon->SetRightHanded(true);
				weapon->SetCanDrop(false);
			}

			m_entitySpawnHistory.push_back({type, ids, ent});

			return ent;
		}
		return {};
	}
} // namespace oxygen
