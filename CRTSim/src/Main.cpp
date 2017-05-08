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

#include "Main.h"

// I know this is just what you wanted to see at the start of this code: a big old block of GLOBAL VARIABLES! :D

HINSTANCE gInst = NULL;
HWND gSrcWnd = NULL;
HWND gDstWnd = NULL;
HDC gScreenDC = NULL;								// The device context for the desktop. Only used in this example because we're using the desktop as our "clean" source image. Games would not need this.
bool gEvenFrame = true;								// We keep track of alternating frames to flip between composite RTTs and also to stagger the NTSC artifacts texture.
IDirect3D9* gD3D = NULL;
IDirect3DDevice9* gDevice = NULL;
IDirect3DSurface9* gBackbuffer = NULL;				// The default backbuffer, saved off so we can restore it after switching to another RTT.
ID3DXEffect* gCompEffect = NULL;					// The effect for producing composite images of the current "clean" game scene and the previous frame's composite image.
ID3DXEffect* gScreenEffect = NULL;					// The effect for drawing the screen mesh.
ID3DXEffect* gFrameEffect = NULL;					// The effect for drawing the frame mesh.
ID3DXEffect* gPostEffect = NULL;					// The effect for downsampling and upsampling an image with blurring to produce bloom.
ID3DXEffect* gPresentEffect = NULL;					// The effect for blending all inputs together to produce the final image presented to the screen.
IDirect3DTexture9* gMaskTexture = NULL;				// The texture containing the shadow mask image. Loaded from disk.
IDirect3DTexture9* gArtifactsTexture = NULL;		// The texture containing the NTSC artifacts image. Loaded from disk.
IDirect3DTexture9* gRTT_Clean = NULL;				// The RTT that the "in-CRT" game scene gets drawn to. (Not actually a render target texture in this example, but in a typical game it would be.)
IDirect3DTexture9* gRTT_Composite_Even = NULL;		// One of two RTTs that the "in-CRT" composite images with artifacts get drawn to.
IDirect3DTexture9* gRTT_Composite_Odd = NULL;		// One of two RTTs that the "in-CRT" composite images with artifacts get drawn to.
IDirect3DTexture9* gRTT_Full = NULL;				// The RTT that the screen and frame meshes get drawn to.
IDirect3DTexture9* gRTT_Downsample = NULL;			// The RTT that the downsampled and blurred image gets drawn to.
IDirect3DTexture9* gRTT_Upsample = NULL;			// The RTT that the upsampled and blurred images gets drawn to.
IDirect3DTexture9* gCurrentTarget = NULL;			// A pointer to either gRTT_Composite_Even or gRTT_Composite_Odd depending on whether we're on an even or odd frame. This is what we draw to on this frame.
IDirect3DTexture9* gCurrentSource = NULL;			// The other one of gRTT_Composite_Even or gRTT_Composite_Odd, whichever gCurrentTarget doesn't point to. This is what we sample from on this frame.
M3DFile* gScreen;									// The mesh containing the face of the screen, where the "in-CRT" image will be drawn with "in-world" effects applied.
M3DFile* gFrame;									// The mesh containing the frame of the screen, where reflections of the "in-CRT" image will appear.
FullscreenQuad* gSrcQuad;							// A quad for drawing to the entirety of an RTT of the size of the source buffer (default 256x224 in this example). Accounts for half-pixel offsets inherent to D3D9.
FullscreenQuad* gDstQuad;							// A quad for drawing to the entirety of an RTT of the size of the destination buffer (default 1600x900 in this example).
FullscreenQuad* gDownsampleQuad;					// A quad for drawing to the entirety of an RTT a fraction of the size of the destination buffer (1/16 resolution in each dimension).

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR cmdLine, int cmdShow)
{
	// Set the current directory to the location of the executable for convenience.
	// Requires shlwapi to be included and linked.
	char ExePath[MAX_PATH] = {0};
	GetModuleFileName(NULL, ExePath, MAX_PATH);
	PathRemoveFileSpec(ExePath);
	SetCurrentDirectory(ExePath);

	gInst = hInst;

	CreateAssets();
	MainLoop();

	// TODO: Clean up assets after the main loop exits. Left as an exercise to the reader.
}

void MainLoop()
{
	// Loop and render until we get a quit message from closing either window.

	bool bDone = false;
	MSG winMsg;
	while (!bDone)
	{
		while (PeekMessage(&winMsg, NULL, 0, 0, PM_REMOVE))
		{
			if (winMsg.message == WM_QUIT)
			{
				bDone = true;
			}

			TranslateMessage(&winMsg);
			DispatchMessage(&winMsg);
		}

		Render();
	}
}

void CreateAssets()
{
	CreateAssets_Windows();
	CreateAssets_D3D();
}

void CreateAssets_Windows()
{
	// Build source and destination windows.
	// This is not applicable to a typical game scenario where you'd already have a main game window.

	WNDCLASSEX SrcWndClass, DstWndClass;

	ZeroMemory(&SrcWndClass, sizeof(WNDCLASSEX));
	ZeroMemory(&DstWndClass, sizeof(WNDCLASSEX));

	SrcWndClass.cbSize = sizeof(WNDCLASSEX);
	SrcWndClass.style = CS_HREDRAW | CS_VREDRAW;
	SrcWndClass.lpfnWndProc = CommonWndProc;
	SrcWndClass.hInstance = gInst;
	SrcWndClass.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
	SrcWndClass.lpszClassName = "Source Window Class Ex";

	DstWndClass.cbSize = sizeof(WNDCLASSEX);
	DstWndClass.style = CS_HREDRAW | CS_VREDRAW;
	DstWndClass.lpfnWndProc = CommonWndProc;
	DstWndClass.hInstance = gInst;
	DstWndClass.hbrBackground = GetSysColorBrush(COLOR_WINDOW);
	DstWndClass.lpszClassName = "Destination Window Class Ex";

	RegisterClassEx(&SrcWndClass);
	RegisterClassEx(&DstWndClass);

	DWORD WindowStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	DWORD SrcWindowExStyle = 0 | WS_EX_LAYERED;
	DWORD DstWindowExStyle = 0;

	RECT SrcRect;
	SrcRect.left = 64;
	SrcRect.right = SrcRect.left + Parameters::Init_SrcWidth;
	SrcRect.top = 64;
	SrcRect.bottom = SrcRect.top + Parameters::Init_SrcHeight;

	AdjustWindowRectEx(&SrcRect, WindowStyle, FALSE, SrcWindowExStyle);

	RECT DstRect;
	DstRect.left = SrcRect.right + 64;
	DstRect.right = DstRect.left + Parameters::Init_DstWidth;
	DstRect.top = 64;
	DstRect.bottom = DstRect.top + Parameters::Init_DstHeight;

	AdjustWindowRectEx(&DstRect, WindowStyle, FALSE, DstWindowExStyle);

	gSrcWnd = CreateWindowEx(SrcWindowExStyle, "Source Window Class Ex", "Source Window", WindowStyle, SrcRect.left, SrcRect.top, SrcRect.right - SrcRect.left, SrcRect.bottom - SrcRect.top, NULL, NULL, gInst, NULL);
	gDstWnd = CreateWindowEx(DstWindowExStyle, "Destination Window Class Ex", "Destination Window", WindowStyle, DstRect.left, DstRect.top, DstRect.right - DstRect.left, DstRect.bottom - DstRect.top, NULL, NULL, gInst, NULL);

	SetLayeredWindowAttributes(gSrcWnd, RGB(255,255,255), NULL, LWA_COLORKEY);

	ShowWindow(gSrcWnd, SW_NORMAL);
	ShowWindow(gDstWnd, SW_NORMAL);

	gScreenDC = GetDC(NULL);
}

void CreateAssets_D3D()
{
	// Initialize D3D9 and load all the assets we'll need for rendering.
	// None of this stuff gets cleaned up.

	D3DPRESENT_PARAMETERS Params;
	ZeroMemory(&Params, sizeof(D3DPRESENT_PARAMETERS));
	Params.EnableAutoDepthStencil = TRUE;
	Params.AutoDepthStencilFormat = D3DFMT_D24S8;
	Params.Windowed = TRUE;
	Params.FullScreen_RefreshRateInHz = 0;
	Params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	Params.SwapEffect = D3DSWAPEFFECT_COPY;
	Params.hDeviceWindow = gDstWnd;
	Params.MultiSampleType = D3DMULTISAMPLE_NONE;
	Params.MultiSampleQuality = 0;
	Params.BackBufferFormat = D3DFMT_X8R8G8B8;
	Params.BackBufferCount = 1;
	Params.BackBufferWidth = Parameters::Init_DstWidth;
	Params.BackBufferHeight = Parameters::Init_DstHeight;
	Params.Flags = 0;

	HRESULT hRes = D3D_OK;

	gD3D = Direct3DCreate9(D3D_SDK_VERSION);
	hRes = gD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, gDstWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &Params, &gDevice);

	// Build all our RTTs. Note that the "clean" game scene texture is marked as D3DUSAGE_DYNAMIC instead of D3DUSAGE_RENDERTARGET in this case,
	// because we're going to be blitting image data from the desktop into it. That's unique to this example; in most games this would be D3DUSAGE_RENDERTARGET.
	hRes = gDevice->CreateTexture(Parameters::Init_SrcWidth, Parameters::Init_SrcHeight, 1, D3DUSAGE_DYNAMIC, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &gRTT_Clean, NULL);
	hRes = gDevice->CreateTexture(Parameters::Init_SrcWidth, Parameters::Init_SrcHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &gRTT_Composite_Even, NULL);
	hRes = gDevice->CreateTexture(Parameters::Init_SrcWidth, Parameters::Init_SrcHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &gRTT_Composite_Odd, NULL);

	hRes = gDevice->CreateTexture(Parameters::Init_DstWidth, Parameters::Init_DstHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &gRTT_Full, NULL);
	hRes = gDevice->CreateTexture(Parameters::Init_DstWidth / 16, Parameters::Init_DstHeight / 16, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &gRTT_Downsample, NULL);
	hRes = gDevice->CreateTexture(Parameters::Init_DstWidth, Parameters::Init_DstHeight, 1, D3DUSAGE_RENDERTARGET, D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, &gRTT_Upsample, NULL);

	// Load our textures.
	hRes = D3DXCreateTextureFromFileEx(gDevice, "mask.bmp", 64, 32, 0, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &gMaskTexture);
	hRes = D3DXCreateTextureFromFileEx(gDevice, "artifacts.bmp", 256, 224, 1, 0, D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, D3DX_FILTER_NONE, D3DX_FILTER_NONE, 0, NULL, NULL, &gArtifactsTexture);

	// Load and compile all our effect files.
	gCompEffect = LoadEffect("composite.fx", gDevice);
	gScreenEffect = LoadEffect("screen.fx", gDevice);
	gFrameEffect = LoadEffect("frame.fx", gDevice);
	gPostEffect = LoadEffect("post.fx", gDevice);
	gPresentEffect = LoadEffect("present.fx", gDevice);


	// Load the screen and frame meshes, which have been saved in a proprietary format.
	LoadM3DFile("screen.m3d", gDevice, gScreen);
	LoadM3DFile("frame.m3d", gDevice, gFrame);

	// Generate quads for drawing to RTTs of known sizes.
	// The only reason we need more than one of these is because of the half-pixel offset in D3D9.
	MakeFullscreenQuad(gDevice, gSrcQuad, Parameters::Init_SrcWidth, Parameters::Init_SrcHeight);
	MakeFullscreenQuad(gDevice, gDstQuad, Parameters::Init_DstWidth, Parameters::Init_DstHeight);
	MakeFullscreenQuad(gDevice, gDownsampleQuad, Parameters::Init_DstWidth / 16, Parameters::Init_DstHeight / 16);

	// And finally, save off the default backbuffer so we can restore it before the final draw call.
	hRes = gDevice->GetRenderTarget(0, &gBackbuffer);
}

void Render()
{
	// This is the function that does all the heavy lifting.
	// See the individual .fx files for HLSL implementations.

	HRESULT hRes = D3D_OK;

	// Set up this frame to sample from the previous frame's composite image and draw to the other one.
	if (gEvenFrame)
	{
		gCurrentTarget = gRTT_Composite_Even;
		gCurrentSource = gRTT_Composite_Odd;
	}
	else
	{
		gCurrentTarget = gRTT_Composite_Odd;
		gCurrentSource = gRTT_Composite_Even;
	}

	// Build the current "clean" game scene.
	// (In this example, I blit the contents from beneath the source window into the RTT,
	// but for an actual game, we'd render the game scene to this RTT instead.)
	{
		IDirect3DSurface9* pSurf = NULL;
		hRes = gRTT_Clean->GetSurfaceLevel(0, &pSurf);

		D3DSURFACE_DESC Desc;
		hRes = pSurf->GetDesc(&Desc);

		HDC SurfDC = NULL;
		hRes = pSurf->GetDC(&SurfDC);

		POINT TopLeft;
		TopLeft.x = 0;
		TopLeft.y = 0;
		ClientToScreen(gSrcWnd, &TopLeft);

		BOOL bBlit = BitBlt(SurfDC, 0, 0, Parameters::Init_SrcWidth, Parameters::Init_SrcHeight, gScreenDC, TopLeft.x, TopLeft.y, SRCCOPY);

		pSurf->ReleaseDC(SurfDC);

		pSurf->Release();
	}

	// Now we start the actual rendering phase.
	// The process looks like this:
	// 1. Build the new composite image from the current "clean" scene and the previous frame's composite.
	// 2. Draw the screen and frame meshes, sampling from the new composite image (output of step 1).
	// 3. Downsample and blur the buffer containing the screen and frame (output of step 2).
	// 4. Upsamples and blur the blur containing the downsampled image (output of step 3).
	// 5. Blend the outputs of steps 2 and 4, drawing to the backbuffer for presentation to the screen.
	{
		// Define the transforms for our camera position.
		// The distance of the camera from the screen and frame meshes is a function of its FOV.
		float DegreesToRadians = 3.141592653589793f / 180.0f;
		float DefaultFOV = 15.0f;

		float xp = 1.0f / sin(DefaultFOV * 0.5f * DegreesToRadians);
		float Distance = xp * cos(DefaultFOV * 0.5f * DegreesToRadians);

		D3DXVECTOR3 CamPos(-Distance, 0.0f, 0.0f);
		D3DXVECTOR3 LookPos(0.0f, 0.0f, 0.0f);
		D3DXVECTOR3 UpDir(0.0f, 0.0f, 1.0f);

		D3DXVECTOR4 CamPos4(CamPos, 0.0f);

		D3DXMATRIX WorldMat;
		D3DXMatrixIdentity(&WorldMat);

		D3DXMATRIX ViewMat;
		D3DXMatrixLookAtRH(&ViewMat, &CamPos, &LookPos, &UpDir);

		D3DXMATRIX ProjMat;
		D3DXMatrixPerspectiveFovRH(&ProjMat, 15.0f * 3.141592653589793f / 180.0f, (float)Parameters::Init_DstWidth / (float)Parameters::Init_DstHeight, 1.0f, 100.0f);

		D3DXMATRIX WVPMat = WorldMat * ViewMat * ProjMat;

		IDirect3DSurface9* pSurf = NULL;

		// Now let's begin rendering for reals.
		hRes = gDevice->BeginScene();

		// Step 1. Build the composite image.
		{
			hRes = gCurrentTarget->GetSurfaceLevel(0, &pSurf);
			hRes = gDevice->SetRenderTarget(0, pSurf);
			pSurf->Release();

			hRes = gDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255,0,128,255), 1.0f, 0);

			D3DXVECTOR4 RcpScrWidth(1.0f / (float)Parameters::Init_SrcWidth, 0.0f, 0.0f, 0.0f);
			D3DXVECTOR4 RcpScrHeight(0.0f, 1.0f / (float)Parameters::Init_SrcHeight, 0.0f, 0.0f);
			hRes = gCompEffect->SetVector(gCompEffect->GetParameterByName(NULL, "RcpScrWidth"), &RcpScrWidth);
			hRes = gCompEffect->SetVector(gCompEffect->GetParameterByName(NULL, "RcpScrHeight"), &RcpScrHeight);

			hRes = gCompEffect->SetFloat(gCompEffect->GetParameterByName(NULL, "Tuning_Sharp"), Parameters::Tuning_Sharp);
			hRes = gCompEffect->SetVector(gCompEffect->GetParameterByName(NULL, "Tuning_Persistence"), (D3DXVECTOR4*)(&Parameters::Tuning_Persistence));
			hRes = gCompEffect->SetFloat(gCompEffect->GetParameterByName(NULL, "Tuning_Bleed"), Parameters::Tuning_Bleed);
			hRes = gCompEffect->SetFloat(gCompEffect->GetParameterByName(NULL, "Tuning_Artifacts"), Parameters::Tuning_Artifacts);

			hRes = gCompEffect->SetTexture(gCompEffect->GetParameterByName(NULL, "curFrameMap"), gRTT_Clean);
			hRes = gCompEffect->SetTexture(gCompEffect->GetParameterByName(NULL, "prevFrameMap"), gCurrentSource);

			hRes = gCompEffect->SetTexture(gCompEffect->GetParameterByName(NULL, "NTSCArtifactTex"), gArtifactsTexture);

			hRes = gCompEffect->SetFloat(gCompEffect->GetParameterByName(NULL, "NTSCLerp"), gEvenFrame ? 0.0f : 1.0f);

			hRes = gCompEffect->SetTechnique(gCompEffect->GetTechniqueByName("Composite"));

			UINT NumPasses = 0;
			hRes = gCompEffect->Begin(&NumPasses, 0);

			for (UINT Pass = 0; Pass < NumPasses; ++Pass)
			{
				hRes = gCompEffect->BeginPass(Pass);

				hRes = gDevice->SetVertexDeclaration(gSrcQuad->pVD);

				hRes = gDevice->SetStreamSource(0, gSrcQuad->pVB_Position, 0, 3 * sizeof(float));
				hRes = gDevice->SetStreamSource(1, gSrcQuad->pVB_TexCoord, 0, 2 * sizeof(float));

				hRes = gDevice->SetIndices(gSrcQuad->pIB);

				hRes = gDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 3, 0, 1);

				hRes = gCompEffect->EndPass();
			}

			hRes = gCompEffect->End();
		}

		// Step 2A. Draw the screen mesh.
		{
			hRes = gRTT_Full->GetSurfaceLevel(0, &pSurf);
			hRes = gDevice->SetRenderTarget(0, pSurf);
			pSurf->Release();

			hRes = gDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255,0,0,0), 1.0f, 0);

			hRes = gScreenEffect->SetFloat(gScreenEffect->GetParameterByName(NULL, "Tuning_Overscan"), 1.0f / Parameters::Tuning_Overscan);
			hRes = gScreenEffect->SetFloat(gScreenEffect->GetParameterByName(NULL, "Tuning_Dimming"), Parameters::Tuning_Dimming);
			hRes = gScreenEffect->SetFloat(gScreenEffect->GetParameterByName(NULL, "Tuning_Satur"), Parameters::Tuning_Satur);
			hRes = gScreenEffect->SetFloat(gScreenEffect->GetParameterByName(NULL, "Tuning_ReflScalar"), Parameters::Tuning_ReflScalar);
			hRes = gScreenEffect->SetFloat(gScreenEffect->GetParameterByName(NULL, "Tuning_Barrel"), Parameters::Tuning_Barrel);
			hRes = gScreenEffect->SetFloat(gScreenEffect->GetParameterByName(NULL, "Tuning_Mask_Brightness"), Parameters::Tuning_Mask_Brightness);
			hRes = gScreenEffect->SetFloat(gScreenEffect->GetParameterByName(NULL, "Tuning_Mask_Opacity"), Parameters::Tuning_Mask_Opacity);

			hRes = gScreenEffect->SetFloat(gScreenEffect->GetParameterByName(NULL, "Tuning_Diff_Brightness"), Parameters::Tuning_Diff_Brightness);
			hRes = gScreenEffect->SetFloat(gScreenEffect->GetParameterByName(NULL, "Tuning_Spec_Brightness"), Parameters::Tuning_Spec_Brightness);
			hRes = gScreenEffect->SetFloat(gScreenEffect->GetParameterByName(NULL, "Tuning_Spec_Power"), Parameters::Tuning_Spec_Power);
			hRes = gScreenEffect->SetFloat(gScreenEffect->GetParameterByName(NULL, "Tuning_Fres_Brightness"), Parameters::Tuning_Fres_Brightness);
			hRes = gScreenEffect->SetVector(gScreenEffect->GetParameterByName(NULL, "Tuning_LightPos"), (D3DXVECTOR4*)(&Parameters::Tuning_LightPos));

			hRes = gScreenEffect->SetMatrix(gScreenEffect->GetParameterByName(NULL, "wvpMat"), &WVPMat);
			hRes = gScreenEffect->SetMatrix(gScreenEffect->GetParameterByName(NULL, "worldMat"), &WorldMat);
			hRes = gScreenEffect->SetVector(gScreenEffect->GetParameterByName(NULL, "camPos"), &CamPos4);

			D3DXVECTOR4 UVScalar(1.0f, 0.0f, 0.0f, 0.0f);
			D3DXVECTOR4 UVOffset(0.0f, 0.0f, 0.0f, 0.0f);
			D3DXVECTOR4 CRTMask_Scale((float)Parameters::Init_SrcWidth * 0.5f, (float)Parameters::Init_SrcHeight, 0.0f, 0.0f);
			D3DXVECTOR4 CRTMask_Offset(0.0f, 0.0f, 0.0f, 0.0f);

			float Ratio_Viewport = 4.0f / 3.0f;
			float Ratio_ScreenBuffer = (float)Parameters::Init_SrcWidth / (float)Parameters::Init_SrcHeight;
			float Ratio_DesiredPixel = Parameters::Tuning_PixelRatio;

			UVScalar.y = (Ratio_ScreenBuffer / Ratio_Viewport) * Ratio_DesiredPixel;
			UVOffset.y = (1.0f - (((Ratio_ScreenBuffer / Ratio_Viewport) * Ratio_DesiredPixel))) * 0.5f;

			hRes = gScreenEffect->SetVector(gScreenEffect->GetParameterByName(NULL, "UVScalar"), &UVScalar);
			hRes = gScreenEffect->SetVector(gScreenEffect->GetParameterByName(NULL, "UVOffset"), &UVOffset);
			hRes = gScreenEffect->SetVector(gScreenEffect->GetParameterByName(NULL, "CRTMask_Scale"), &CRTMask_Scale);
			hRes = gScreenEffect->SetVector(gScreenEffect->GetParameterByName(NULL, "CRTMask_Offset"), &CRTMask_Offset);

			hRes = gScreenEffect->SetTexture(gScreenEffect->GetParameterByName(NULL, "compFrameMap"), gCurrentTarget);
			hRes = gScreenEffect->SetTexture(gScreenEffect->GetParameterByName(NULL, "shadowMaskMap"), gMaskTexture);

			hRes = gScreenEffect->SetTechnique(gScreenEffect->GetTechniqueByName("gScreen"));

			UINT NumPasses = 0;
			hRes = gScreenEffect->Begin(&NumPasses, 0);

			for (UINT Pass = 0; Pass < NumPasses; ++Pass)
			{
				hRes = gScreenEffect->BeginPass(Pass);

				hRes = gDevice->SetVertexDeclaration(gScreen->pVD);

				for (UINT i = 0; i < gScreen->NumStreams; ++i)
				{
					hRes = gDevice->SetStreamSource(i, gScreen->Streams[i].pVB, 0, gScreen->Streams[i].Stride);
				}

				hRes = gDevice->SetIndices(gScreen->pIB);

				hRes = gDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, gScreen->NumVertices, 0, gScreen->NumIndices / 3);

				hRes = gScreenEffect->EndPass();
			}

			hRes = gScreenEffect->End();
		}

		// Step 2B. Draw the frame mesh.
		{
			hRes = gFrameEffect->SetFloat(gFrameEffect->GetParameterByName(NULL, "Tuning_Overscan"), Parameters::Tuning_Overscan);
			hRes = gFrameEffect->SetFloat(gFrameEffect->GetParameterByName(NULL, "Tuning_Dimming"), Parameters::Tuning_Dimming);
			hRes = gFrameEffect->SetFloat(gFrameEffect->GetParameterByName(NULL, "Tuning_Satur"), Parameters::Tuning_Satur);
			hRes = gFrameEffect->SetFloat(gFrameEffect->GetParameterByName(NULL, "Tuning_ReflScalar"), Parameters::Tuning_ReflScalar);
			hRes = gFrameEffect->SetFloat(gFrameEffect->GetParameterByName(NULL, "Tuning_Barrel"), Parameters::Tuning_Barrel);
			hRes = gFrameEffect->SetFloat(gFrameEffect->GetParameterByName(NULL, "Tuning_Mask_Brightness"), Parameters::Tuning_Mask_Brightness);
			hRes = gFrameEffect->SetFloat(gFrameEffect->GetParameterByName(NULL, "Tuning_Mask_Opacity"), Parameters::Tuning_Mask_Opacity);

			hRes = gFrameEffect->SetFloat(gFrameEffect->GetParameterByName(NULL, "Tuning_Diff_Brightness"), Parameters::Tuning_Diff_Brightness);
			hRes = gFrameEffect->SetFloat(gFrameEffect->GetParameterByName(NULL, "Tuning_Spec_Brightness"), Parameters::Tuning_Spec_Brightness);
			hRes = gFrameEffect->SetFloat(gFrameEffect->GetParameterByName(NULL, "Tuning_Spec_Power"), Parameters::Tuning_Spec_Power);
			hRes = gFrameEffect->SetFloat(gFrameEffect->GetParameterByName(NULL, "Tuning_Fres_Brightness"), Parameters::Tuning_Fres_Brightness);
			hRes = gFrameEffect->SetVector(gFrameEffect->GetParameterByName(NULL, "Tuning_LightPos"), (D3DXVECTOR4*)(&Parameters::Tuning_LightPos));

			hRes = gFrameEffect->SetVector(gFrameEffect->GetParameterByName(NULL, "Tuning_FrameColor"), (D3DXVECTOR4*)(&Parameters::Tuning_FrameColor));

			hRes = gFrameEffect->SetMatrix(gFrameEffect->GetParameterByName(NULL, "wvpMat"), &WVPMat);
			hRes = gFrameEffect->SetMatrix(gFrameEffect->GetParameterByName(NULL, "worldMat"), &WorldMat);
			hRes = gFrameEffect->SetVector(gFrameEffect->GetParameterByName(NULL, "camPos"), &CamPos4);

			D3DXVECTOR4 UVScalar(1.0f, 0.0f, 0.0f, 0.0f);
			D3DXVECTOR4 UVOffset(0.0f, 0.0f, 0.0f, 0.0f);
			D3DXVECTOR4 CRTMask_Scale((float)Parameters::Init_SrcWidth * 0.5f, (float)Parameters::Init_SrcHeight * 0.5f, 0.0f, 0.0f);
			D3DXVECTOR4 CRTMask_Offset(0.0f, 0.0f, 0.0f, 0.0f);

			float Ratio_Viewport = 4.0f / 3.0f;
			float Ratio_FrameBuffer = (float)Parameters::Init_SrcWidth / (float)Parameters::Init_SrcHeight;
			float Ratio_DesiredPixel = Parameters::Tuning_PixelRatio;

			UVScalar.y = (Ratio_FrameBuffer / Ratio_Viewport) * Ratio_DesiredPixel;
			UVOffset.y = (1.0f - (((Ratio_FrameBuffer / Ratio_Viewport) * Ratio_DesiredPixel))) * 0.5f;

			hRes = gFrameEffect->SetVector(gFrameEffect->GetParameterByName(NULL, "UVScalar"), &UVScalar);
			hRes = gFrameEffect->SetVector(gFrameEffect->GetParameterByName(NULL, "UVOffset"), &UVOffset);
			hRes = gFrameEffect->SetVector(gFrameEffect->GetParameterByName(NULL, "CRTMask_Scale"), &CRTMask_Scale);
			hRes = gFrameEffect->SetVector(gFrameEffect->GetParameterByName(NULL, "CRTMask_Offset"), &CRTMask_Offset);

			hRes = gFrameEffect->SetTexture(gFrameEffect->GetParameterByName(NULL, "compFrameMap"), gCurrentTarget);
			hRes = gFrameEffect->SetTexture(gFrameEffect->GetParameterByName(NULL, "shadowMaskMap"), gMaskTexture);

			hRes = gFrameEffect->SetTechnique(gFrameEffect->GetTechniqueByName("gFrame"));

			UINT NumPasses = 0;
			hRes = gFrameEffect->Begin(&NumPasses, 0);

			for (UINT Pass = 0; Pass < NumPasses; ++Pass)
			{
				hRes = gFrameEffect->BeginPass(Pass);

				hRes = gDevice->SetVertexDeclaration(gFrame->pVD);

				for (UINT i = 0; i < gFrame->NumStreams; ++i)
				{
					hRes = gDevice->SetStreamSource(i, gFrame->Streams[i].pVB, 0, gFrame->Streams[i].Stride);
				}

				hRes = gDevice->SetIndices(gFrame->pIB);

				hRes = gDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, gFrame->NumVertices, 0, gFrame->NumIndices / 3);

				hRes = gFrameEffect->EndPass();
			}

			hRes = gFrameEffect->End();
		}

		// Step 3. Downsample and blur the image of the screen and frame.
		{
			hRes = gRTT_Downsample->GetSurfaceLevel(0, &pSurf);
			hRes = gDevice->SetRenderTarget(0, pSurf);
			pSurf->Release();

			hRes = gDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255,0,0,0), 1.0f, 0);

			hRes = gPostEffect->SetTexture(gPostEffect->GetParameterByName(NULL, "PreBloomBuffer"), gRTT_Full);
			hRes = gPostEffect->SetTexture(gPostEffect->GetParameterByName(NULL, "DownsampleBuffer"), NULL);

			float InvAspectRatio = (float)Parameters::Init_DstHeight / (float)Parameters::Init_DstWidth;
			D3DXVECTOR4 BloomScaleVec4(InvAspectRatio * Parameters::Tuning_Bloom_Downsample_Spread, Parameters::Tuning_Bloom_Downsample_Spread, 0.0f, 0.0f);

			hRes = gPostEffect->SetVector(gPostEffect->GetParameterByName(NULL, "BloomScale"), &BloomScaleVec4);

			hRes = gPostEffect->SetTechnique(gPostEffect->GetTechniqueByName("PostProcessDownsample"));

			UINT NumPasses = 0;
			hRes = gPostEffect->Begin(&NumPasses, 0);

			for (UINT Pass = 0; Pass < NumPasses; ++Pass)
			{
				hRes = gPostEffect->BeginPass(Pass);

				hRes = gDevice->SetVertexDeclaration(gDownsampleQuad->pVD);

				hRes = gDevice->SetStreamSource(0, gDownsampleQuad->pVB_Position, 0, 3 * sizeof(float));
				hRes = gDevice->SetStreamSource(1, gDownsampleQuad->pVB_TexCoord, 0, 2 * sizeof(float));

				hRes = gDevice->SetIndices(gDownsampleQuad->pIB);

				hRes = gDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 3, 0, 1);

				hRes = gPostEffect->EndPass();
			}

			hRes = gPostEffect->End();
		}

		// Step 4. Upsample and blur the downsampled/blurred image.
		{
			hRes = gRTT_Upsample->GetSurfaceLevel(0, &pSurf);
			hRes = gDevice->SetRenderTarget(0, pSurf);
			pSurf->Release();

			hRes = gDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255,0,0,0), 1.0f, 0);

			hRes = gPostEffect->SetTexture(gPostEffect->GetParameterByName(NULL, "PreBloomBuffer"), NULL);
			hRes = gPostEffect->SetTexture(gPostEffect->GetParameterByName(NULL, "DownsampleBuffer"), gRTT_Downsample);

			float InvAspectRatio = (float)Parameters::Init_DstHeight / (float)Parameters::Init_DstWidth;
			D3DXVECTOR4 BloomScaleVec4(InvAspectRatio * Parameters::Tuning_Bloom_Upsample_Spread, Parameters::Tuning_Bloom_Upsample_Spread, 0.0f, 0.0f);

			hRes = gPostEffect->SetVector(gPostEffect->GetParameterByName(NULL, "BloomScale"), &BloomScaleVec4);

			hRes = gPostEffect->SetTechnique(gPostEffect->GetTechniqueByName("PostProcessUpsample"));

			UINT NumPasses = 0;
			hRes = gPostEffect->Begin(&NumPasses, 0);

			for (UINT Pass = 0; Pass < NumPasses; ++Pass)
			{
				hRes = gPostEffect->BeginPass(Pass);

				hRes = gDevice->SetVertexDeclaration(gDstQuad->pVD);

				hRes = gDevice->SetStreamSource(0, gDstQuad->pVB_Position, 0, 3 * sizeof(float));
				hRes = gDevice->SetStreamSource(1, gDstQuad->pVB_TexCoord, 0, 2 * sizeof(float));

				hRes = gDevice->SetIndices(gDstQuad->pIB);

				hRes = gDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 3, 0, 1);

				hRes = gPostEffect->EndPass();
			}

			hRes = gPostEffect->End();
		}

		// Step 5. Blend the original and blurred images to produce bloom.
		{
			hRes = gDevice->SetRenderTarget(0, gBackbuffer);

			hRes = gDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_ARGB(255,0,0,0), 1.0f, 0);

			hRes = gPresentEffect->SetTexture(gPresentEffect->GetParameterByName(NULL, "PreBloomBuffer"), gRTT_Full);
			hRes = gPresentEffect->SetTexture(gPresentEffect->GetParameterByName(NULL, "UpsampledBuffer"), gRTT_Upsample);

			hRes = gPresentEffect->SetFloat(gPresentEffect->GetParameterByName(NULL, "BloomScalar"), Parameters::Tuning_Bloom_Intensity);
			hRes = gPresentEffect->SetFloat(gPresentEffect->GetParameterByName(NULL, "BloomPower"), Parameters::Tuning_Bloom_Power);

			hRes = gPresentEffect->SetTechnique(gPresentEffect->GetTechniqueByName("PresentPassTechnique"));

			UINT NumPasses = 0;
			hRes = gPresentEffect->Begin(&NumPasses, 0);

			for (UINT Pass = 0; Pass < NumPasses; ++Pass)
			{
				hRes = gPresentEffect->BeginPass(Pass);

				hRes = gDevice->SetVertexDeclaration(gDstQuad->pVD);

				hRes = gDevice->SetStreamSource(0, gDstQuad->pVB_Position, 0, 3 * sizeof(float));
				hRes = gDevice->SetStreamSource(1, gDstQuad->pVB_TexCoord, 0, 2 * sizeof(float));

				hRes = gDevice->SetIndices(gDstQuad->pIB);

				hRes = gDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 3, 0, 1);

				hRes = gPresentEffect->EndPass();
			}

			hRes = gPresentEffect->End();
		}

		// And we're done!
		hRes = gDevice->EndScene();

		hRes = gDevice->Present(NULL, NULL, NULL, NULL);
	}

	gEvenFrame = !gEvenFrame;
}
