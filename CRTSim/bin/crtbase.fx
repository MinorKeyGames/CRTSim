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

// Common code shared by the screen and frame effects.
// Provides a function for sampling into the "in-CRT" composite image with "in-world" effects applied.

uniform float4x4 wvpMat;
uniform float4x4 worldMat;

uniform float3 camPos;

uniform float2 UVScalar;
uniform float2 UVOffset;

uniform float2 CRTMask_Scale;
uniform float2 CRTMask_Offset;

uniform float Tuning_Overscan;
uniform float Tuning_Dimming;
uniform float Tuning_Satur;
uniform float Tuning_ReflScalar;
uniform float Tuning_Barrel;
uniform float Tuning_Mask_Brightness;
uniform float Tuning_Mask_Opacity;
uniform float Tuning_Diff_Brightness;
uniform float Tuning_Spec_Brightness;
uniform float Tuning_Spec_Power;
uniform float Tuning_Fres_Brightness;
uniform float3 Tuning_LightPos;

uniform texture compFrameMap;
uniform texture shadowMaskMap;

uniform sampler2D compFrameSampler = sampler_state
{
	Texture = (compFrameMap);
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = 0xff000000;
};

uniform sampler2D shadowMaskSampler = sampler_state
{
	Texture = (shadowMaskMap);
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = WRAP;
	AddressV = WRAP;
};

half4 SampleCRT(half2 uv)
{
	half2 ScaledUV = uv;
	ScaledUV *= UVScalar;
	ScaledUV += UVOffset;
	
	half2 scanuv = ScaledUV * CRTMask_Scale;// + CRTMask_Offset;
	half3 scantex = tex2D(shadowMaskSampler, scanuv).rgb;
	
	scantex += Tuning_Mask_Brightness;			// adding looks better
	scantex = lerp(half3(1,1,1), scantex, Tuning_Mask_Opacity);

	// Apply overscan after scanline sampling is done.
	half2 overscanuv = (ScaledUV * Tuning_Overscan) - ((Tuning_Overscan - 1.0f) * 0.5f);
	
	// Curve UVs for composite texture inwards to garble things a bit.
	overscanuv = overscanuv - half2(0.5,0.5);
	half rsq = (overscanuv.x*overscanuv.x) + (overscanuv.y*overscanuv.y);
	overscanuv = overscanuv + (overscanuv * (Tuning_Barrel * rsq)) + half2(0.5,0.5);
		
	half3 comptex = tex2D(compFrameSampler, overscanuv).rgb;

	half4 emissive = half4(comptex * scantex, 1);
	half desat = dot(half4(0.299, 0.587, 0.114, 0.0), emissive);
	emissive = lerp(half4(desat,desat,desat,1), emissive, Tuning_Satur);
	
	return emissive;
}
