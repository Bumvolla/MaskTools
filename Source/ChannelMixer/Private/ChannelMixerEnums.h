// Copyright (c) 2025 Sora Mas
// All rights reserved.

#pragma once

UENUM()
enum class EChannelMixerChannel : uint8
{
	Red UMETA(DisplayName="Red"),
	Green UMETA(DisplayName="Green"),
	Blue UMETA(DisplayName="Blue"),
	Alpha UMETA(DisplayName="Alpha"),
	Result UMETA(DisplayName="Result")
};

UENUM()
enum class EChannelMixerCBAction : uint8
{
	Open UMETA(DisplayName="Open"),
	Close UMETA(DisplayName="Close"),
	Default UMETA(DisplayName="Default")
};

UENUM()
enum class EChannelMixerTextureChannel : uint8
{
	Red UMETA(DisplayName="Red"),
	Green UMETA(DisplayName="Green"),
	Blue UMETA(DisplayName="Blue"),
	Alpha UMETA(DisplayName="Alpha")
};