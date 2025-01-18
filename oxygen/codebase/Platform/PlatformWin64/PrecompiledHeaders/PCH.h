#define OXYGEN_PLATFORM_WIN64

#include <intrin.h>

#define OXYENDIANLITTLE

#define OXYSOCKETDESCRIPTORTYPE oxyU64

//
// engine macros
//
// OXYDEBUGBREAK
// Switches the thread into single-step mode, allowing you to debug code
// line-by-line and continue execution if needed
#define OXYDEBUGBREAK() __writeeflags(__readeflags() | 0x100)

// OXYCHECK asserts in debug builds, but does not evaluate or execute in release
#ifdef OXYBUILDDEBUG
#define OXYCHECK(x)                                                            \
	do                                                                         \
	{                                                                          \
		if (!(x))                                                              \
		{                                                                      \
			OXYDEBUGBREAK();                                                   \
		}                                                                      \
	} while (0)
#else
#define OXYCHECK(x)                                                            \
	do                                                                         \
	{                                                                          \
	} while (0)
#endif
// OXYVERIFY
// OXYVERIFY asserts only in debug builds, but still evaluates and executes in
// release
#ifdef OXYBUILDDEBUG
#define OXYVERIFY(x)                                                           \
	do                                                                         \
	{                                                                          \
		if (!(x))                                                              \
		{                                                                      \
			OXYDEBUGBREAK();                                                   \
		}                                                                      \
	} while (0)
#else
#define OXYVERIFY(x)                                                           \
	do                                                                         \
	{                                                                          \
		x;                                                                     \
	} while (0)
#endif