// Copyright Epic Games, Inc. All Rights Reserved.


#include "EOK_PurchaseItem_AsyncFunction.h"

#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlinePurchaseInterface.h"

UEOK_PurchaseItem_AsyncFunction* UEOK_PurchaseItem_AsyncFunction::PurchaseItem(FString ItemID)
{
	UEOK_PurchaseItem_AsyncFunction* BlueprintNode = NewObject<UEOK_PurchaseItem_AsyncFunction>();
	BlueprintNode->ItemID = ItemID;
	return BlueprintNode;
}

void UEOK_PurchaseItem_AsyncFunction::Activate()
{
	PurchaseItem();
	Super::Activate();
}

void UEOK_PurchaseItem_AsyncFunction::PurchaseItem()
{
	if (const IOnlineSubsystem* SubsystemRef = Online::GetSubsystem(this->GetWorld()))
	{
		if (const IOnlineStoreV2Ptr StoreV2Ptr = SubsystemRef->GetStoreV2Interface())
		{
			if (const IOnlineIdentityPtr IdentityPointerRef = SubsystemRef->GetIdentityInterface())
			{
				if (const IOnlinePurchasePtr Purchase = SubsystemRef->GetPurchaseInterface())
				{
					FPurchaseCheckoutRequest Request = {};
					Request.AddPurchaseOffer(TEXT(""), ItemID, 1);

					Purchase->Checkout(*IdentityPointerRef->GetUniquePlayerId(0).Get(),
						Request,
						FOnPurchaseCheckoutComplete::CreateLambda(
							[this](
							const FOnlineError& Result,
							const TSharedRef<FPurchaseReceipt>& Receipt)
							{
								if (Result.WasSuccessful())
								{
									if(!bDelegateCalled)
									{
										OnSuccess.Broadcast();
										bDelegateCalled = true;
									}
								}
								else
								{
									if(!bDelegateCalled)
									{
										OnFail.Broadcast();
										bDelegateCalled = true;
									}
								}
							})
					);
				}
				else
				{
					if(!bDelegateCalled)
					{
						OnFail.Broadcast();
						bDelegateCalled = true;
					}
				}
			}
			else
			{
				if(!bDelegateCalled)
				{
					OnFail.Broadcast();
					bDelegateCalled = true;
				}
			}
		}
		else
		{
			if(!bDelegateCalled)
			{
				OnFail.Broadcast();
				bDelegateCalled = true;
			}
		}
	}
	else
	{
		if(!bDelegateCalled)
		{
			OnFail.Broadcast();
			bDelegateCalled = true;
		}
	}
}
