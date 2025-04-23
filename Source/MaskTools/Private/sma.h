// sma.h (Shared across all plugins)
#pragma once
#include "CoreMinimal.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonWriter.h"
#include "Serialization/JsonSerializer.h"

#ifndef ANALYTICS_PREFIX
#error "ANALYTICS_PREFIX must be defined before including sma.h!"
#endif

// Macro ensures unique function names per plugin
#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)
#define LOG_EVENT CONCAT(ANALYTICS_PREFIX, _LogEvent)

inline void LOG_EVENT(const FString& EventName, const TMap<FString, FString>& Params = {})
{
    static FString ClientID = []() {
        FString Path = FPaths::Combine(
            FPaths::ProjectSavedDir(),
            TEXT("PluginAnalytics"),
            TEXT("ClientID_") + FString(ANALYTICS_PREFIX) + TEXT(".txt")
        );

        FString ID;
        if (FFileHelper::LoadFileToString(ID, *Path))
            return ID;

        ID = FGuid::NewGuid().ToString();
        FFileHelper::SaveStringToFile(ID, *Path);
        return ID;
        }();

    // Firebase Measurement Protocol implementation
    TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
    Request->SetURL("url");

    TSharedPtr<FJsonObject> Payload = MakeShareable(new FJsonObject);
    Payload->SetStringField("client_id", ClientID);

    TArray<TSharedPtr<FJsonValue>> Events;
    TSharedPtr<FJsonObject> Event = MakeShareable(new FJsonObject);
    Event->SetStringField("name", EventName);

    if (Params.Num() > 0)
    {
        TSharedPtr<FJsonObject> ParamsObj = MakeShareable(new FJsonObject);
        for (const auto& Param : Params)
        {
            ParamsObj->SetStringField(Param.Key, Param.Value);
        }
        Event->SetObjectField("params", ParamsObj);
    }

    Events.Add(MakeShareable(new FJsonValueObject(Event)));
    Payload->SetArrayField("events", Events);

    FString RequestBody;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestBody);
    FJsonSerializer::Serialize(Payload.ToSharedRef(), Writer);

    Request->SetVerb(TEXT("POST"));
    Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Request->SetContentAsString(RequestBody);
    Request->ProcessRequest();
}

#undef LOG_EVENT