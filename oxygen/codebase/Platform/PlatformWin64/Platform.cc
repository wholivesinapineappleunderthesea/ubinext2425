#include "OxygenPCH.h"
#ifndef OXYGEN_PLATFORM_WIN64
#error "Win64 Platform.cc included in build for incorrect platform"
#endif

#include "Platform/Platform.h"

#include "Singleton/EngineSingletons.h"

// Ubisoft API
#include "App/app.h"
#include "PrivateMembers.h"



extern int WINDOW_WIDTH;
extern int WINDOW_HEIGHT;
extern HWND MAIN_WINDOW_HANDLE;

namespace oxygen
{
	namespace
	{
		oxySize g_executableDirectoryLen{};
		oxyChar g_executableDirectory[MAX_PATH + 1]{};
		std::vector<std::string> g_launchArguments{};
		WSADATA g_wsaData{};
		oxyBool g_wsaInitialized{};
		oxyU64 g_renderCount{};
		oxyU64 g_updateCount{};
	}; // namespace
	auto GetExecutableDirectory() -> std::string_view
	{
		return {g_executableDirectory, g_executableDirectoryLen};
	}
	auto GetLaunchArguments() -> std::span<const std::string>
	{
		return g_launchArguments;
	}

	auto LogMessage(const char* str) -> void
	{
		OutputDebugStringA(str);
	}

	auto ReadFileContents(std::string_view absolutePath) -> std::vector<oxyU8>
	{
		std::vector<oxyU8> fileContents;
		const auto file =
			CreateFileA(absolutePath.data(), GENERIC_READ, 0, nullptr,
						OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE)
			return fileContents;

		LARGE_INTEGER fileSize;
		if (!GetFileSizeEx(file, &fileSize))
		{
			CloseHandle(file);
			return fileContents;
		}

		fileContents.resize(static_cast<size_t>(fileSize.QuadPart));
		DWORD bytesRead;
		if (!ReadFile(file, fileContents.data(),
					  static_cast<DWORD>(fileSize.QuadPart), &bytesRead,
					  nullptr))
		{
			CloseHandle(file);
			return {};
		}
		CloseHandle(file);
		fileContents.resize(bytesRead);
		return fileContents;
	}

	struct InternalFileMapWinX64 : FileMap
	{
	  private:
		void* m_fileHandle{};
		void* m_mapHandle{};
		friend struct InternalFileMapDeleter;
		friend auto CreateFileMap(std::string_view path, oxyBool write,
								  oxySize requestSize) -> UniqueFileMap;
	};
	auto InternalFileMapDeleter::operator()(FileMap* ptr) const -> void
	{
		const auto p = static_cast<InternalFileMapWinX64*>(ptr);
		OXYCHECK(p->m_fileHandle != INVALID_HANDLE_VALUE);
		OXYCHECK(p->m_mapHandle != nullptr);
		OXYVERIFY(UnmapViewOfFile(p->GetMap()));
		OXYVERIFY(CloseHandle(p->m_mapHandle));
		OXYVERIFY(CloseHandle(p->m_fileHandle));
		delete p;
	}
	auto CreateFileMap(std::string_view path, oxyBool write,
					   oxySize requestSize) -> UniqueFileMap
	{
		/* If an application specifies a size for the file mapping object that
		 * is larger than the size of the actual named file on disk and if the
		 * page protection allows write access (that is, the flProtect parameter
		 * specifies PAGE_READWRITE or PAGE_EXECUTE_READWRITE), then the file on
		 * disk is increased to match the specified size of the file mapping
		 * object. */
		if (!write && requestSize)
			return {};
		if (path.size() > MAX_PATH)
			return {};
		WCHAR wpath[MAX_PATH + 1]{};
		if (!MultiByteToWideChar(CP_UTF8, 0, path.data(),
								 static_cast<int>(path.size()), wpath,
								 MAX_PATH))
			return {};
		const auto fileHandle = CreateFileW(
			wpath, write ? GENERIC_READ | GENERIC_WRITE : GENERIC_READ,
			FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
			nullptr);
		if (fileHandle == INVALID_HANDLE_VALUE)
			return {};
		LARGE_INTEGER size{};
		if (!GetFileSizeEx(fileHandle, &size))
		{
			CloseHandle(fileHandle);
			return {};
		}
		/* An attempt to map a file with a length of 0 (zero) fails with an
		 * error code of ERROR_FILE_INVALID. Applications should test for files
		 * with a length of 0 (zero) and reject those files. */
		if (!size.QuadPart)
		{
			CloseHandle(fileHandle);
			return {};
		}
		DWORD lowSize{};
		DWORD highSize{};
		if (requestSize)
		{
			lowSize = static_cast<DWORD>(requestSize);
			highSize = static_cast<DWORD>(requestSize >> 32);
		}
		const auto mapHandle = CreateFileMappingW(
			fileHandle, nullptr, write ? PAGE_READWRITE : PAGE_READONLY,
			highSize, lowSize, nullptr);
		if (!mapHandle)
		{
			CloseHandle(fileHandle);
			return {};
		}
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			CloseHandle(mapHandle);
			CloseHandle(fileHandle);
			return {};
		}
		const auto map = MapViewOfFile(
			mapHandle, write ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
		if (!map)
		{
			CloseHandle(mapHandle);
			CloseHandle(fileHandle);
			return {};
		}
		auto p = new InternalFileMapWinX64{};
		p->m_fileHandle = fileHandle;
		p->m_mapHandle = mapHandle;
		p->m_data = map;
		p->m_size = size.QuadPart;
		return UniqueFileMap{p};
	}
	auto Win64PlatformInit() -> void
	{
		OXYVERIFY(GetModuleFileNameA(nullptr, g_executableDirectory, MAX_PATH));
		g_executableDirectoryLen = strlen(g_executableDirectory);
		OXYCHECK(g_executableDirectoryLen);
		// Remove the executable name
		while (g_executableDirectoryLen &&
			   g_executableDirectory[g_executableDirectoryLen - 1] != '\\')
			--g_executableDirectoryLen;
		g_executableDirectory[g_executableDirectoryLen] = '\0';

		g_launchArguments.reserve(__argc);
		for (int i = 0; i < __argc; ++i)
		{
#ifdef _UNICODE
			if (!__wargv[i])
				continue;
			const auto len = WideCharToMultiByte(CP_UTF8, 0, __wargv[i], -1,
												 nullptr, 0, nullptr, nullptr);
			if (!len)
				continue;
			auto str = std::string(len, '\0');
			WideCharToMultiByte(CP_UTF8, 0, __wargv[i], -1, str.data(),
								static_cast<int>(str.size()), nullptr, nullptr);
			// remove the null terminator
			if (str.back() == '\0')
				str.pop_back();
			g_launchArguments.push_back(std::move(str));
#else
			if (__argv[i])
				g_launchArguments.emplace_back(__argv[i]);
#endif
		}

		if (WSAStartup(MAKEWORD(2, 2), &g_wsaData) != 0)
		{
			MessageBoxW(nullptr, L"WSAStartup failed", L"Error", MB_OK);
			std::abort();
		}
		g_wsaInitialized = true;
		std::atexit([]() {
			if (g_wsaInitialized)
				WSACleanup();
			g_wsaInitialized = false;
		});

		EngineSingletons::Construct();
		std::atexit(EngineSingletons::Destruct);
	}
	auto Win64PlatformRender() -> void
	{
		if (!g_updateCount)
			return;
		oxyS32 w, h;
		GraphicsAbstraction::GetWindowSize(w, h);
		GfxRenderer::GetInstance().BeginFrame(w, h);
		GfxRenderer::GetInstance().EndFrame();
		++g_renderCount;
	}
	auto Win64PlatformUpdate(float deltaTimeSeconds) -> void
	{
		NetSystem::GetInstance().Update(deltaTimeSeconds);
		InputManager::GetInstance().Update();
		UIManager::GetInstance().Update();
		GameManager::GetInstance().Update(deltaTimeSeconds);
		++g_updateCount;
	}
	auto Win64PlatformShutdown() -> void
	{
		EngineSingletons::Destruct();
		if (g_wsaInitialized)
			WSACleanup();
		g_wsaInitialized = false;
	}

	namespace GraphicsAbstraction
	{
		auto GetWindowSize(oxyS32& width, oxyS32& height) -> void
		{
			width = static_cast<oxyS32>(WINDOW_WIDTH);
			height = static_cast<oxyS32>(WINDOW_HEIGHT);
		}

		auto
		LoadTexture(const char* absolutePath) -> std::shared_ptr<const Texture>
		{
			const auto ubisprite = App::CreateSprite(absolutePath, 1, 1);
			if (!ubisprite)
				return nullptr;

			const auto width =
				*ubisprite.*g_CSimpleSpriteMemberPointerMTexWidth;
			const auto height =
				*ubisprite.*g_CSimpleSpriteMemberPointerMTexHeight;
			if (!width || !height) // no real other way to test if it failed as
								   // the constructor wont throw or anything...
			{
				delete ubisprite;
				return nullptr;
			}
			struct TextureDeleter
			{
				auto operator()(const Texture* texture) -> void
				{
					delete static_cast<CSimpleSprite*>(
						texture->m_internalPlatformHandle);
				}
			};
			const auto tex = new Texture;
			tex->m_width = width;
			tex->m_height = height;
			tex->m_internalPlatformHandle = ubisprite;
			return std::unique_ptr<const Texture, TextureDeleter>{tex};
		}

		auto DrawTexturedQuad(const TexturedQuad& quad) -> void
		{
			auto& sprite = *static_cast<CSimpleSprite*>(
				quad.m_texture->m_internalPlatformHandle);
			sprite.*g_CSimpleSpriteMemberPointerMXPos = 0.f;
			sprite.*g_CSimpleSpriteMemberPointerMYPos = 0.f;
			sprite.*g_CSimpleSpriteMemberPointerMAngle = 0.f;
			sprite.*g_CSimpleSpriteMemberPointerMScale = 1.f;
			auto& spritepts = sprite.*g_CSimpleSpriteMemberPointerMPoints;
			auto& spriteuvs = sprite.*g_CSimpleSpriteMemberPointerMUVCoords;
			for (int i = 0; i < std::size(spritepts) / 2; i++)
			{
				const auto x =
					(quad.m_vertices[i].x + 1.f) * (APP_VIRTUAL_WIDTH * 0.5f);
				const auto y =
					(quad.m_vertices[i].y + 1.f) * (APP_VIRTUAL_HEIGHT * 0.5f);
				spritepts[i * 2] = x;
				spritepts[i * 2 + 1] = y;
				const auto u = quad.m_textureCoords[i].x;
				const auto v = quad.m_textureCoords[i].y;
				spriteuvs[i * 2] = u;
				spriteuvs[i * 2 + 1] = v;
			}
			sprite.*g_CSimpleSpriteMemberPointerMRed = quad.m_colour.x;
			sprite.*g_CSimpleSpriteMemberPointerMGreen = quad.m_colour.y;
			sprite.*g_CSimpleSpriteMemberPointerMBlue = quad.m_colour.z;
			sprite.Draw();
		}
	}; // namespace GraphicsAbstraction

	namespace AudioAbstraction
	{

	};

	namespace InputAbstraction
	{
		auto IsForeground() -> oxyBool
		{
			return GetForegroundWindow() == MAIN_WINDOW_HANDLE;
		}

		auto HideAndLockCursor(oxyBool lock) -> void
		{
			POINT p;
			p.x = WINDOW_WIDTH / 2;
			p.y = WINDOW_HEIGHT / 2;
			ClientToScreen(MAIN_WINDOW_HANDLE, &p);
			if (lock && IsForeground() && !App::IsKeyPressed(VK_HOME))
			{
				RECT r;
				GetWindowRect(MAIN_WINDOW_HANDLE, &r);
				ClipCursor(&r);
				SetCursorPos(p.x, p.y);
				ShowCursor(FALSE);
			}
			else
			{
				ClipCursor(nullptr);
				ShowCursor(TRUE);
			}
		}
		auto GetMousePosition(oxyF32& x, oxyF32& y) -> void
		{
			if (!IsForeground())
			{
				// Return the center of the window
				x = static_cast<oxyF32>(WINDOW_WIDTH) / 2;
				y = static_cast<oxyF32>(WINDOW_HEIGHT) / 2;
				return;
			}
			App::GetMousePos(x, y);
#if APP_USE_VIRTUAL_RES
			APP_VIRTUAL_TO_NATIVE_COORDS(x, y);
			// conv ndc to screen space
			x = (x + 1.f) * (WINDOW_WIDTH * 0.5f);
			y = (y + 1.f) * (WINDOW_HEIGHT * 0.5f);
#endif
		}
		auto GetMouseStates(std::bitset<MouseButton_Count>& buttons) -> void
		{
			if (!IsForeground())
			{
				buttons.reset();
				return;
			}
#define SETBTNDOWN(idx, appParam) buttons[idx] = App::IsKeyPressed(appParam);
			SETBTNDOWN(MouseButton_Left, VK_LBUTTON);
			SETBTNDOWN(MouseButton_Right, VK_RBUTTON);
			SETBTNDOWN(MouseButton_Middle, VK_MBUTTON);
			SETBTNDOWN(MouseButton_X1, VK_XBUTTON1);
			SETBTNDOWN(MouseButton_X2, VK_XBUTTON2);
#undef SETBTNDOWN
		}
		auto GetKeyStates(std::bitset<KeyboardButton_Count>& keys) -> void
		{
			if (!IsForeground())
			{
				keys.reset();
				return;
			}
#define SETKEYDOWN(idx, appParam) keys[idx] = App::IsKeyPressed(appParam);
			SETKEYDOWN(KeyboardButton_A, 'A');
			SETKEYDOWN(KeyboardButton_B, 'B');
			SETKEYDOWN(KeyboardButton_C, 'C');
			SETKEYDOWN(KeyboardButton_D, 'D');
			SETKEYDOWN(KeyboardButton_E, 'E');
			SETKEYDOWN(KeyboardButton_F, 'F');
			SETKEYDOWN(KeyboardButton_G, 'G');
			SETKEYDOWN(KeyboardButton_H, 'H');
			SETKEYDOWN(KeyboardButton_I, 'I');
			SETKEYDOWN(KeyboardButton_J, 'J');
			SETKEYDOWN(KeyboardButton_K, 'K');
			SETKEYDOWN(KeyboardButton_L, 'L');
			SETKEYDOWN(KeyboardButton_M, 'M');
			SETKEYDOWN(KeyboardButton_N, 'N');
			SETKEYDOWN(KeyboardButton_O, 'O');
			SETKEYDOWN(KeyboardButton_P, 'P');
			SETKEYDOWN(KeyboardButton_Q, 'Q');
			SETKEYDOWN(KeyboardButton_R, 'R');
			SETKEYDOWN(KeyboardButton_S, 'S');
			SETKEYDOWN(KeyboardButton_T, 'T');
			SETKEYDOWN(KeyboardButton_U, 'U');
			SETKEYDOWN(KeyboardButton_V, 'V');
			SETKEYDOWN(KeyboardButton_W, 'W');
			SETKEYDOWN(KeyboardButton_X, 'X');
			SETKEYDOWN(KeyboardButton_Y, 'Y');
			SETKEYDOWN(KeyboardButton_Z, 'Z');
			SETKEYDOWN(KeyboardButton_0, '0');
			SETKEYDOWN(KeyboardButton_1, '1');
			SETKEYDOWN(KeyboardButton_2, '2');
			SETKEYDOWN(KeyboardButton_3, '3');
			SETKEYDOWN(KeyboardButton_4, '4');
			SETKEYDOWN(KeyboardButton_5, '5');
			SETKEYDOWN(KeyboardButton_6, '6');
			SETKEYDOWN(KeyboardButton_7, '7');
			SETKEYDOWN(KeyboardButton_8, '8');
			SETKEYDOWN(KeyboardButton_9, '9');
			SETKEYDOWN(KeyboardButton_F1, VK_F1);
			SETKEYDOWN(KeyboardButton_F2, VK_F2);
			SETKEYDOWN(KeyboardButton_F3, VK_F3);
			SETKEYDOWN(KeyboardButton_F4, VK_F4);
			SETKEYDOWN(KeyboardButton_F5, VK_F5);
			SETKEYDOWN(KeyboardButton_F6, VK_F6);
			SETKEYDOWN(KeyboardButton_F7, VK_F7);
			SETKEYDOWN(KeyboardButton_F8, VK_F8);
			SETKEYDOWN(KeyboardButton_F9, VK_F9);
			SETKEYDOWN(KeyboardButton_F10, VK_F10);
			SETKEYDOWN(KeyboardButton_F11, VK_F11);
			SETKEYDOWN(KeyboardButton_F12, VK_F12);
			SETKEYDOWN(KeyboardButton_F13, VK_F13);
			SETKEYDOWN(KeyboardButton_F14, VK_F14);
			SETKEYDOWN(KeyboardButton_F15, VK_F15);
			SETKEYDOWN(KeyboardButton_F16, VK_F16);
			SETKEYDOWN(KeyboardButton_F17, VK_F17);
			SETKEYDOWN(KeyboardButton_F18, VK_F18);
			SETKEYDOWN(KeyboardButton_F19, VK_F19);
			SETKEYDOWN(KeyboardButton_F20, VK_F20);
			SETKEYDOWN(KeyboardButton_F21, VK_F21);
			SETKEYDOWN(KeyboardButton_F22, VK_F22);
			SETKEYDOWN(KeyboardButton_F23, VK_F23);
			SETKEYDOWN(KeyboardButton_F24, VK_F24);
			SETKEYDOWN(KeyboardButton_NumPad0, VK_NUMPAD0);
			SETKEYDOWN(KeyboardButton_NumPad1, VK_NUMPAD1);
			SETKEYDOWN(KeyboardButton_NumPad2, VK_NUMPAD2);
			SETKEYDOWN(KeyboardButton_NumPad3, VK_NUMPAD3);
			SETKEYDOWN(KeyboardButton_NumPad4, VK_NUMPAD4);
			SETKEYDOWN(KeyboardButton_NumPad5, VK_NUMPAD5);
			SETKEYDOWN(KeyboardButton_NumPad6, VK_NUMPAD6);
			SETKEYDOWN(KeyboardButton_NumPad7, VK_NUMPAD7);
			SETKEYDOWN(KeyboardButton_NumPad8, VK_NUMPAD8);
			SETKEYDOWN(KeyboardButton_NumPad9, VK_NUMPAD9);
			SETKEYDOWN(KeyboardButton_NumPadDecimal, VK_DECIMAL);
			SETKEYDOWN(KeyboardButton_NumPadEnter, VK_RETURN);
			SETKEYDOWN(KeyboardButton_NumPadAdd, VK_ADD);
			SETKEYDOWN(KeyboardButton_NumPadSubtract, VK_SUBTRACT);
			SETKEYDOWN(KeyboardButton_NumPadMultiply, VK_MULTIPLY);
			SETKEYDOWN(KeyboardButton_NumPadDivide, VK_DIVIDE);
			SETKEYDOWN(KeyboardButton_NumPadLock, VK_NUMLOCK);
			SETKEYDOWN(KeyboardButton_Left, VK_LEFT);
			SETKEYDOWN(KeyboardButton_Right, VK_RIGHT);
			SETKEYDOWN(KeyboardButton_Up, VK_UP);
			SETKEYDOWN(KeyboardButton_Down, VK_DOWN);
			SETKEYDOWN(KeyboardButton_Home, VK_HOME);
			SETKEYDOWN(KeyboardButton_End, VK_END);
			SETKEYDOWN(KeyboardButton_PageUp, VK_PRIOR);
			SETKEYDOWN(KeyboardButton_PageDown, VK_NEXT);
			SETKEYDOWN(KeyboardButton_Insert, VK_INSERT);
			SETKEYDOWN(KeyboardButton_Delete, VK_DELETE);
			SETKEYDOWN(KeyboardButton_Pause, VK_PAUSE);
			SETKEYDOWN(KeyboardButton_PrintScreen, VK_SNAPSHOT);
			SETKEYDOWN(KeyboardButton_ScrollLock, VK_SCROLL);
			SETKEYDOWN(KeyboardButton_Escape, VK_ESCAPE);
			SETKEYDOWN(KeyboardButton_Backtick, VK_OEM_3);
			SETKEYDOWN(KeyboardButton_Tab, VK_TAB);
			SETKEYDOWN(KeyboardButton_CapsLock, VK_CAPITAL);
			SETKEYDOWN(KeyboardButton_LeftShift, VK_SHIFT);
			SETKEYDOWN(KeyboardButton_LeftControl, VK_CONTROL);
			SETKEYDOWN(KeyboardButton_LeftWindows, VK_LWIN);
			SETKEYDOWN(KeyboardButton_LeftAlt, VK_LMENU);
			SETKEYDOWN(KeyboardButton_Space, VK_SPACE);
			SETKEYDOWN(KeyboardButton_RightAlt, VK_RMENU);
			SETKEYDOWN(KeyboardButton_RightFunction, VK_RWIN);
			SETKEYDOWN(KeyboardButton_RightMenu, VK_APPS);
			SETKEYDOWN(KeyboardButton_RightControl, VK_RCONTROL);
			SETKEYDOWN(KeyboardButton_RightShift, VK_RSHIFT);
			SETKEYDOWN(KeyboardButton_Enter, VK_RETURN);
			SETKEYDOWN(KeyboardButton_Backspace, VK_BACK);
			SETKEYDOWN(KeyboardButton_Comma, VK_OEM_COMMA);
			SETKEYDOWN(KeyboardButton_Period, VK_OEM_PERIOD);
			SETKEYDOWN(KeyboardButton_Slash, VK_OEM_2);
			SETKEYDOWN(KeyboardButton_Semicolon, VK_OEM_1);
			SETKEYDOWN(KeyboardButton_Apostrophe, VK_OEM_7);
			SETKEYDOWN(KeyboardButton_LeftBracket, VK_OEM_4);
			SETKEYDOWN(KeyboardButton_RightBracket, VK_OEM_6);
			SETKEYDOWN(KeyboardButton_Backslash, VK_OEM_5);
			SETKEYDOWN(KeyboardButton_Hyphen, VK_OEM_MINUS);
			SETKEYDOWN(KeyboardButton_Equals, VK_OEM_PLUS);
#undef SETKEYDOWN
		}
		auto GetControllerConnected(int index) -> oxyBool
		{
			if (index >= MAX_CONTROLLERS)
				return false;
			const auto& controller =
				CSimpleControllers::GetInstance().GetController(index);
			return controller.*g_CControllerMemberPointerMConnected;
		}
		auto GetControllerStates(
			int index, std::bitset<ControllerButton_Count>& buttons) -> void
		{
			if (index >= MAX_CONTROLLERS)
				return;
			if (!IsForeground())
			{
				buttons.reset();
				return;
			}
			const auto& controller =
				CSimpleControllers::GetInstance().GetController(index);
			if (!(controller.*g_CControllerMemberPointerMConnected))
				return;
			buttons[ControllerButton_LeftThumb] =
				controller.CheckButton(XINPUT_GAMEPAD_LEFT_THUMB);
			buttons[ControllerButton_RightThumb] =
				controller.CheckButton(XINPUT_GAMEPAD_RIGHT_THUMB);
			buttons[ControllerButton_LeftShoulder] =
				controller.CheckButton(XINPUT_GAMEPAD_LEFT_SHOULDER);
			buttons[ControllerButton_RightShoulder] =
				controller.CheckButton(XINPUT_GAMEPAD_RIGHT_SHOULDER);
			buttons[ControllerButton_South] =
				controller.CheckButton(XINPUT_GAMEPAD_A);
			buttons[ControllerButton_East] =
				controller.CheckButton(XINPUT_GAMEPAD_B);
			buttons[ControllerButton_West] =
				controller.CheckButton(XINPUT_GAMEPAD_X);
			buttons[ControllerButton_North] =
				controller.CheckButton(XINPUT_GAMEPAD_Y);
			buttons[ControllerButton_StartSelect] =
				controller.CheckButton(XINPUT_GAMEPAD_START);
			buttons[ControllerButton_BackShare] =
				controller.CheckButton(XINPUT_GAMEPAD_BACK);
			buttons[ControllerButton_DPadUp] =
				controller.CheckButton(XINPUT_GAMEPAD_DPAD_UP);
			buttons[ControllerButton_DPadDown] =
				controller.CheckButton(XINPUT_GAMEPAD_DPAD_DOWN);
			buttons[ControllerButton_DPadLeft] =
				controller.CheckButton(XINPUT_GAMEPAD_DPAD_LEFT);
			buttons[ControllerButton_DPadRight] =
				controller.CheckButton(XINPUT_GAMEPAD_DPAD_RIGHT);
		}
		auto GetControllerAxisStates(
			int index, std::span<oxyF32, ControllerAxis_Count> axes) -> void
		{
			if (index >= MAX_CONTROLLERS)
				return;
			if (!IsForeground())
			{
				for (auto& a : axes)
					a = 0.f;
				return;
			}
			const auto& controller =
				CSimpleControllers::GetInstance().GetController(index);
			if (!(controller.*g_CControllerMemberPointerMConnected))
				return;
			axes[ControllerAxis_LeftThumbX] = controller.GetLeftThumbStickX();
			axes[ControllerAxis_LeftThumbY] = controller.GetLeftThumbStickY();
			axes[ControllerAxis_RightThumbX] = controller.GetRightThumbStickX();
			axes[ControllerAxis_RightThumbY] = controller.GetRightThumbStickY();
			axes[ControllerAxis_LeftTrigger] = controller.GetLeftTrigger();
			axes[ControllerAxis_RightTrigger] = controller.GetRightTrigger();
		}
	}; // namespace InputAbstraction

	namespace NetworkAbstraction
	{
		NetworkSocket::~NetworkSocket()
		{
			if (m_descriptor != INVALID_SOCKET)
				closesocket(m_descriptor);
		}

		auto NetworkSocket::Send(const void* data, oxySize size) -> oxySSize
		{
			if (m_descriptor == INVALID_SOCKET)
				return -1;
			const auto res = send(m_descriptor, static_cast<const char*>(data),
								  static_cast<int>(size), 0);
			if (res == SOCKET_ERROR)
				return -1;
			return res;
		}

		auto NetworkSocket::Receive(void* data, oxySize size) -> oxySSize
		{
			if (m_descriptor == INVALID_SOCKET)
				return -1;
			const auto res = recv(m_descriptor, static_cast<char*>(data),
								  static_cast<int>(size), 0);
			if (res < 0)
			{
				const auto err = WSAGetLastError();
				if (err == WSAETIMEDOUT)
					return 0;
				if (err == WSAEWOULDBLOCK)
					return 0;
				return -1;
			}
			return res;
		}

		auto NetworkSocket::Accept() -> std::unique_ptr<NetworkSocket>
		{
			if (m_descriptor == INVALID_SOCKET)
				return nullptr;
			const auto res = accept(m_descriptor, nullptr, nullptr);
			if (res == INVALID_SOCKET)
				return {};
			auto rv = std::make_unique<NetworkSocket>();
			rv->m_descriptor = res;

			sockaddr_in addr{};
			int len = sizeof addr;
			OXYCHECK(getpeername(res, reinterpret_cast<sockaddr*>(&addr),
								 &len) == 0);
			rv->m_address = addr.sin_addr.s_addr;
			rv->m_port = ntohs(addr.sin_port);

			// snd/rcv timeout
			const DWORD timeout = 400;
			setsockopt(rv->m_descriptor, SOL_SOCKET, SO_RCVTIMEO,
					   reinterpret_cast<const char*>(&timeout), sizeof timeout);
			setsockopt(rv->m_descriptor, SOL_SOCKET, SO_SNDTIMEO,
					   reinterpret_cast<const char*>(&timeout), sizeof timeout);

			return rv;
		}

		auto NetworkSocket::BroadcastMessage(const void* data, oxySize size)
			-> std::vector<std::string>
		{
			// client function
			// return vector of ip strings that responded
			if (m_descriptor == INVALID_SOCKET)
				return {};
			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_port = htons(m_port);
			addr.sin_addr.s_addr = INADDR_BROADCAST;
			sendto(m_descriptor, static_cast<const char*>(data),
				   static_cast<int>(size), 0,
				   reinterpret_cast<sockaddr*>(&addr), sizeof addr);

			LogMessage(std::format("Broadcast {} bytes\n", size).c_str());

			// receive responses
			std::vector<std::string> rv;
			
			char buffer[1024];
			while (true)
			{
				sockaddr_in from{};
				int fromlen = sizeof from;

				// Receive response
				const auto res =
					recvfrom(m_descriptor, buffer, sizeof(buffer), 0,
							 reinterpret_cast<sockaddr*>(&from), &fromlen);

				if (res < 0)
				{
					const auto err = WSAGetLastError();
					if (err == WSAETIMEDOUT || err == WSAEWOULDBLOCK)
					{
						// Timeout or no more responses
						break;
					}

					// Handle other errors
					LogMessage("BroadcastMessage error receiving response\n");
					return rv;
				}

				// turn ip into string
				const auto ip = ntohl(from.sin_addr.s_addr);
				const auto ipStr =
					std::format("{}.{}.{}.{}", (ip >> 24) & 0xFF,
								(ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
				rv.push_back(ipStr);
			}

			return rv;
		}

		auto NetworkSocket::RespondToBroadcasts(const void* data,
												oxySize size) -> void
		{
			// host function
			// respond to broadcasts with data
			if (m_descriptor == INVALID_SOCKET)
				return;

			// receive responses
			while (true)
			{
				char buffer[1024];
				sockaddr_in from{};
				int fromlen = sizeof from;
				const auto res =
					recvfrom(m_descriptor, buffer, sizeof buffer, 0,
							 reinterpret_cast<sockaddr*>(&from), &fromlen);
				if (res < 0)
				{
					const auto err = WSAGetLastError();
					if (err == WSAETIMEDOUT)
					{
						LogMessage("RespondBroadcast timeout\n");
						break;
					}
					if (err == WSAEWOULDBLOCK)
					{
						LogMessage("RespondBroadcast no more responses\n");
						break;
					}
					return;
				}

				// send response
				const  auto sres = sendto(m_descriptor, static_cast<const char*>(data),
					   static_cast<int>(size), 0,
					   reinterpret_cast<sockaddr*>(&from), fromlen);

				LogMessage("Responded to broadcast\n");
				LogMessage(std::format("Recv {} bytes\n", res).c_str());
				LogMessage(std::format("Sent {} bytes\n", sres).c_str());

			}
		}

		auto ConnectToHost(const char* host,
						   oxyU16 port) -> std::unique_ptr<NetworkSocket>
		{
			auto rv = std::make_unique<NetworkSocket>();
			rv->m_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (rv->m_descriptor == INVALID_SOCKET)
				return nullptr;
			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = inet_addr(host);

			// snd/rcv timeout
			const DWORD timeout = 400;
			setsockopt(rv->m_descriptor, SOL_SOCKET, SO_RCVTIMEO,
					   reinterpret_cast<const char*>(&timeout), sizeof timeout);
			setsockopt(rv->m_descriptor, SOL_SOCKET, SO_SNDTIMEO,
					   reinterpret_cast<const char*>(&timeout), sizeof timeout);

			if (connect(rv->m_descriptor, reinterpret_cast<sockaddr*>(&addr),
						sizeof addr) == SOCKET_ERROR)
			{
				closesocket(rv->m_descriptor);
				return nullptr;
			}
			rv->m_address = addr.sin_addr.s_addr;
			rv->m_port = port;
			return rv;
		}

		auto HostServer(oxyU16 port) -> std::unique_ptr<NetworkSocket>
		{
			auto rv = std::make_unique<NetworkSocket>();
			rv->m_descriptor = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (rv->m_descriptor == INVALID_SOCKET)
				return nullptr;
			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = INADDR_ANY;
			if (bind(rv->m_descriptor, reinterpret_cast<sockaddr*>(&addr),
					 sizeof addr) == SOCKET_ERROR)
			{
				closesocket(rv->m_descriptor);
				return nullptr;
			}
			if (listen(rv->m_descriptor, SOMAXCONN) == SOCKET_ERROR)
			{
				closesocket(rv->m_descriptor);
				return nullptr;
			}

			// non-blocking
			u_long mode = 1;
			ioctlsocket(rv->m_descriptor, FIONBIO, &mode);

			rv->m_address = addr.sin_addr.s_addr;
			rv->m_port = port;
			return rv;
		}

		auto
		CreateBroadcastSendSocket(oxyU16 port) -> std::unique_ptr<NetworkSocket>
		{
			// client function
			auto rv = std::make_unique<NetworkSocket>();
			rv->m_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (rv->m_descriptor == INVALID_SOCKET)
				return nullptr;
			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = INADDR_BROADCAST;

			// snd/rcv timeout
			const DWORD timeout = 2000;
			setsockopt(rv->m_descriptor, SOL_SOCKET, SO_RCVTIMEO,
					   reinterpret_cast<const char*>(&timeout), sizeof timeout);
			setsockopt(rv->m_descriptor, SOL_SOCKET, SO_SNDTIMEO,
					   reinterpret_cast<const char*>(&timeout), sizeof timeout);

			// broadcast
			const int broadcast = 1;
			setsockopt(rv->m_descriptor, SOL_SOCKET, SO_BROADCAST,
					   reinterpret_cast<const char*>(&broadcast),
					   sizeof broadcast);

			rv->m_address = addr.sin_addr.s_addr;
			rv->m_port = port;
			return rv;
		}

		auto CreateBroadcastListenSocket(oxyU16 port)
			-> std::unique_ptr<NetworkSocket>
		{
			// host function
			auto rv = std::make_unique<NetworkSocket>();
			rv->m_descriptor = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			if (rv->m_descriptor == INVALID_SOCKET)
				return nullptr;
			sockaddr_in addr{};
			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			addr.sin_addr.s_addr = INADDR_ANY;

			// snd/rcv timeout
			const DWORD timeout = 2000;
			setsockopt(rv->m_descriptor, SOL_SOCKET, SO_RCVTIMEO,
					   reinterpret_cast<const char*>(&timeout), sizeof timeout);
			setsockopt(rv->m_descriptor, SOL_SOCKET, SO_SNDTIMEO,
					   reinterpret_cast<const char*>(&timeout), sizeof timeout);

			// broadcast
			const auto broadcast = 1;
			setsockopt(rv->m_descriptor, SOL_SOCKET, SO_BROADCAST,
					   reinterpret_cast<const char*>(&broadcast),
					   sizeof broadcast);

			if (bind(rv->m_descriptor, reinterpret_cast<sockaddr*>(&addr),
					 sizeof addr) == SOCKET_ERROR)
			{
				return nullptr;
			}

			rv->m_address = addr.sin_addr.s_addr;
			rv->m_port = port;
			return rv;
		}

	} // namespace NetworkAbstraction
}; // namespace oxygen
