// Copyright Epic Games, Inc. All Rights Reserved.

#include "PJR_CookPickupItem.h"
#include "Net/UnrealNetwork.h"

APJR_CookPickupItem::APJR_CookPickupItem()
{
	// Este actor debe replicarse para que todos los clientes vean su estado.
	bReplicates = true;
	// Replica tambien posicion y rotacion (el jugador lo mueve consigo).
	SetReplicateMovement(true);
}

void APJR_CookPickupItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Registra las variables que Photon/Unreal enviara a todos los clientes.
	DOREPLIFETIME(APJR_CookPickupItem, bPJR_IsCarried);
	DOREPLIFETIME(APJR_CookPickupItem, PJR_CarriedByPlayerId);
}

void APJR_CookPickupItem::PJR_SetCarried(bool bNewIsCarried, int32 NewCarriedByPlayerId)
{
	// Solo el Master Client debe llamar a esta funcion.
	// La variable replicada se actualiza aqui y Unreal la envia a los demas clientes.
	bPJR_IsCarried = bNewIsCarried;
	PJR_CarriedByPlayerId = bNewIsCarried ? NewCarriedByPlayerId : -1;

	// En el Master Client el OnRep no se llama automaticamente,
	// asi que aplicamos el visual aqui directamente.
	PJR_ApplyCarriedVisual();
}

// Esta funcion se ejecuta SOLO en los clientes (no en el Master Client)
// cuando reciben el nuevo valor de bPJR_IsCarried por red.
void APJR_CookPickupItem::OnRep_IsCarried()
{
	PJR_ApplyCarriedVisual();
}
