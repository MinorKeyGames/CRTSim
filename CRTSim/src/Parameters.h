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

#pragma once

class Parameters
{
private:
	Parameters() {}

public:
	static int Init_SrcWidth;
	static int Init_SrcHeight;
	static int Init_DstWidth;
	static int Init_DstHeight;

	static float Tuning_Sharp;
	static float Tuning_Persistence[4];
	static float Tuning_Bleed;
	static float Tuning_Artifacts;

	static float Tuning_PixelRatio;
	static float Tuning_Overscan;
	static float Tuning_Dimming;
	static float Tuning_Satur;
	static float Tuning_ReflScalar;
	static float Tuning_Barrel;
	static float Tuning_Mask_Brightness;
	static float Tuning_Mask_Opacity;

	static float Tuning_Diff_Brightness;
	static float Tuning_Spec_Brightness;
	static float Tuning_Spec_Power;
	static float Tuning_Fres_Brightness;
	static float Tuning_LightPos[4];
	static float Tuning_FrameColor[4];

	static float Tuning_Bloom_Downsample_Spread;
	static float Tuning_Bloom_Upsample_Spread;
	static float Tuning_Bloom_Intensity;
	static float Tuning_Bloom_Power;
};
