#include "OxygenPCH.h"
#include "NetSystem.h"

#include "GameManager/GameManager.h"

namespace oxygen
{
	NetSystem::NetSystem()
	{
	}
	NetSystem::~NetSystem()
	{
		KillConnections();
	}
	auto NetSystem::Update(oxyF32 deltaTimeSeconds) -> void
	{

		if (m_clientHostSocket.m_connected)
		{
			std::vector<oxyU8> buffer;
			while (m_clientHostSocket.m_receiveQueue.TryPop(buffer))
			{
				if (buffer.size())
				{
					ProcessReceivedMessage(m_clientHostSocket, buffer);
				}
				buffer.clear();
			}
			// std::lock_guard lock{m_clientHostSocket.m_receiveMutex};
			// while (!m_clientHostSocket.m_receiveQueue.empty())
			//{
			//	auto buffer =
			//		std::move(m_clientHostSocket.m_receiveQueue.front());
			//	m_clientHostSocket.m_receiveQueue.pop();
			//	if (buffer.size())
			//	{
			//		ProcessReceivedMessage(m_clientHostSocket, buffer);
			//	}
			// }
		}
		else if (m_isClient)
		{
			// disconnected!!
			m_isClient = false;
			GameManager::GetInstance().ClientDisconnectedFromHost();
		}

		{
			if (m_timeSinceLastPing > k_timeBetweenPing)
			{
				PingAll();
				m_timeSinceLastPing = 0.f;
			}
			m_timeSinceLastPing += deltaTimeSeconds;

			// accept new clients
			ServerAcceptPeers();

			// pop all messages
			for (auto& client : m_clients)
			{
				std::vector<oxyU8> buffer;
				while (client->m_receiveQueue.TryPop(buffer))
				{
					if (buffer.size())
					{
						ProcessReceivedMessage(*client, buffer);
					}
					buffer.clear();
				}
				//{
				//	std::lock_guard lock{client->m_receiveMutex};
				//	while (!client->m_receiveQueue.empty())
				//	{
				//		auto buffer = std::move(client->m_receiveQueue.front());
				//		client->m_receiveQueue.pop();
				//		if (buffer.size())
				//		{
				//			ProcessReceivedMessage(*client, buffer);
				//		}
				//	}
				//}
			}

			for (auto it = m_clients.begin(); it != m_clients.end();)
			{
				if (!(*it)->m_connected)
				{
					// tell game manager
					GameManager::GetInstance().HostPeerDisconnected(**it);

					// join threads
					if ((*it)->m_receiveThread.joinable())
						(*it)->m_receiveThread.join();
					if ((*it)->m_sendThread.joinable())
						(*it)->m_sendThread.join();

					it = m_clients.erase(it);
				}
				else
				{
					++it;
				}
			}
		}
		if (m_broadcastDiscoveryRunning && !m_clientBroadcastSendSocket)
		{
			m_broadcastDiscoveryRunning = false;
			std::lock_guard lock{m_broadcastDiscoveredHostsMutex};
			m_discoveredHosts = std::move(m_broadcastDiscoveredHosts);
		}
	}
	auto NetSystem::StartHost() -> void
	{
		KillConnections();

		m_requestShutdown = false;
		m_isHost = true;
		m_serverSocket = NetworkAbstraction::HostServer(k_enginePort);

		m_serverBroadcastListenSocket =
			NetworkAbstraction::CreateBroadcastListenSocket(
				k_engineBroadcastPort);
		m_serverBroadcastListenThread =
			std::thread{&NetSystem::ServerBroadcastListenThread, this};
	}
	auto NetSystem::ConnectToHost(const std::string& ip) -> oxyBool
	{
		KillConnections();

		m_requestShutdown = false;
		m_isClient = true;
		m_clientHostSocket.m_socket =
			NetworkAbstraction::ConnectToHost(ip.c_str(), k_enginePort);
		if (!m_clientHostSocket.m_socket)
			return false;
		m_clientHostSocket.m_connected = true;
		m_clientHostSocket.m_receiveThread =
			std::thread{&NetSystem::NetConnectionReceiveThread, this,
						std::ref(m_clientHostSocket)};
		m_clientHostSocket.m_sendThread =
			std::thread{&NetSystem::NetConnectionSendThread, this,
						std::ref(m_clientHostSocket)};
		return true;
	}
	auto NetSystem::KillConnections() -> void
	{
		m_isHost = false;
		m_requestShutdown = true;
		for (auto& client : m_clients)
		{
			client->m_connected = false;
			if (client->m_receiveThread.joinable())
				client->m_receiveThread.join();
			if (client->m_sendThread.joinable())
				client->m_sendThread.join();
		}
		for (auto& client : m_clients)
			client->m_socket.reset();
		m_serverSocket.reset();
		if (m_serverBroadcastListenThread.joinable())
			m_serverBroadcastListenThread.join();
		m_isClient = false;
		m_clientHostSocket.m_connected = false;
		if (m_clientHostSocket.m_receiveThread.joinable())
			m_clientHostSocket.m_receiveThread.join();
		if (m_clientHostSocket.m_sendThread.joinable())
			m_clientHostSocket.m_sendThread.join();
		m_clientHostSocket.m_socket.reset();
		if (m_clientBroadcastSendThread.joinable())
			m_clientBroadcastSendThread.join();

		m_nextNetObjID = k_netSessionDefaultMinObjid;
	}
	auto NetSystem::HostSendToAll(oxyU16 type,
								  const std::span<oxyU8>& data) -> void
	{
		for (const auto& client : m_clients)
		{
			if (client->m_connected)
			{
				client->WriteData(type, data);
			}
		}
	}
	auto NetSystem::HostSendToAllExcept(oxyU64 excludeClientID, oxyU16 type,
										const std::span<oxyU8>& data) -> void
	{
		for (const auto& client : m_clients)
		{
			if (client->m_connected && client->m_uniqueID != excludeClientID)
			{
				client->WriteData(type, data);
			}
		}
	}
	auto NetSystem::CliSendToHost(oxyU16 type,
								  const std::span<oxyU8>& data) -> void
	{
		if (m_clientHostSocket.m_connected)
		{
			m_clientHostSocket.WriteData(type, data);
		}
	}
	auto NetSystem::CliDiscoverHosts() -> void
	{
		if (!m_clientBroadcastSendSocket && !m_broadcastDiscoveryRunning)
		{
			if (m_clientBroadcastSendThread.joinable())
				m_clientBroadcastSendThread.join();

			m_clientBroadcastSendSocket =
				NetworkAbstraction::CreateBroadcastSendSocket(
					k_engineBroadcastPort);
			if (!m_clientBroadcastSendSocket)
				return;
			m_broadcastDiscoveryRunning = true;
			m_clientBroadcastSendThread =
				std::thread{&NetSystem::ClientBroadcastSendThread, this};
		}
	}
	auto NetSystem::PingAll() -> void
	{
		if (m_clientHostSocket.m_connected)
		{
			m_clientHostSocket.WriteData(NetProtoMsgType_AnyPing, {});
		}
		for (const auto& client : m_clients)
		{
			if (client->m_connected)
			{
				client->WriteData(NetProtoMsgType_AnyPing, {});
			}
		}
	}
	auto NetSystem::ServerAcceptPeers() -> void
	{
		if (!m_requestShutdown && m_isHost && m_serverSocket)
		{
			auto clientSocket = m_serverSocket->Accept();
			if (clientSocket)
			{
				auto client = std::make_unique<NetConnection>();
				client->m_socket = std::move(clientSocket);
				client->m_connected = true;
				client->m_receiveThread =
					std::thread{&NetSystem::NetConnectionReceiveThread, this,
								std::ref(*client)};
				client->m_sendThread =
					std::thread{&NetSystem::NetConnectionSendThread, this,
								std::ref(*client)};
				client->m_uniqueID = GenNewUniqueID();
				auto& clientref = *client;
				m_clients.push_back(std::move(client));
				GameManager::GetInstance().HostNewPeerConnected(clientref);
			}
		}
	}
	auto NetSystem::GenNewUniqueID() -> oxyU64
	{
		oxyU64 id{};
		while (true)
		{
			id = RandomU64(0, std::numeric_limits<oxyU64>::max());
			for (const auto& client : m_clients)
			{
				if (client->m_uniqueID == id)
					continue;
			}
			if (m_clientHostSocket.m_uniqueID == id)
				continue;
			break;
		}
		return id;
	}
	auto NetSystem::ReceiveAllFromNetworkSocket(
		NetworkAbstraction::NetworkSocket& sock)
		-> std::optional<std::vector<oxyU8>>
	{
		constexpr auto k_bufferStep = 1024;
		std::optional<std::vector<oxyU8>> rv;
		rv.emplace(k_bufferStep);
		auto received =
			sock.Receive(rv.value().data(), rv.value().size());
		if (received < 0)
			return {}; // empty = disconnected/error
		oxySize totalReceived = received;
		while (totalReceived == rv.value().size() && received > 0)
		{
			rv.value().resize(rv.value().size() + k_bufferStep);
			received =
				sock.Receive(rv.value().data() + totalReceived, k_bufferStep);
			if (received < 0)
				return {}; // empty = disconnected/error
			totalReceived += received;
		}
		rv.value().resize(totalReceived);
		return rv;
	}
	auto
	NetSystem::SendAllToNetworkSocket(NetworkAbstraction::NetworkSocket& sock,
									  const std::span<oxyU8>& data) -> oxyBool
	{
		const auto sent = sock.Send(data.data(), data.size());
		if (sent < 0)
			return false;
		auto totalsent = sent;
		while (totalsent < static_cast<oxySSize>(data.size()))
		{
			const auto remaining = data.size() - totalsent;
			const auto sent = sock.Send(data.data() + totalsent, remaining);
			if (sent < 0)
				return false;
			totalsent += sent;
		}
		return true;
	}
	auto NetSystem::ProcessReceivedMessage(NetConnection& conn,
										   std::span<const oxyU8> data) -> void
	{
		if (data.size() < sizeof(NetConnection::MessageHeader))
			return;
		const auto hdr =
			reinterpret_cast<const NetConnection::MessageHeader*>(data.data());
		if (hdr->m_size + sizeof(NetConnection::MessageHeader) > data.size())
		{
			LogMessage("WARN: NetSystem::ProcessReceivedMessage "
					   "INCORRECT SIZE IN PACKET\n");
			return;
		}

		if (hdr->m_type == NetProtoMsgType_AnyPing)
		{
			// ping
			return;
		}

		if (hdr->m_type == NetProtoMsgType_SrvWelcome)
		{
			if (data.size() !=
				sizeof(oxyU64) + sizeof(NetConnection::MessageHeader))
				return;
			const auto id = *reinterpret_cast<const oxyU64*>(
				data.data() + sizeof(NetConnection::MessageHeader));
			conn.m_uniqueID = id;
			return;
		}

		GameManager::GetInstance().HandlePacket(
			conn, hdr->m_type,
			data.subspan(sizeof(NetConnection::MessageHeader), hdr->m_size));

		ProcessReceivedMessage(
			conn,
			data.subspan(sizeof(NetConnection::MessageHeader) + hdr->m_size));
	}
	auto
	NetSystem::NetConnectionReceiveThread(NetConnection& connection) -> void
	{
		while (connection.m_connected)
		{
			auto buffer = ReceiveAllFromNetworkSocket(*connection.m_socket);
			if (!buffer.has_value())
			{
				connection.m_connected = false;
				return;
			}
			// std::lock_guard lock{connection.m_receiveMutex};
			// connection.m_receiveQueue.push(std::move(buffer.value
			while (
				!connection.m_receiveQueue.TryPush(std::move(buffer.value())) &&
				connection.m_connected)
			{
			}
		}
	}
	auto NetSystem::NetConnectionSendThread(NetConnection& connection) -> void
	{
		while (connection.m_connected)
		{
			std::vector<oxyU8> buffer;
			if (connection.m_sendQueue.TryPop(buffer))
			{
				if (buffer.size())
				{
					if (!SendAllToNetworkSocket(*connection.m_socket, buffer))
					{
						connection.m_connected = false;
						return;
					}
				}
				buffer.clear();
			}
			// std::lock_guard lock{connection.m_sendMutex};
			// while (!connection.m_sendQueue.empty())
			//{
			//	auto buffer = std::move(connection.m_sendQueue.front());
			//	connection.m_sendQueue.pop();
			//	if (buffer.size())
			//	{
			//		if (!SendAllToNetworkSocket(*connection.m_socket, buffer))
			//		{
			//			connection.m_connected = false;
			//			return;
			//		}
			//	}
			// }
		}
	}
	auto NetSystem::ClientBroadcastSendThread() -> void
	{
		auto hosts = std::vector<std::string>{};
		if (!m_requestShutdown && !m_isHost && m_clientBroadcastSendSocket)
		{
			hosts = m_clientBroadcastSendSocket->BroadcastMessage(
				"OxygenEngine", sizeof("OxygenEngine"));
			for (const auto& host : hosts)
			{
				LogMessage("Found host: ");
				LogMessage(host.c_str());
				LogMessage("\n");
			}
		}
		m_clientBroadcastSendSocket.reset();
		std::lock_guard lock{m_broadcastDiscoveredHostsMutex};
		m_broadcastDiscoveredHosts = std::move(hosts);
	}
	auto NetSystem::ServerBroadcastListenThread() -> void
	{
		while (!m_requestShutdown && m_isHost && m_serverBroadcastListenSocket)
		{

			m_serverBroadcastListenSocket->RespondToBroadcasts(
				"OxygenEngine", sizeof("OxygenEngine"));
		}
		m_serverBroadcastListenSocket.reset();
	}
	auto NetConnection::WriteData(oxyU16 type,
								  std::span<const oxyU8> data) -> void
	{
		OXYCHECK(data.size() < (std::numeric_limits<oxyU16>::max)());
		MessageHeader hdr;
		hdr.m_type = type;
		hdr.m_size = static_cast<oxyU16>(data.size());
		auto buffer = std::vector<oxyU8>(sizeof(MessageHeader) + data.size());
		std::memcpy(buffer.data(), &hdr, sizeof(MessageHeader));
		std::memcpy(buffer.data() + sizeof(MessageHeader), data.data(),
					data.size());
		// std::lock_guard lock{m_sendMutex};
		// m_sendQueue.push(std::move(buffer));
		while (!m_sendQueue.TryPush(std::move(buffer)))
		{
		}
	}
}; // namespace oxygen