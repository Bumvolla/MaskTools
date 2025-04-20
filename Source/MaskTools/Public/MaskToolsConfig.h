// Copyright (c) 2025 Sora Mas
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "MaskToolsConfig.generated.h"


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

/**
 * 
 */
UCLASS(config = Editor, defaultconfig)
class MASKTOOLS_API UMaskToolsConfig : public UObject
{

	GENERATED_BODY()

public:
	UMaskToolsConfig();

	/*
	Sample
	*/
	UPROPERTY(EditAnywhere, config, Category = "Texture Mixer")
	FString DefaultMaskPrefix;

	/*
	Sample
	*/
	UPROPERTY(EditAnywhere, config, Category = "Texture Mixer")
	FString DefaultMaskName;

	/*
	Sample
	*/
	UPROPERTY(EditAnywhere, config, Category = "Texture Mixer")
	FString DefaultMaskSuffix;

	/*
	Path where masks should be saved.
	Default is: /Game/GeneratedMasks
	*/
	UPROPERTY(EditAnywhere, config, Category = "Texture Mixer", meta = (RelativeToGameContentDir))
	FDirectoryPath DefaultMaskSavePath;

	/*
	Default resolution masks will be exported if not changed.
	Default is 512
	*/
	UPROPERTY(EditAnywhere, config, Category = "Texture Mixer")
	EMaskResolutions DefaultMaskResolution;

	/*
	Sample
	*/
	UPROPERTY(EditAnywhere, config, Category = "Texture Mixer")
	bool bDefaultAddPrefix;

	/*
	Sample
	*/
	UPROPERTY(EditAnywhere, config, Category = "Texture Mixer")
	bool bDefaultAddSuffix;

	/*
	Defines if Split Channels will attempt to search for completely black channels in splitted textures
	and will avoid try to export them
	*/
	UPROPERTY(EditAnywhere, config, Category = "Texture Splitter")
	bool bDiscardEmptyChannels;
};
