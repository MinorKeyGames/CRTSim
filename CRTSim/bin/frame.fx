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

#include "crtbase.fx"

uniform float4 Tuning_FrameColor;

struct vsFrameOut
{
	half4 pos : POSITION;
	half4 color : COLOR;
	half2 uv : TEXCOORD0;
	half blend : TEXCOORD1;
	half3 norm : TEXCOORD2;
	half3 camDir : TEXCOORD3;
	half3 lightDir : TEXCOORD4;
};

vsFrameOut frameVertexShader(
	half3 pos : POSITION,
	half3 norm : NORMAL,
	half4 color : COLOR,
	half2 uv : TEXCOORD0,
	half blend : TEXCOORD1)
{
	vsFrameOut output;
	output.pos = mul(half4(pos, 1), wvpMat);	
	output.norm = norm;			// These are in world space so okay yeah.
	output.color = color;
	output.uv = uv;
	output.blend = blend;
	
	// This is really unnecessary since it SHOULD always be identity, but whatever...
	half3 worldPos = mul(half4(pos, 1), worldMat).xyz;
	
	// Don't normalize this pre-pixel shader
	output.camDir = camPos - worldPos;
	output.lightDir = Tuning_LightPos - worldPos;
	
	return output;
}

half4 framePixelShader(vsFrameOut frag) : COLOR
{
	half3 norm = normalize(frag.norm);
	
	half3 camDir = normalize(frag.camDir);
	half3 lightDir = normalize(frag.lightDir);
	
	half diffuse = saturate(dot(norm, lightDir));
	
	half3 worldUp = half3(0,0,1);
	half hemi = dot(norm, worldUp) * 0.5f + 0.5f;
	hemi = hemi * 0.4f + 0.3f;			// [0.3,0.7] range
	
	half4 colordiff = Tuning_FrameColor * (diffuse + hemi) * Tuning_Diff_Brightness;
	
	half3 halfVec = normalize(lightDir + camDir);
	half spec = saturate(dot(norm, halfVec));
	spec = pow(spec, Tuning_Spec_Power);
	half4 colorspec = half4(0.25, 0.25, 0.25, 1) * spec * Tuning_Spec_Brightness;
	
	half4 emissive = SampleCRT(frag.uv);
	
	colorspec += (emissive * frag.blend * Tuning_ReflScalar);
	
	half fres = 1.0f - dot(camDir, norm);
	fres = (fres*fres) * Tuning_Fres_Brightness;
	half4 colorfres = half4(0.15, 0.15, 0.15, 1) * fres;
	
	half4 nearfinal = (colorfres + colordiff + colorspec);
	
	return (nearfinal * lerp(half4(1,1,1,1), frag.color, Tuning_Dimming));
}

technique Frame
{
	pass Pass1
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		FillMode = solid;
		CullMode = cw;
		ZFunc = Less;
		ZWriteEnable = true;
		VertexShader = compile vs_2_0 frameVertexShader();
		PixelShader = compile ps_2_0 framePixelShader();
	}
}
