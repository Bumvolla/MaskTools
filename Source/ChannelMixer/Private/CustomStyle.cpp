#include "CustomStyle.h"
#include "Modules/ModuleManager.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

#include "Misc/Paths.h"

UStyle* UStyleFactory::CreateStyle(FName StyleSetName, FString IconName, FName PropertyName)
{
    UStyle* GeneratedStyle = NewObject<UStyle>();
    GeneratedStyle->Initialize(StyleSetName, IconName, PropertyName);
    return GeneratedStyle;
}

void UStyle::Initialize(FName In_StyleSetName, FString In_IconName, FName In_PropertyName)
{
    StyleSetName = In_StyleSetName;
    IconName = In_IconName;
    PropertyName = In_PropertyName;
    CreatedSlateStyleSet = nullptr;

    InitializeIcons();
}

void UStyle::InitializeIcons()
{
    if (!CreatedSlateStyleSet.IsValid())
    {
        CreatedSlateStyleSet = CreateSlateStyleSet();
        FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet);

    }
}

TSharedRef<FSlateStyleSet> UStyle::CreateSlateStyleSet()
{
    TSharedRef<FSlateStyleSet> CustomStyleSet = MakeShareable(new FSlateStyleSet(StyleSetName));

    const FString ModuleString = UStyle::GetClass()->GetMetaData(TEXT("ModuleName"));
    const TSharedPtr<IPlugin> ContainerPlugin = IPluginManager::Get().GetModuleOwnerPlugin(FName(*ModuleString));

    // Set the content root for the style set
    const FString PluginResourcesDirectory = FString::Printf(TEXT("%sResources"), *ContainerPlugin->GetBaseDir());
    if (!FPaths::DirectoryExists(PluginResourcesDirectory)) return CustomStyleSet;
    CustomStyleSet->SetContentRoot(PluginResourcesDirectory);

    // Define the icon size
    const FVector2D Icon16x16(16.f, 16.f);

    // Add the icon to the style set
    const FString IconPath = FString::Printf(TEXT("%s/%s"), *PluginResourcesDirectory, *IconName); 
    CustomStyleSet->Set(PropertyName, new FSlateImageBrush(IconName, Icon16x16));

    return CustomStyleSet;
}

void UStyle::ShutDown()
{
    if (CreatedSlateStyleSet.IsValid())
    {
        FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
        CreatedSlateStyleSet.Reset();
    }
}