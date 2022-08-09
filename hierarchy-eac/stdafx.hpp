#pragma once

using uint8_t = unsigned char;
using uint16_t = unsigned short;
using uint32_t = unsigned int;
using uint64_t = unsigned long long;

#include <cstdarg>
#include <ntifs.h>
#include <ntimage.h>

class eac_instance_t;
extern eac_instance_t *g_eac_instance;

//
// EAC_SHA1_CREATE_HASH -> E8 ? ? ? ? 48 8D 55 3F
// EAC_IS_RELOC -> 48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 54 41 55 41 56 41 57 48 63 05
//

#define EAC_SHA1_CREATE_HASH(base) static_cast<uint8_t*>(base)+0xF054 
#define EAC_IS_RELOC(base) static_cast<uint8_t *>(base)+ 0x33F8
#define CURRENT_THREAD_ID PsGetCurrentThreadId()

#include "hde64.hpp"
#include "utils.hpp"
#include "eac.hpp"
#include "vm.hpp"