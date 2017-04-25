//////////////////////////////////////////////////////////////////////////
//
// CC0 1.0 Universal (CC0 1.0)
// Public Domain Dedication 
//
// To the extent possible under law, J. Kyle Pittman has waived all
// copyright and related or neighboring rights to this implementation
// of CRT simulation. This work is published from the United States.
//
// For more information, please visit
// https://creativecommons.org/publicdomain/zero/1.0/
//
//////////////////////////////////////////////////////////////////////////

#pragma once

#define WIN32_LEAN_AND_MEAN

#define WINVER 0x0501
#define _WIN32_WINDOWS WINVER
#define _WIN32_WINNT WINVER

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

#pragma comment(lib, "shlwapi.lib")

#include <windows.h>

#include <stdio.h>

#include <shlwapi.h>

#include <d3d9.h>
#include <d3dx9.h>
