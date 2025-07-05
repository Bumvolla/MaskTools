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
