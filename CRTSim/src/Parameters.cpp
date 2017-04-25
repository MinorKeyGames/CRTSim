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

#include "Parameters.h"

int Parameters::Init_SrcWidth = 256;
int Parameters::Init_SrcHeight = 224;
int Parameters::Init_DstWidth = 1600;
int Parameters::Init_DstHeight = 900;

float Parameters::Tuning_Sharp = 0.8f;
float Parameters::Tuning_Persistence[4] = {0.7f, 0.525f, 0.42f, 0.0f};
float Parameters::Tuning_Bleed = 0.5f;
float Parameters::Tuning_Artifacts = 0.5f;

float Parameters::Tuning_PixelRatio = (8.0f / 7.0f);
float Parameters::Tuning_Overscan = 1.0f;
float Parameters::Tuning_Dimming = 0.5f;
float Parameters::Tuning_Satur = 1.35f;
float Parameters::Tuning_ReflScalar = 0.3f;
float Parameters::Tuning_Barrel = -0.115f;
float Parameters::Tuning_Mask_Brightness = 0.45f;
float Parameters::Tuning_Mask_Opacity = 1.0f;

float Parameters::Tuning_Diff_Brightness = 0.5f;
float Parameters::Tuning_Spec_Brightness = 0.35f;
float Parameters::Tuning_Spec_Power = 50.0f;
float Parameters::Tuning_Fres_Brightness = 1.0f;
float Parameters::Tuning_LightPos[4] = {-10.0f, -5.0f, 10.0f, 0.0f};
float Parameters::Tuning_FrameColor[4] = {0.06f, 0.06f, 0.06f, 1.0f};

float Parameters::Tuning_Bloom_Downsample_Spread = 0.025f;
float Parameters::Tuning_Bloom_Upsample_Spread = 0.025f;
float Parameters::Tuning_Bloom_Intensity = 0.25f;
float Parameters::Tuning_Bloom_Power = 2.0f;
