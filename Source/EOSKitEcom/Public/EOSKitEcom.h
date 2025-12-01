#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class EOSKITECOM_API FEOSKitEcomModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

