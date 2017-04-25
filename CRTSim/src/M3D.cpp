#include "M3D.h"

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

void LoadM3DFile(const char* Filename, IDirect3DDevice9* pDevice, M3DFile*& pOutM3DFile)
{
	pOutM3DFile = new M3DFile();

	HRESULT hRes = D3D_OK;
	void* pvBuffer = NULL;

	FILE* pFile = NULL;
	fopen_s(&pFile, Filename, "rb");

	unsigned int MagicNumber = 0;
	unsigned char MajorVersion = 0;
	unsigned char MinorVersion = 0;

	unsigned char Alignment[2] = {0, 0};

	pOutM3DFile->NumStreams = 0;
	pOutM3DFile->NumVertices = 0;
	pOutM3DFile->NumIndices = 0;

	fread_s(&MagicNumber, sizeof(MagicNumber), sizeof(MagicNumber), 1, pFile);
	fread_s(&MajorVersion, sizeof(MajorVersion), sizeof(MajorVersion), 1, pFile);
	fread_s(&MinorVersion, sizeof(MinorVersion), sizeof(MinorVersion), 1, pFile);
	fread_s(Alignment, sizeof(unsigned char) * 2, sizeof(unsigned char), 2, pFile);
	fread_s(&pOutM3DFile->NumStreams, sizeof(pOutM3DFile->NumStreams), sizeof(pOutM3DFile->NumStreams), 1, pFile);
	fread_s(&pOutM3DFile->NumVertices, sizeof(pOutM3DFile->NumVertices), sizeof(pOutM3DFile->NumVertices), 1, pFile);
	fread_s(&pOutM3DFile->NumIndices, sizeof(pOutM3DFile->NumIndices), sizeof(pOutM3DFile->NumIndices), 1, pFile);

	bool bUnused = false;
	fread_s(&bUnused, sizeof(bUnused), sizeof(bUnused), 1, pFile);

	UINT IndexSize = 0;
	fread_s(&IndexSize, sizeof(IndexSize), sizeof(IndexSize), 1, pFile);

	hRes = pDevice->CreateIndexBuffer(pOutM3DFile->NumIndices * IndexSize, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pOutM3DFile->pIB, NULL);
	hRes = pOutM3DFile->pIB->Lock(0, 0, &pvBuffer, 0);
	unsigned short* pBuffer_Index = (unsigned short*)pvBuffer;
	fread_s(pBuffer_Index, sizeof(unsigned short) * pOutM3DFile->NumIndices, sizeof(unsigned short), pOutM3DFile->NumIndices, pFile);
	hRes = pOutM3DFile->pIB->Unlock();

	D3DVERTEXELEMENT9* VertexElements = new D3DVERTEXELEMENT9[pOutM3DFile->NumStreams + 1];

	pOutM3DFile->Streams = new M3DStream[pOutM3DFile->NumStreams];
	for (UINT i = 0; i < pOutM3DFile->NumStreams; ++i)
	{
		VertexElements[i].Stream = i;
		VertexElements[i].Offset = 0;
		VertexElements[i].Method = D3DDECLMETHOD_DEFAULT;
		VertexElements[i].UsageIndex = 0;

		EM3DStreamUsage Usage;

		fread_s(&Usage, sizeof(Usage), sizeof(Usage), 1, pFile);
		fread_s(&pOutM3DFile->Streams[i].Stride, sizeof(pOutM3DFile->Streams[i].Stride), sizeof(pOutM3DFile->Streams[i].Stride), 1, pFile);

		hRes = pDevice->CreateVertexBuffer(pOutM3DFile->NumVertices * pOutM3DFile->Streams[i].Stride, 0, 0, D3DPOOL_MANAGED, &pOutM3DFile->Streams[i].pVB, NULL);
		hRes = pOutM3DFile->Streams[i].pVB->Lock(0, 0, &pvBuffer, 0);

		UINT NumComponents = 0;

		switch (Usage)
		{
		case SU_Position:
		case SU_Normal:
		case SU_Texcoord0:
		case SU_Texcoord1:
			{
				NumComponents = pOutM3DFile->Streams[i].Stride / sizeof(float);
				float* pBuffer_Position = (float*)pvBuffer;
				fread_s(pBuffer_Position, sizeof(float) * NumComponents * pOutM3DFile->NumVertices, sizeof(float), NumComponents * pOutM3DFile->NumVertices, pFile);

				VertexElements[i].Type = BYTE(D3DDECLTYPE_FLOAT1 + (NumComponents - 1));

				switch (Usage)
				{
				case SU_Position:
					VertexElements[i].Usage = D3DDECLUSAGE_POSITION;
					break;
				case SU_Normal:
					VertexElements[i].Usage = D3DDECLUSAGE_NORMAL;
					break;
				case SU_Texcoord0:
				case SU_Texcoord1:
					VertexElements[i].Usage = D3DDECLUSAGE_TEXCOORD;
					VertexElements[i].UsageIndex = BYTE(Usage - SU_Texcoord0);
					break;
				}
			}
			break;

		case SU_Color:
			{
				NumComponents = pOutM3DFile->Streams[i].Stride / sizeof(unsigned int);
				unsigned int* pBuffer_Position = (unsigned int*)pvBuffer;
				fread_s(pBuffer_Position, sizeof(unsigned int) * NumComponents * pOutM3DFile->NumVertices, sizeof(unsigned int), NumComponents * pOutM3DFile->NumVertices, pFile);

				VertexElements[i].Type = D3DDECLTYPE_D3DCOLOR;
				VertexElements[i].Usage = D3DDECLUSAGE_COLOR;
			}
			break;

		default:
			break;
		}

		hRes = pOutM3DFile->Streams[i].pVB->Unlock();
	}

	VertexElements[pOutM3DFile->NumStreams].Stream = 0xff;
	VertexElements[pOutM3DFile->NumStreams].Offset = 0;
	VertexElements[pOutM3DFile->NumStreams].Type = D3DDECLTYPE_UNUSED;
	VertexElements[pOutM3DFile->NumStreams].Method = 0;
	VertexElements[pOutM3DFile->NumStreams].Usage = 0;
	VertexElements[pOutM3DFile->NumStreams].UsageIndex = 0;

	hRes = pDevice->CreateVertexDeclaration(VertexElements, &pOutM3DFile->pVD);
}
