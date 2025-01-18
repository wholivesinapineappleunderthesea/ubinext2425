#pragma once

#include "Singleton/Singleton.h"

// this is lazy
#include "Platform/Platform.h"

namespace oxygen
{
	struct NetConnection
	{
		auto WriteData(oxyU16 type, std::span<const oxyU8> data) -> void;

		auto GetUniqueID() const -> oxyU64
		{
			return m_uniqueID;
		}

	  private:
		oxyU64 m_uniqueID{};
		std::unique_ptr<NetworkAbstraction::NetworkSocket> m_socket{};
		std::thread m_receiveThread{};
		std::thread m_sendThread{};
		struct MessageHeader
		{
			oxyU16 m_type{};
			oxyU16 m_size{};
		};

		static inline constexpr auto k_queueSize = 1024;
		oxyBool m_connected{};

		// TODO:
		// SPSCQueue is BUSTED
		// I'm going to use a mutex for now
		// watch this later: https://www.youtube.com/watch?v=K3P_Lmq6pw0

		//// producer: main game update thread
		//// consumer: send thread
		SPSCQueue<std::vector<oxyU8>, k_queueSize>
			m_sendQueue{};
		//// producer: receive thread
		//// consumer: main game update thread
		SPSCQueue<std::vector<oxyU8>, k_queueSize>
			m_receiveQueue{};
		// std::queue<std::vector<oxyU8>> m_sendQueue{};
		// std::queue<std::vector<oxyU8>> m_receiveQueue{};
		// std::mutex m_sendMutex{};
		// std::mutex m_receiveMutex{};

		friend struct NetSystem;
	};

	struct NetSystem : SingletonBase<NetSystem>
	{
		NetSystem();
		~NetSystem();

		auto Update(oxyF32 deltaTimeSeconds) -> void;

		auto StartHost() -> void;
		auto ConnectToHost(const std::string& ip) -> oxyBool;

		auto KillConnections() -> void;

		auto HostSendToAll(oxyU16 type, const std::span<oxyU8>& data) -> void;
		auto HostSendToAllExcept(oxyU64 excludeClientID, oxyU16 type,
								 const std::span<oxyU8>& data) -> void;

		auto CliSendToHost(oxyU16 type, const std::span<oxyU8>& data) -> void;

		auto CliDiscoverHosts() -> void;
		auto CliGetDiscoveredHosts() const -> std::span<const std::string>
		{
			return m_discoveredHosts;
		}
		auto CliIsDiscoveringHosts() const -> oxyBool
		{
			return m_broadcastDiscoveryRunning;
		}

		auto GetNewNetObjID() -> oxyObjectID
		{
			return m_nextNetObjID++;
		}

		auto IsHost() const -> oxyBool
		{
			return m_isHost;
		}

		auto IsClient() const -> oxyBool
		{
			return m_isClient;
		}

		static inline constexpr auto k_enginePort = 28672;
		static inline constexpr auto k_engineBroadcastPort = 28678;
		static inline constexpr auto k_timeBetweenPing = 4.0f;
		static inline constexpr auto k_netSessionDefaultMinObjid =
			oxyObjectID{0xC0000001};

	  private:
		auto PingAll() -> void;

		auto ServerAcceptPeers() -> void;

		auto GenNewUniqueID() -> oxyU64;

		auto
		ReceiveAllFromNetworkSocket(NetworkAbstraction::NetworkSocket& sock)
			-> std::optional<std::vector<oxyU8>>;
		auto SendAllToNetworkSocket(NetworkAbstraction::NetworkSocket& sock,
									const std::span<oxyU8>& data) -> oxyBool;

		auto ProcessReceivedMessage(NetConnection& conn,
									std::span<const oxyU8> data) -> void;

		auto NetConnectionReceiveThread(NetConnection& connection) -> void;
		auto NetConnectionSendThread(NetConnection& connection) -> void;

		auto ClientBroadcastSendThread() -> void;
		auto ServerBroadcastListenThread() -> void;

		// (if client)
		// client->server
		NetConnection m_clientHostSocket{};
		std::unique_ptr<NetworkAbstraction::NetworkSocket>
			m_clientBroadcastSendSocket{};
		std::thread m_clientBroadcastSendThread{};
		oxyBool m_broadcastDiscoveryRunning{};
		// broadcast thread out:
		std::vector<std::string> m_broadcastDiscoveredHosts{};
		std::mutex m_broadcastDiscoveredHostsMutex{};
		// for update/main thread:
		std::vector<std::string> m_discoveredHosts{};

		// (if host)
		// server listen socket
		std::unique_ptr<NetworkAbstraction::NetworkSocket> m_serverSocket{};
		std::unique_ptr<NetworkAbstraction::NetworkSocket>
			m_serverBroadcastListenSocket{};
		std::thread m_serverBroadcastListenThread{};
		std::vector<std::unique_ptr<NetConnection>> m_clients{};

		oxyBool m_isHost{};
		oxyBool m_isClient{};
		oxyBool m_requestShutdown{};

		oxyF32 m_timeSinceLastPing{};
		oxyObjectID m_nextNetObjID{k_netSessionDefaultMinObjid};
	};
}; // namespace oxygen