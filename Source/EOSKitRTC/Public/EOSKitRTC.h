#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class EOSKITRTC_API FEOSKitRTCModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

