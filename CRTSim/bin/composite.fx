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

// This is the second step of the CRT simulation process,
// after the ntsc.fx shader has transformed the RGB values with a lookup table.
// This is where we apply effects "inside the screen," including spatial and temporal bleeding,
// an unsharp mask to simulate overshoot/undershoot, NTSC artifacts, and so on.

uniform float2 RcpScrWidth;
uniform float2 RcpScrHeight;

uniform float Tuning_Sharp;				// typically [0,1], defines the weighting of the sharpness taps
uniform float4 Tuning_Persistence;		// typically [0,1] per channel, defines the total blending of previous frame values
uniform float Tuning_Bleed;				// typically [0,1], defines the blending of L/R values with center value from prevous frame
uniform float Tuning_Artifacts;			// typically [0,1], defines the weighting of NTSC scanline artifacts (not physically accurate by any means)

// These are render target textures at the game scene resolution (256x224)
// representing the current scene prior to any compositing and the previous frame after compositing.
// Once we process this frame, we'll swap these and the final image will become our next frame's "previous."
uniform texture curFrameMap;
uniform texture prevFrameMap;

uniform texture NTSCArtifactTex;		// This is a texture map containing diagonal stripes of red, green, and blue, used to simulate chroma overlap between pixels as described in the slides.
uniform float NTSCLerp;					// Defines an interpolation between the two NTSC filter states. Typically would be 0 or 1 for vsynced 60 fps gameplay or 0.5 for unsynced, but can be whatever.

uniform sampler2D curFrameSampler = sampler_state
{
	Texture = (curFrameMap);
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = None;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

uniform sampler2D prevFrameSampler = sampler_state
{
	Texture = (prevFrameMap);
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = None;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

uniform sampler2D NTSCArtifactSampler = sampler_state
{
	Texture = (NTSCArtifactTex);
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = None;
	AddressU = WRAP;
	AddressV = WRAP;
};

// Weight for applying an unsharp mask at a distance of 1, 2, or 3 pixels from changes in luma.
// The sign of each weight changes in order to alternately simulate overshooting and undershooting.
const float SharpWeight[3] =
{
	1.0, -0.3162277, 0.1
};

struct vsCompositeOut
{
	half4 pos		: POSITION;
	half2 uv		: TEXCOORD0;
};



// Passthrough vertex shader. Nothing interesting here.
vsCompositeOut compositeVertexShader(	half3 pos : POSITION,
										half2 uv : TEXCOORD0)
{
	vsCompositeOut output;
	output.pos = half4(pos, 1);
	
	output.uv = uv;
	
	return output;
}

// Calculate luma for an RGB value.
half Brightness(half4 InVal)
{
	return dot(InVal, half4(0.299, 0.587, 0.114, 0.0));
}

half4 compositePixelShader(vsCompositeOut frag) : COLOR
{
	half4 NTSCArtifact1 = tex2D(NTSCArtifactSampler, frag.uv);
	half4 NTSCArtifact2 = tex2D(NTSCArtifactSampler, frag.uv + RcpScrHeight);
	half4 NTSCArtifact = lerp(NTSCArtifact1, NTSCArtifact2, NTSCLerp);
	
	half2 LeftUV = frag.uv - RcpScrWidth;
	half2 RightUV = frag.uv + RcpScrWidth;
	
	half4 Cur_Left = tex2D(curFrameSampler, LeftUV);
	half4 Cur_Local = tex2D(curFrameSampler, frag.uv);
	half4 Cur_Right = tex2D(curFrameSampler, RightUV);
	
	half4 TunedNTSC = NTSCArtifact * Tuning_Artifacts;
		
	// Note: The "persistence" and "bleed" parameters have some overlap, but they are not redundant.
	// "Persistence" affects bleeding AND trails. (Scales the sum of the previous value and its scaled neighbors.)
	// "Bleed" only affects bleeding. (Scaling of neighboring previous values.)
	
	half4 Prev_Left = tex2D(prevFrameSampler, LeftUV);
	half4 Prev_Local = tex2D(prevFrameSampler, frag.uv);
	half4 Prev_Right = tex2D(prevFrameSampler, RightUV);
	
	// Apply NTSC artifacts based on differences in luma between local pixel and neighbors..
	Cur_Local =
		saturate(Cur_Local +
		(((Cur_Left - Cur_Local) + (Cur_Right - Cur_Local)) * TunedNTSC));
	
	half curBrt = Brightness(Cur_Local);
	half offset = 0;
	
	// Step left and right looking for changes in luma that would produce a ring or halo on this pixel due to undershooting/overshooting.
	// (Note: It would probably be more accurate to look at changes in luma between pixels at a distance of N and N+1,
	// as opposed to 0 and N as done here, but this works pretty well and is a little cheaper.)
	for (int i = 0; i < 3; ++i)
	{
		half2 StepSize = (half2(1.0/256.0,0) * (float(i + 1)));
		half4 neighborleft = tex2D(curFrameSampler, frag.uv - StepSize);
		half4 neighborright = tex2D(curFrameSampler, frag.uv + StepSize);
		
		half NBrtL = Brightness(neighborleft);
		half NBrtR = Brightness(neighborright);
		offset += ((((curBrt - NBrtL) + (curBrt - NBrtR))) * SharpWeight[i]);
	}
	
	// Apply the NTSC artifacts to the unsharp offset as well.
	Cur_Local = saturate(Cur_Local + (offset * Tuning_Sharp * lerp(half4(1,1,1,1), NTSCArtifact, Tuning_Artifacts)));
	
	// Take the max here because adding is overkill; bleeding should only brighten up dark areas, not blow out the whole screen.
	Cur_Local = saturate(max(Cur_Local, Tuning_Persistence * (1.0 / (1.0 + (2.0 * Tuning_Bleed))) * (Prev_Local + ((Prev_Left + Prev_Right) * Tuning_Bleed))));
	
	return Cur_Local;
}

technique Composite
{
	pass Pass1
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		FillMode = solid;
		CullMode = cw;
		ZFunc = Always;
		ZWriteEnable = false;
		VertexShader = compile vs_2_0 compositeVertexShader();
		PixelShader = compile ps_2_0 compositePixelShader();
	}
}
