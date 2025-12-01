#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class EOSKITSESSIONS_API FEOSKitSessionsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
