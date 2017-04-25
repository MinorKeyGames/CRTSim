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

// Blends the original full-resolution scene with the blurred output of post.fx to create bloom.

uniform texture PreBloomBuffer;
uniform texture UpsampledBuffer;

uniform float BloomScalar;
uniform float BloomPower;

uniform sampler2D PreBloomBufferSampler = sampler_state
{
	Texture = (PreBloomBuffer);
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = None;
	
	AddressU = CLAMP;
	AddressV = CLAMP;
};

uniform sampler2D UpsampledBufferSampler = sampler_state
{
	Texture = (UpsampledBuffer);
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = None;
	
	AddressU = CLAMP;
	AddressV = CLAMP;
};

// Apply power to brightness while preserving color
// TODO: Clamp ActLuma to very small number to prevent (zero) division by zero when a component is zero?
half4 ColorPow(half4 InColor, half InPower)
{
	// This method preserves color better.
	half4 RefLuma = half4(0.299, 0.587, 0.114, 0.0);
	half ActLuma = dot(InColor, RefLuma);
	half4 ActColor = InColor / ActLuma;
	half PowLuma = pow(ActLuma, InPower);
	half4 PowColor = ActColor * PowLuma;
	return PowColor;
}

struct vsPresentPassOut
{
	half4 pos		: POSITION;
	half2 uv		: TEXCOORD0;
};

vsPresentPassOut PresentPassVertexShader(half3 pos : POSITION,
										half2 uv : TEXCOORD0)
{
	vsPresentPassOut output;
	
	output.pos = half4(pos, 1);
	output.uv = uv;
	
	return output;
}

half4 PresentPassPixelShader(vsPresentPassOut frag) : COLOR
{
	half4 PreBloom = tex2D(PreBloomBufferSampler, frag.uv);
	half4 Blurred = tex2D(UpsampledBufferSampler, frag.uv);
	
	return PreBloom + (ColorPow(Blurred, BloomPower) * BloomScalar);
}

technique PresentPassTechnique
{
	pass Pass1
	{
		StencilEnable = false;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		FillMode = solid;
		CullMode = cw;
		ZFunc = Always;
		ZWriteEnable = false;
		DepthBias = 0.0f;
		VertexShader = compile vs_2_0 PresentPassVertexShader();
		PixelShader = compile ps_2_0 PresentPassPixelShader();
	}
}
