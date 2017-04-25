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

#include "FullscreenQuad.h"

void MakeFullscreenQuad(IDirect3DDevice9* pDevice, FullscreenQuad*& pOutQuad, int Width, int Height)
{
	pOutQuad = new FullscreenQuad();

	HRESULT hRes = D3D_OK;

	int NumVertices = 3;
	int NumIndices = 3;
	UINT IndexSize = (UINT)sizeof(unsigned short);
	void* pvBuffer = NULL;

	pOutQuad->pVB_Position = NULL;
	pOutQuad->pVB_TexCoord = NULL;
	hRes = pDevice->CreateVertexBuffer(NumVertices * 3 * sizeof(float), 0, 0, D3DPOOL_MANAGED, &pOutQuad->pVB_Position, NULL);
	hRes = pDevice->CreateVertexBuffer(NumVertices * 2 * sizeof(float), 0, 0, D3DPOOL_MANAGED, &pOutQuad->pVB_TexCoord, NULL);

	pOutQuad->pVD = NULL;
	D3DVERTEXELEMENT9 VertexElements[3] =
	{
		{0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{1, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};
	hRes = pDevice->CreateVertexDeclaration(VertexElements, &pOutQuad->pVD);

	hRes = pOutQuad->pVB_Position->Lock(0, 0, &pvBuffer, 0);
	float* pBuffer_Position = (float*)pvBuffer;
	pBuffer_Position[0] = -1.0f; pBuffer_Position[1] = +1.0f; pBuffer_Position[2] = 0.0f;
	pBuffer_Position[3] = -1.0f; pBuffer_Position[4] = -3.0f; pBuffer_Position[5] = 0.0f;
	pBuffer_Position[6] = +3.0f; pBuffer_Position[7] = +1.0f; pBuffer_Position[8] = 0.0f;
	hRes = pOutQuad->pVB_Position->Unlock();

	float U1 = (1.0f / (float)(Width * 2));
	float V1 = (1.0f / (float)(Height * 2));
	float U2 = 2.0f + U1;
	float V2 = 2.0f + V1;
	hRes = pOutQuad->pVB_TexCoord->Lock(0, 0, &pvBuffer, 0);
	float* pBuffer_TexCoord = (float*)pvBuffer;
	pBuffer_TexCoord[0] = U1; pBuffer_TexCoord[1] = V1;
	pBuffer_TexCoord[2] = U1; pBuffer_TexCoord[3] = V2;
	pBuffer_TexCoord[4] = U2; pBuffer_TexCoord[5] = V1;
	hRes = pOutQuad->pVB_TexCoord->Unlock();

	pOutQuad->pIB = NULL;
	hRes = pDevice->CreateIndexBuffer(NumIndices * IndexSize, 0, D3DFMT_INDEX16, D3DPOOL_MANAGED, &pOutQuad->pIB, NULL);

	hRes = pOutQuad->pIB->Lock(0, 0, &pvBuffer, 0);
	unsigned short* pBuffer_Index = (unsigned short*)pvBuffer;
	pBuffer_Index[0] = 0;
	pBuffer_Index[1] = 1;
	pBuffer_Index[2] = 2;
	hRes = pOutQuad->pIB->Unlock();
}
