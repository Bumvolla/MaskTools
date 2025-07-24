// Copyright (c) 2025 Sora Mas
// All rights reserved. 

#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EMaskCreationMethod : uint8
{
	PixelData = 0 UMETA(DisplayName = "Pixel Data Copy"),
	Material = 1 UMETA(DisplayName = "Material Based")
};

UENUM()
enum class EMaskResolutions : uint8
{
	ThirtyTwo UMETA(DisplayName = "32"),
	SixtyFour UMETA(DisplayName = "64"),
	OneHundredTwentyEight UMETA(DisplayName = "128"),
	TwoHundredFiftySix UMETA(DisplayName = "256"),
	FiveHundredTwelve UMETA(DisplayName = "512"),
	OneThousandTwentyFour UMETA(DisplayName = "1024"),
	TwoThousandFortyEight UMETA(DisplayName = "2048"),
	FourThousandNinetySix UMETA(DisplayName = "4096"),
	EightThousandOneHundredNinetyTwo UMETA(DisplayName = "8192")
};

UENUM()
enum class EResizeMethod : uint32
{
	Default = 0, // uses a good default filter; = AdaptiveSharp
	PointSample,
	Box,
	Triangle, 
	Bilinear, // same as triangle
	CubicGaussian, // smooth Mitchell B=1,C=0, B-spline, Gaussian-like
	CubicSharp, // sharp interpolating cubic, Catmull-ROM (has negative lobes)
	CubicMitchell, // compromise between sharp and smooth cubic, Mitchell-Netrevalli filter with B=1/3, C=1/3 (has negative lobes)
	AdaptiveSharp,  // sharper adaptive filter; uses CubicSharp for upsample and CubicMitchell for downsample, nop for same size
	AdaptiveSmooth,  // smoother adaptive filter; uses CubicMitchell for upsample and CubicGaussian for downsample, nop for same size

	WithoutFlagsMask = 63,
	Flag_WrapX = 64,  // default edge mode is clamp; set these to wrap instead
	Flag_WrapY = 128
};
