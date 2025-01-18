#pragma once

#include <cstdint>

using oxyU8 = std::uint8_t;
using oxyU16 = std::uint16_t;
using oxyU32 = std::uint32_t;
using oxyU64 = std::uint64_t;
using oxyS8 = std::int8_t;
using oxyS16 = std::int16_t;
using oxyS32 = std::int32_t;
using oxyS64 = std::int64_t;
using oxyF32 = float;
using oxyF64 = double;

using oxyBool = bool;

using oxyChar = char;
using oxyWChar = wchar_t;

using oxySize = size_t;
using oxySSize = ptrdiff_t;

using oxyObjectID = oxyU64;

#include "Math/Defs.h"
#include "Math/Hash.h"
#include "Math/Random.h"

#include "Containers/SPSCQueue.h"

namespace oxygen
{
	/**
	 * @brief A non-copyable type to be inherited from. Expresses clearly that a
	 * type cannot be copied.
	 *
	 */
	struct NonCopyable
	{
		NonCopyable() = default;
		NonCopyable(const NonCopyable&) = delete;
		NonCopyable& operator=(const NonCopyable&) = delete;
	};

	enum ControllerButton : oxyU8
	{
		ControllerButton_LeftThumb,
		ControllerButton_RightThumb,
		ControllerButton_LeftShoulder,
		ControllerButton_RightShoulder,
		ControllerButton_South,
		ControllerButton_East,
		ControllerButton_West,
		ControllerButton_North,
		ControllerButton_StartSelect,
		ControllerButton_BackShare,
		ControllerButton_DPadUp,
		ControllerButton_DPadDown,
		ControllerButton_DPadLeft,
		ControllerButton_DPadRight,
		ControllerButton_Count
	};

	enum ControllerAxis : oxyU8
	{
		ControllerAxis_LeftThumbX,
		ControllerAxis_LeftThumbY,
		ControllerAxis_RightThumbX,
		ControllerAxis_RightThumbY,
		ControllerAxis_LeftTrigger,
		ControllerAxis_RightTrigger,
		ControllerAxis_Count
	};

	enum MouseButton : oxyU8
	{
		MouseButton_Left,
		MouseButton_Right,
		MouseButton_Middle,
		MouseButton_X1,
		MouseButton_X2,
		MouseButton_Count
	};

	enum KeyboardButton : oxyU8
	{
		// A-Z
		KeyboardButton_A,
		KeyboardButton_B,
		KeyboardButton_C,
		KeyboardButton_D,
		KeyboardButton_E,
		KeyboardButton_F,
		KeyboardButton_G,
		KeyboardButton_H,
		KeyboardButton_I,
		KeyboardButton_J,
		KeyboardButton_K,
		KeyboardButton_L,
		KeyboardButton_M,
		KeyboardButton_N,
		KeyboardButton_O,
		KeyboardButton_P,
		KeyboardButton_Q,
		KeyboardButton_R,
		KeyboardButton_S,
		KeyboardButton_T,
		KeyboardButton_U,
		KeyboardButton_V,
		KeyboardButton_W,
		KeyboardButton_X,
		KeyboardButton_Y,
		KeyboardButton_Z,

		// 0-9
		KeyboardButton_0,
		KeyboardButton_1,
		KeyboardButton_2,
		KeyboardButton_3,
		KeyboardButton_4,
		KeyboardButton_5,
		KeyboardButton_6,
		KeyboardButton_7,
		KeyboardButton_8,
		KeyboardButton_9,

		// F1-F24
		KeyboardButton_F1,
		KeyboardButton_F2,
		KeyboardButton_F3,
		KeyboardButton_F4,
		KeyboardButton_F5,
		KeyboardButton_F6,
		KeyboardButton_F7,
		KeyboardButton_F8,
		KeyboardButton_F9,
		KeyboardButton_F10,
		KeyboardButton_F11,
		KeyboardButton_F12,
		KeyboardButton_F13,
		KeyboardButton_F14,
		KeyboardButton_F15,
		KeyboardButton_F16,
		KeyboardButton_F17,
		KeyboardButton_F18,
		KeyboardButton_F19,
		KeyboardButton_F20,
		KeyboardButton_F21,
		KeyboardButton_F22,
		KeyboardButton_F23,
		KeyboardButton_F24,

		// Numpad 0-9
		KeyboardButton_NumPad0,
		KeyboardButton_NumPad1,
		KeyboardButton_NumPad2,
		KeyboardButton_NumPad3,
		KeyboardButton_NumPad4,
		KeyboardButton_NumPad5,
		KeyboardButton_NumPad6,
		KeyboardButton_NumPad7,
		KeyboardButton_NumPad8,
		KeyboardButton_NumPad9,

		// Numpad etc
		KeyboardButton_NumPadDecimal,
		KeyboardButton_NumPadEnter,
		KeyboardButton_NumPadAdd,
		KeyboardButton_NumPadSubtract,
		KeyboardButton_NumPadMultiply,
		KeyboardButton_NumPadDivide,
		KeyboardButton_NumPadLock,

		// Arrow keys
		KeyboardButton_Left,
		KeyboardButton_Right,
		KeyboardButton_Up,
		KeyboardButton_Down,

		// Home/End, Page Up/Down, Insert/Delete
		KeyboardButton_Home,
		KeyboardButton_End,
		KeyboardButton_PageUp,
		KeyboardButton_PageDown,
		KeyboardButton_Insert,
		KeyboardButton_Delete,

		// Pause/Break, Print Screen, Scroll Lock
		KeyboardButton_Pause,
		KeyboardButton_PrintScreen,
		KeyboardButton_ScrollLock,

		// Escape, Backtick, Tab, Caps Lock, Left Shift, Left Control, Left
		// Windows, Left Alt
		KeyboardButton_Escape,
		KeyboardButton_Backtick,
		KeyboardButton_Tab,
		KeyboardButton_CapsLock,
		KeyboardButton_LeftShift,
		KeyboardButton_LeftControl,
		KeyboardButton_LeftWindows,
		KeyboardButton_LeftAlt,

		// Space
		KeyboardButton_Space,

		// Right Alt, Right Function, Right Menu, Right Control, Right Shift,
		// Enter, Backspace
		KeyboardButton_RightAlt,
		KeyboardButton_RightFunction,
		KeyboardButton_RightMenu,
		KeyboardButton_RightControl,
		KeyboardButton_RightShift,
		KeyboardButton_Enter,
		KeyboardButton_Backspace,

		// Comma, Period, Slash, Semicolon, Apostrophe, Left Bracket, Right
		// Bracket, Backslash, Hyphen, Equals
		KeyboardButton_Comma,
		KeyboardButton_Period,
		KeyboardButton_Slash,
		KeyboardButton_Semicolon,
		KeyboardButton_Apostrophe,
		KeyboardButton_LeftBracket,
		KeyboardButton_RightBracket,
		KeyboardButton_Backslash,
		KeyboardButton_Hyphen,
		KeyboardButton_Equals,

		KeyboardButton_Count
	};

	enum CollisionHull : oxyU8
	{
		CollisionHull_None = 0xFF,
		CollisionHull_Point = 0,
		CollisionHull_Player,
		CollisionHull_PlayerCrouched,
		CollisionHull_Grenade,
	};

	inline constexpr oxyVec3 k_collisionHullMins[] = {
		oxyVec3{0.f, 0.f, 0.f},		  // CollisionHull_Point
		oxyVec3{-24.f, -24.f, -48.f}, // CollisionHull_Player
		oxyVec3{-24.f, -24.f, -48.f}, // CollisionHull_PlayerCrouched
		oxyVec3{-12.f, -12.f, -12.f},	  // CollisionHull_Grenade
	};
	inline constexpr oxyVec3 k_collisionHullMaxs[] = {
		oxyVec3{0.f, 0.f, 0.f},	   // CollisionHull_Point
		oxyVec3{24.f, 24.f, 48.f}, // CollisionHull_Player
		oxyVec3{24.f, 24.f, 48.f}, // CollisionHull_PlayerCrouched
		oxyVec3{12.f, 12.f, 12.f},	   // CollisionHull_Grenade
	};

	enum CollisionResponseType : oxyU8
	{
		CollisionResponseType_None,
		CollisionResponseType_Slide,
		CollisionResponseType_Bounce,
	};

	enum EntitySummonType : oxyU8
	{
		EntitySummonType_Player,
		EntitySummonType_Count
	};

	enum AnimationHash : oxyU32
	{
		AnimationHash_Idle = 0x7c161a2b,
		AnimationHash_RunForward = 0x947ec374,
		AnimationHash_RunBackward = 0x144ff8d,
		AnimationHash_Dying = 0x12c8a4ff,
		AnimationHash_Throw = 0x8e526e33,
	};

	enum EntitySpawnType : oxyU8
	{
		EntitySpawnType_Player,
		EntitySpawnType_Golfclub,
		EntitySpawnType_GolfclubLauncher,
		EntitySpawnType_Golfball,
		EntitySpawntype_GolfballLauncher,
		EntitySpawnType_Count
	};

	enum NetProtoMsgType : oxyU16
	{
		NetProtoMsgType_AnyPing = 0,

		// srv->cl
		// oxyU64: unique client id (only the client and host know this, not
		// other peers)
		NetProtoMsgType_SrvWelcome,

		// [utf8 string]: world name
		NetProtoMsgType_SrvChangeLevel,

		// oxyObjectID: entity id to possess
		NetProtoMsgType_SrvSetLocalPlayer,
		// oxyU16: EntitySpawnType
		// oxyVec3: position
		// oxyQuat: rotation
		// oxyU16: count of object ids
		// for each object id:
		// oxyObjectID: entity (if idx 0), or component id for spawning
		NetProtoMsgType_SrvEntitySpawn,

		// oxyObjectID: entity id
		NetProtoMsgType_SrvEntityDestroy,

		// oxyU16: count of entities in msg
		// for each entity:
		// oxyObjectID: entity id
		// oxyVec3: position
		// oxyQuat: rotation
		NetProtoMsgType_SrvEntityTransformRepl,

		// oxyObjectID: health component id
		// oxyS32: health
		// oxyS32: max health
		// oxyU8: health state
		NetProtoMsgType_SrvHealhComponentChange,

		// oxyObjectID: pawn component id
		// oxyObjectID: weapon component id
		NetProtoMsgType_SrvPawnPickupWeapon,

		// oxyObjectID: pawn component id
		NetProtoMsgType_SrvPawnDropWeapon,

		// client->srv
		// oxyVec3: position
		// oxyQuat: rotation
		NetProtoMsgType_CliLocalPlayerEntityMove,

		// oxyObjectID: pawn component id
		NetProtoMsgType_CliPawnDropWeapon,

		// oxyVec3: euler
		// oxyVec3: position
		// oxyU8: righthanded
		NetProtoMsgType_CliLocalPlayerFireWeapon,

	};

	enum PickupType : oxyU8
	{
		PickupType_Health,
		PickupType_Ammo,
		PickupType_Weapon,
		PickupType_Count
	};

	enum WeaponFireType : oxyU8
	{
		WeaponFireType_Bullets,
		WeaponFireType_GolfClub,
		WeaponFireType_GolfBall,
		WeaponFireType_Count
	};

	enum HealthState : oxyU8
	{
		HealthState_Alive,
		HealthState_Invulnerable,
		HealthState_Dead,
	};

	enum DamageType : oxyU8
	{
		DamageType_None,
		DamageType_Explosive,
		DamageType_Bullet,
		DamageType_Melee,
		DamageType_FallDamage,
		DamageType_Count,
	};

	template <typename TRet, typename... TArgs> struct CallbackList
	{
		using fnptr_type = TRet (*)(void*, TArgs...);

		auto AddCallback(std::weak_ptr<void> obj, fnptr_type fn) -> void
		{
			m_callbacks.emplace_back(std::move(obj), fn);
		}

		template <typename TFun>
		auto IterateCallbacks(TFun&& fun, TArgs... args)

			-> void
			requires(not std::same_as<void, TRet>)
		{
			for (auto it = m_callbacks.begin(); it != m_callbacks.end();)
			{
				auto& [obj, fn] = *it;
				if (auto sp = obj.lock())
				{
					fun(fn(sp, std::forward<TArgs>(args)...));
					++it;
				}
				else
				{
					it = m_callbacks.erase(it);
				}
			}
		}

		template <typename TFun>
		auto IterateCallbacks(TFun&& fun, TArgs... args)

			-> void
		{
			for (auto it = m_callbacks.begin(); it != m_callbacks.end();)
			{
				auto& [obj, fn] = *it;
				if (auto sp = obj.lock())
				{
					fn(sp.get(), std::forward<TArgs>(args)...);
					fun();
					++it;
				}
				else
				{
					it = m_callbacks.erase(it);
				}
			}
		}

	  private:
		std::vector<std::pair<std::weak_ptr<void>, fnptr_type>> m_callbacks;
	};

}; // namespace oxygen