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

// M3D is my proprietary format for storing vertex and index data.
// This is a trimmed down implementation to load the files needed for this example.

enum EM3DStreamUsage
{
	SU_Position,
	SU_Normal,
	SU_Tangent,
	SU_Binormal,
	SU_Color,
	SU_Texcoord0,
	SU_Texcoord1,
};

struct M3DStream
{
	IDirect3DVertexBuffer9* pVB;
	UINT Stride;
};

struct M3DFile
{
	UINT NumStreams;
	UINT NumVertices;
	UINT NumIndices;

	M3DStream* Streams;
	IDirect3DVertexDeclaration9* pVD;
	IDirect3DIndexBuffer9* pIB;
};

void LoadM3DFile(const char* Filename, IDirect3DDevice9* pDevice, M3DFile*& pOutM3DFile);
