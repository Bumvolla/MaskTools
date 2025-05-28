// Copyright (c) 2025 Sora Mas \n All rights reserved. 

#pragma once

#include "CoreMinimal.h"

UENUM()
enum class EMaskCreationMethod : uint8
{
	Material UMETA(DisplayName = "Material Based"),
	PixelData UMETA(DisplayName = "Pixel Data Copy"),
	Shader UMETA(DisplayName = "ShaderBased")
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