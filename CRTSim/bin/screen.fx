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

struct vsScreenOut
{
	half4 pos		: POSITION;
	half4 color		: COLOR;
	half2 uv		: TEXCOORD0;
	half3 norm		: TEXCOORD1;
	half3 camDir	: TEXCOORD2;
	half3 lightDir	: TEXCOORD3;
};



vsScreenOut screenVertexShader(		half3 pos : POSITION,
									half3 norm : NORMAL,
									half4 color	: COLOR,
									half2 uv : TEXCOORD0)
{
	vsScreenOut output;
	output.pos = mul(half4(pos, 1), wvpMat);
	
	half3 worldPos = mul(half4(pos, 1), worldMat).xyz;
	
	output.color = color;
	output.uv = uv;
	
	output.norm = norm;
	
	half3x3 wMat3 = half3x3(worldMat[0].xyz,worldMat[1].xyz,worldMat[2].xyz);
	half3x3 invWorldRot = transpose(wMat3);
	
	// Don't normalize this pre-pixel shader
	output.camDir = mul(camPos - worldPos, invWorldRot);
	output.lightDir = mul(Tuning_LightPos - worldPos, invWorldRot);
	
	return output;
}

half4 screenPixelShader(vsScreenOut frag) : COLOR
{	
	half3 norm = normalize(frag.norm);
	
	half3 camDir = normalize(frag.camDir);
	half3 lightDir = normalize(frag.lightDir);
	
	half3 refl = reflect(camDir, frag.norm);
		
	half diffuse = saturate(dot(norm, lightDir));
	half4 colordiff = half4(0.175, 0.15, 0.2, 1) * diffuse * Tuning_Diff_Brightness;
	
	half3 halfVec = normalize(lightDir + camDir);
	half spec = saturate(dot(norm, halfVec));
	spec = pow(spec, Tuning_Spec_Power);
	half4 colorspec = half4(0.25, 0.25, 0.25, 1) * spec * Tuning_Spec_Brightness;
	
	half fres = 1.0f - dot(camDir, norm);
	fres = (fres*fres) * Tuning_Fres_Brightness;
	half4 colorfres = half4(0.45, 0.4, 0.5, 1) * fres;
	
	half4 emissive = SampleCRT(frag.uv);
	
	half4 nearfinal = colorfres + colordiff + colorspec + emissive;
	
	return (nearfinal * lerp(half4(1,1,1,1), frag.color, Tuning_Dimming));
}

technique Screen
{
	pass Pass1
	{
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		FillMode = solid;
		CullMode = cw;
		ZFunc = Less;
		ZWriteEnable = true;
		VertexShader = compile vs_2_0 screenVertexShader();
		PixelShader = compile ps_2_0 screenPixelShader();
	}
}
