// Copyright (c) 2025 Sora Mas 
// All rights reserved. 

#include "EnchancedNotifications.h"

#include <SlateOptMacros.h>
#include "Widgets/Input/SButton.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/SBoxPanel.h"

#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "EnchancedNotifications.h"

void UEnchancedNotifications::LaunchNotification(const FString& Title, const float& Duration)
{
    FNotificationInfo Info(FText::FromString(Title));
    Info.FadeOutDuration = Duration;

     FSlateNotificationManager::Get().AddNotification(Info);
}

void UEnchancedNotifications::OpenCBDirNotification(const FString& Title, const FString& Dir)
{
    TSharedPtr<SNotificationItem> NotificationPtr;

    FNotificationInfo Info(FText::FromString(Title));

    Info.ButtonDetails.Add(FNotificationButtonInfo(
        FText::FromString(TEXT("Open Folder")),
        FText::FromString(TEXT("")),
        FSimpleDelegate::CreateLambda([Dir]()
            {
                FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
                ContentBrowserModule.Get().FocusPrimaryContentBrowser(false);
                TArray<FString> Paths;
                Paths.Add(Dir);
                ContentBrowserModule.Get().SyncBrowserToFolders(Paths);
            }
        )
    ));

    Info.ExpireDuration = 12.f;
    Info.FadeOutDuration = 1.f;
    Info.bFireAndForget = true;

    Info.WidthOverride = 600.0f;
    Info.bUseLargeFont = false;
    Info.bUseThrobber = false;
    Info.bUseSuccessFailIcons = false;

    //@todo: Change the notification icon
    Info.Image = FCoreStyle::Get().GetBrush(TEXT("Icons.SuccessWithColor"));

    NotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);

    if (NotificationPtr.IsValid())
    {
        NotificationPtr->SetCompletionState(SNotificationItem::CS_Pending);
    }
}

