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
#include "Parameters.h"
#include "Helpers.h"
#include "M3D.h"
#include "FullscreenQuad.h"

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow);
void CreateAssets();
void CreateAssets_Windows();
void CreateAssets_D3D();
void MainLoop();
void Render();
