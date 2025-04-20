// Copyright (c) 2025 Sora Mas 
// All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "EnchancedNotifications.generated.h"

/**
 * 
 */
UCLASS()
class ENCHANCEDEDITORLOGGING_API UEnchancedNotifications : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Editor Notifications")
	static void LaunchNotification(const FString& Title, const float& Duration = 10.f);

	UFUNCTION(BlueprintCallable, Category = "Editor Notifcations")
	static void OpenCBDirNotification(const FString& Title, const FString& Dir);

};
