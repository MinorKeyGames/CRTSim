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

#include "Helpers.h"

LRESULT CALLBACK CommonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	default:
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void ReadFileContents(const char* Filename, char*& OutBuffer, UINT& OutBufferLen)
{
	FILE* pFile = NULL;
	fopen_s(&pFile, Filename, "rb");

	fseek(pFile, 0, SEEK_END);
	OutBufferLen = ftell(pFile);
	fseek(pFile, 0, SEEK_SET);

	OutBuffer = new char[OutBufferLen];

	fread_s(OutBuffer, OutBufferLen, sizeof(char), OutBufferLen, pFile);
}

ID3DXEffect* LoadEffect(const char* Filename, IDirect3DDevice9* pDevice)
{
	char* FileContents = NULL;
	UINT ContentLen = 0;
	ReadFileContents(Filename, FileContents, ContentLen);

	ID3DXEffect* RetVal = NULL;
	ID3DXBuffer* pError = NULL;
	HRESULT hRes = D3DXCreateEffect(pDevice, FileContents, ContentLen, NULL, NULL, 0, NULL, &RetVal, &pError);

	if (FileContents)
	{
		delete[] FileContents;
	}

	if (hRes != D3D_OK)
	{
		const char* ErrorMessageBuffer = NULL;

		if (pError)
		{
			ErrorMessageBuffer = (const char*)pError->GetBufferPointer();
		}
		else
		{
			ErrorMessageBuffer = "Error loading effect file";
		}

		MessageBox(NULL, ErrorMessageBuffer, "Shader Compilation Error", MB_ICONERROR);
	}

	return RetVal;
}
