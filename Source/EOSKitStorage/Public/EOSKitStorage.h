#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class EOSKITSTORAGE_API FEOSKitStorageModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};


