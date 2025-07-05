// Copyright (c) 2025 Sora Mas
// All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "MaskEnums.h"
#include "ImageCore.h"
#include "MaskToolsConfig.generated.h"


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
	EMaskCreationMethod MixerCreationMethod;

	/*
	Sample
	*/
	UPROPERTY(EditAnywhere, config, Category = "Texture Mixer")
	EResizeMethod MixerResizeMethod;
	
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
	Sample
	*/
	UPROPERTY(EditAnywhere, config, Category = "Texture Splitter")
	EMaskCreationMethod SplitterCreationMethod;

	/*
	Sample
	*/
	UPROPERTY(EditAnywhere, config, Category = "Texture Splitter")
	EResizeMethod SplitterResizeMethod;

	/*
	Defines if Split Channels will attempt to search for completely black channels in splitted textures
	and will avoid try to export them
	*/
	UPROPERTY(EditAnywhere, config, Category = "Texture Splitter")
	bool bDiscardEmptyChannels;
};
