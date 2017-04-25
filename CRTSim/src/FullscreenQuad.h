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

struct FullscreenQuad
{
	IDirect3DVertexBuffer9* pVB_Position;
	IDirect3DVertexBuffer9* pVB_TexCoord;
	IDirect3DVertexDeclaration9* pVD;
	IDirect3DIndexBuffer9* pIB;
};

void MakeFullscreenQuad(IDirect3DDevice9* pDevice, FullscreenQuad*& pOutQuad, int Width, int Height);
