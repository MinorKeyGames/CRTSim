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

// Multi-purpose code used for both downsampling and upsampling the full-resolution output image.

uniform texture PreBloomBuffer;			// The original full-resolution image before bloom or any other effects have been added.
uniform texture DownsampleBuffer;		// The downsampled and blurred image.

// The distance the bloom blur spreads.
// Includes aspect ratio compensation.
uniform float2 BloomScale;

const float2 Poisson0 = float2(0.000000, 0.000000);
const float2 Poisson1 = float2(0.000000, 1.000000);
const float2 Poisson2 = float2(0.000000, -1.000000);
const float2 Poisson3 = float2(-0.866025, 0.500000);
const float2 Poisson4 = float2(-0.866025, -0.500000);
const float2 Poisson5 = float2(0.866025, 0.500000);
const float2 Poisson6 = float2(0.866025, -0.500000);

const float InvNumSamples = 1.0f / 7.0f;

uniform sampler2D PreBloomBufferSampler = sampler_state
{
	Texture = (PreBloomBuffer);
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

uniform sampler2D DownsampleBufferSampler = sampler_state
{
	Texture = (DownsampleBuffer);
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU = CLAMP;
	AddressV = CLAMP;
};



struct vsPostProcessOut
{
	half4 pos		: POSITION;
	half2 uv		: TEXCOORD0;
};

vsPostProcessOut postProcessVertexShader(half3 pos : POSITION,
										half2 uv : TEXCOORD0)
{
	vsPostProcessOut output;
	
	output.pos = half4(pos, 1);
	output.uv = uv;
	
	return output;
}

half4 postProcessDownsamplePixelShader(vsPostProcessOut frag) : COLOR
{
	half4 bloom = half4(0,0,0,0);
	bloom += tex2D(PreBloomBufferSampler, frag.uv + (Poisson0 * BloomScale));
	bloom += tex2D(PreBloomBufferSampler, frag.uv + (Poisson1 * BloomScale));
	bloom += tex2D(PreBloomBufferSampler, frag.uv + (Poisson2 * BloomScale));
	bloom += tex2D(PreBloomBufferSampler, frag.uv + (Poisson3 * BloomScale));
	bloom += tex2D(PreBloomBufferSampler, frag.uv + (Poisson4 * BloomScale));
	bloom += tex2D(PreBloomBufferSampler, frag.uv + (Poisson5 * BloomScale));
	bloom += tex2D(PreBloomBufferSampler, frag.uv + (Poisson6 * BloomScale));
	bloom *= InvNumSamples;
	return bloom;
}

half4 postProcessUpsamplePixelShader(vsPostProcessOut frag) : COLOR
{
	// Swap X and Y for this one to reduce artifacts in sampling.
	
	half4 bloom = half4(0,0,0,0);
	bloom += tex2D(DownsampleBufferSampler, frag.uv + (Poisson0.yx * BloomScale));
	bloom += tex2D(DownsampleBufferSampler, frag.uv + (Poisson1.yx * BloomScale));
	bloom += tex2D(DownsampleBufferSampler, frag.uv + (Poisson2.yx * BloomScale));
	bloom += tex2D(DownsampleBufferSampler, frag.uv + (Poisson3.yx * BloomScale));
	bloom += tex2D(DownsampleBufferSampler, frag.uv + (Poisson4.yx * BloomScale));
	bloom += tex2D(DownsampleBufferSampler, frag.uv + (Poisson5.yx * BloomScale));
	bloom += tex2D(DownsampleBufferSampler, frag.uv + (Poisson6.yx * BloomScale));
	bloom *= InvNumSamples;
	return bloom;
}

// Techniques have to go here because my Python script will try to compile anything with a technique in it.
technique PostProcessDownsample
{
	pass Pass1
	{
		StencilEnable = false;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		FillMode = solid;
		CullMode = cw;
		ZFunc = Less;
		ZWriteEnable = false;
		DepthBias = 0.0f;
		VertexShader = compile vs_2_0 postProcessVertexShader();
		PixelShader = compile ps_2_0 postProcessDownsamplePixelShader();
	}
}

// This was applied additively back onto the color buffer in other titles.
// Now I'm using it to upsample the downsampled buffer to a full size blur buffer.
// From there, it can be used for whatever. I guess. I dunno.
technique PostProcessUpsample
{
	pass Pass1
	{
		StencilEnable = false;
		AlphaBlendEnable = false;//true;
		AlphaTestEnable = false;
		//SrcBlend = One;
		//DestBlend = One;
		FillMode = solid;
		CullMode = cw;
		ZFunc = Less;
		ZWriteEnable = false;
		DepthBias = 0.0f;
		VertexShader = compile vs_2_0 postProcessVertexShader();
		PixelShader = compile ps_2_0 postProcessUpsamplePixelShader();
	}
}
