#pragma once
#include "App/app.h"

// https://web.archive.org/web/20120401132446/http://bloglitb.blogspot.com/2011/12/access-to-private-members-safer.html
// This technique was invented by litb in 2010, long before
// templates were supercharged in C++17. Inspired by this method, I have
// reimplemented it using more modern C++ features.
//
// https://www.youtube.com/watch?v=SmlLdd1Q2V8
/*
14.7.2p8 The usual access checking rules do not apply to names used to specify
explicit instantiations. [Note: In particular, the template arguments and names
used in the function declarator (including parameter types, return types and
exception specifications) may be private types or objects which would normally
not be accessible and the template may be a member template or member function
which would not normally be accessible.]
*/
namespace oxygen
{
	namespace
	{

		// When explicitly instantiated, this assigns a
		// non-type-template-parameter value to a reference during global
		// initialization
		template <auto& Where, auto What>
			requires std::convertible_to<decltype(Where), decltype(What)>
		struct NTTPAssigner
		{
			static inline decltype(auto) s_assignmentReturnResult{Where = What};
		};

		// int PrivateClass::*g_privateClassBPointer{};
		// template struct NTTPAssigner<g_privateClassBPointer,
		// &PrivateClass::b>;
		// Effectively, this is roughly equivalent to:
		// int PrivateClass::*g_privateClassBPointer = &PrivateClass::b;
	}; // namespace
	namespace GraphicsAbstraction
	{
		// CSimpleSprite:
		// float m_xpos;
		static inline float CSimpleSprite::*g_CSimpleSpriteMemberPointerMXPos{};
		template struct NTTPAssigner<g_CSimpleSpriteMemberPointerMXPos,
									 &CSimpleSprite::m_xpos>;
		// float m_ypos;
		static inline float CSimpleSprite::*g_CSimpleSpriteMemberPointerMYPos{};
		template struct NTTPAssigner<g_CSimpleSpriteMemberPointerMYPos,
									 &CSimpleSprite::m_ypos>;
		// int m_texWidth;
		static inline int CSimpleSprite::*
			g_CSimpleSpriteMemberPointerMTexWidth{};
		template struct NTTPAssigner<g_CSimpleSpriteMemberPointerMTexWidth,
									 &CSimpleSprite::m_texWidth>;

		// int m_texHeight;
		static inline int CSimpleSprite::*
			g_CSimpleSpriteMemberPointerMTexHeight{};
		template struct NTTPAssigner<g_CSimpleSpriteMemberPointerMTexHeight,
									 &CSimpleSprite::m_texHeight>;

		// float m_angle;
		static inline float CSimpleSprite::*
			g_CSimpleSpriteMemberPointerMAngle{};
		template struct NTTPAssigner<g_CSimpleSpriteMemberPointerMAngle,
									 &CSimpleSprite::m_angle>;
		// float m_scale;
		static inline float CSimpleSprite::*
			g_CSimpleSpriteMemberPointerMScale{};
		template struct NTTPAssigner<g_CSimpleSpriteMemberPointerMScale,
									 &CSimpleSprite::m_scale>;
		// float m_points[8];
		static inline float (
			CSimpleSprite::*g_CSimpleSpriteMemberPointerMPoints)[8]{};
		template struct NTTPAssigner<g_CSimpleSpriteMemberPointerMPoints,
									 &CSimpleSprite::m_points>;
		// float m_uvcoords[8];
		static inline float (
			CSimpleSprite::*g_CSimpleSpriteMemberPointerMUVCoords)[8]{};
		template struct NTTPAssigner<g_CSimpleSpriteMemberPointerMUVCoords,
									 &CSimpleSprite::m_uvcoords>;
		// float m_red;
		static inline float CSimpleSprite::*g_CSimpleSpriteMemberPointerMRed{};
		template struct NTTPAssigner<g_CSimpleSpriteMemberPointerMRed,
									 &CSimpleSprite::m_red>;
		// float m_green;
		static inline float CSimpleSprite::*
			g_CSimpleSpriteMemberPointerMGreen{};
		template struct NTTPAssigner<g_CSimpleSpriteMemberPointerMGreen,
									 &CSimpleSprite::m_green>;
		// float m_blue;
		static inline float CSimpleSprite::*g_CSimpleSpriteMemberPointerMBlue{};
		template struct NTTPAssigner<g_CSimpleSpriteMemberPointerMBlue,
									 &CSimpleSprite::m_blue>;
	}; // namespace GraphicsAbstraction

	namespace InputAbstraction
	{
		// bool m_bConnected = false;
		static inline bool CController::*g_CControllerMemberPointerMConnected{};
		template struct NTTPAssigner<g_CControllerMemberPointerMConnected,
									 &CController::m_bConnected>;
	}; // namespace InputAbstraction
};	   // namespace oxygen