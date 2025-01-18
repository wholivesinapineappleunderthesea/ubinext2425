#pragma once
#pragma once

namespace oxygen
{
	auto GetExecutableDirectory() -> std::string_view;

	auto GetLaunchArguments() -> std::span<const std::string>;

	auto LogMessage(const char* str) -> void;

	auto ReadFileContents(std::string_view absolutePath) -> std::vector<oxyU8>;

	struct FileMap : NonCopyable
	{
		auto GetMap() const -> void*
		{
			return m_data;
		}
		auto GetSize() const -> oxySize
		{
			return m_size;
		}
		auto ValidateRange(const void* ptr, oxySize sz) const -> bool
		{
			return reinterpret_cast<const oxyU8*>(ptr) >=
					   reinterpret_cast<const oxyU8*>(m_data) &&
				   (reinterpret_cast<const oxyU8*>(ptr) + sz) <=
					   (reinterpret_cast<const oxyU8*>(m_data) + m_size) &&
				   sz && m_data;
		}

	  protected:
		FileMap() = default;
		~FileMap() = default;
		void* m_data{};
		oxySize m_size{};
	};
	struct InternalFileMapDeleter
	{
		auto operator()(FileMap* ptr) const -> void;
	};
	using UniqueFileMap = std::unique_ptr<FileMap, InternalFileMapDeleter>;
	auto CreateFileMap(std::string_view path, oxyBool write = false,
					   oxySize requestSize = 0) -> UniqueFileMap;

	namespace GraphicsAbstraction
	{
		auto GetWindowSize(oxyS32& width, oxyS32& height) -> void;

		struct Texture
		{
			oxyU32 m_width;
			oxyU32 m_height;
			void* m_internalPlatformHandle;
		};
		auto
		LoadTexture(const char* absolutePath) -> std::shared_ptr<const Texture>;

		struct TexturedQuad
		{
			// NDC, -1 to +1
			oxyVec2 m_vertices[4];
			// OpenGL style uvs
			oxyVec2 m_textureCoords[4];
			// RGB
			oxyVec3 m_colour;
			// Sample texture
			const Texture* m_texture;
		};
		auto DrawTexturedQuad(const TexturedQuad& quad) -> void;
	}; // namespace GraphicsAbstraction

	namespace AudioAbstraction
	{

	};

	namespace InputAbstraction
	{
		auto HideAndLockCursor(oxyBool lock) -> void;
		auto GetMousePosition(oxyF32& x, oxyF32& y) -> void;
		auto GetMouseStates(std::bitset<MouseButton_Count>& buttons) -> void;
		auto GetKeyStates(std::bitset<KeyboardButton_Count>& keys) -> void;
		auto GetControllerConnected(int index) -> oxyBool;
		auto GetControllerStates(
			int index, std::bitset<ControllerButton_Count>& buttons) -> void;
		auto GetControllerAxisStates(
			int index, std::span<oxyF32, ControllerAxis_Count> axes) -> void;

	}; // namespace InputAbstraction

	namespace NetworkAbstraction
	{
		struct NetworkSocket
		{
			~NetworkSocket();

			auto Send(const void* data, oxySize size) -> oxySSize;
			auto Receive(void* data, oxySize size) -> oxySSize;

			auto Accept() -> std::unique_ptr<NetworkSocket>;

			auto BroadcastMessage(const void* data, oxySize size) -> std::vector<std::string>;
			auto RespondToBroadcasts(const void* data, oxySize size) -> void;

		  private:
			OXYSOCKETDESCRIPTORTYPE
			m_descriptor{}; // might need to be changed, unix uses signed
			oxyU32 m_address{};
			oxyU16 m_port{};

			friend auto ConnectToHost(const char* host, oxyU16 port)
				-> std::unique_ptr<NetworkSocket>;
			friend auto
			HostServer(oxyU16 port) -> std::unique_ptr<NetworkSocket>;
			friend auto CreateBroadcastSendSocket(oxyU16 port)
				-> std::unique_ptr<NetworkSocket>;
			friend auto CreateBroadcastListenSocket(oxyU16 port)
				-> std::unique_ptr<NetworkSocket>;
		};
		auto ConnectToHost(const char* host,
						   oxyU16 port) -> std::unique_ptr<NetworkSocket>;
		auto HostServer(oxyU16 port) -> std::unique_ptr<NetworkSocket>;
		auto
		CreateBroadcastSendSocket(oxyU16 port) -> std::unique_ptr<NetworkSocket>;
		auto CreateBroadcastListenSocket(oxyU16 port)
			-> std::unique_ptr<NetworkSocket>;

	}; // namespace NetworkAbstraction

}; // namespace oxygen