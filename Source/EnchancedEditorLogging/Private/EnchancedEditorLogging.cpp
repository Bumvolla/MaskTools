// Copyright (c) 2025 Sora Mas 
// All rights reserved. 

#include "EnchancedEditorLogging.h"
#include "Logging.h"

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FEnchancedEditorLogging"

void FEnchancedEditorLogging::StartupModule()
{
}

void FEnchancedEditorLogging::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FEnchancedEditorLogging, EnchancedEditorLogging);