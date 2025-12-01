#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class EOSKIT_API FEOSKitModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
