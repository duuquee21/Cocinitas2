// Copyright Epic Games, Inc. All Rights Reserved.

#include "PJR_CookingZone.h"
#include "Net/UnrealNetwork.h"

APJR_CookingZone::APJR_CookingZone()
{
	bReplicates = true;
}

void APJR_CookingZone::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APJR_CookingZone, PJR_ZoneState);
	DOREPLIFETIME(APJR_CookingZone, PJR_CurrentItem);
}

void APJR_CookingZone::PJR_SetState(EPJRCookingZoneState NewState)
{
	// Solo el Master Client cambia el estado.
	PJR_ZoneState = NewState;
	// En el Master Client el OnRep no se dispara solo, lo llamamos manualmente.
	PJR_ApplyStateVisual();
}

void APJR_CookingZone::PJR_SetCurrentItem(AActor* NewItem)
{
	PJR_CurrentItem = NewItem;
}

// Se ejecuta en los clientes cuando PJR_ZoneState llega por red.
void APJR_CookingZone::OnRep_ZoneState()
{
	PJR_ApplyStateVisual();
}
