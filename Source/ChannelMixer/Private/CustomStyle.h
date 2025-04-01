// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Styling/SlateStyle.h"
#include "CustomStyle.generated.h"

UCLASS()
class UStyleFactory : public UObject
{
	GENERATED_BODY()

public:
	static UStyle* CreateStyle(FName StyleSetName, FString IconName, FName PropertyName);
};

UCLASS(Meta = (ModuleName = "ChannelMixer"))
class UStyle : public UObject
{

	GENERATED_BODY()

public:

	void Initialize(FName In_StyleSetName, FString In_IconName, FName In_PropertyName);

	void InitializeIcons();
	void ShutDown();
	FName PropertyName;

private:

	FName StyleSetName;
	FString IconName;
	FName ModuleName;

	TSharedRef<FSlateStyleSet> CreateSlateStyleSet();
	TSharedPtr<FSlateStyleSet> CreatedSlateStyleSet;

public:
	FName GetStyleSetName() { return StyleSetName; }
};