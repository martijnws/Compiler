// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#define NOMINMAX

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include <commctrl.h>

#include <D3D11.h>
#include <d2d1.h>
#include <D2d1_1.h>
#include <DXGI1_2.h>
#include <dwrite.h>
#include <Dwrite_1.h>

#include <wincodec.h>
#include <Shlwapi.h>

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <cassert>
#include <iostream>

#include <ComLib\ComPtr.h>
