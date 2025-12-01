#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class EOSKITAUTH_API FEOSKitAuthModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
