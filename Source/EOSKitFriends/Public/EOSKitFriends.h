#pragma once
#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class EOSKITFRIENDS_API FEOSKitFriendsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

