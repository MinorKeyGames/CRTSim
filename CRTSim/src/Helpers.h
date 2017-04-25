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

#include "Includes.h"

LRESULT CALLBACK CommonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ReadFileContents(const char* Filename, char*& OutBuffer, UINT& OutBufferLen);
ID3DXEffect* LoadEffect(const char* Filename, IDirect3DDevice9* pDevice);
