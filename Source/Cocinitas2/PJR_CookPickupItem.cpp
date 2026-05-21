// Copyright Epic Games, Inc. All Rights Reserved.

#include "PJR_CookPickupItem.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

APJR_CookPickupItem::APJR_CookPickupItem()
{
	bReplicates = true;
	SetReplicateMovement(true);
}

void APJR_CookPickupItem::BeginPlay()
{
	Super::BeginPlay();
	// Guardar escala original UNA sola vez al inicio, antes de cualquier pickup.
	PJR_OriginalScale = GetActorScale3D();
}

void APJR_CookPickupItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APJR_CookPickupItem, bPJR_IsCarried);
	DOREPLIFETIME(APJR_CookPickupItem, PJR_CarriedByPlayerId);
	DOREPLIFETIME(APJR_CookPickupItem, PJR_CarriedByCharacter);
}

void APJR_CookPickupItem::PJR_SetCarried(bool bNewIsCarried, int32 NewCarriedByPlayerId, ACharacter* CarriedBy)
{
	bPJR_IsCarried        = bNewIsCarried;
	PJR_CarriedByPlayerId = bNewIsCarried ? NewCarriedByPlayerId : -1;
	PJR_CarriedByCharacter = bNewIsCarried ? CarriedBy : nullptr;

	// Ejecutar en el servidor directamente (OnRep no se llama en el servidor).
	if (bNewIsCarried && CarriedBy)
	{
		PJR_AttachToCarrier(CarriedBy);
	}
	else
	{
		PJR_DetachFromCarrier();
	}

	PJR_ApplyCarriedVisual();

	// Notificar al resto de clientes via Photon Event (implementado en Blueprint).
	PJR_BroadcastCarriedState(bNewIsCarried, PJR_CarriedByPlayerId);
}

// Fires en clientes cuando bPJR_IsCarried cambia.
void APJR_CookPickupItem::OnRep_IsCarried()
{
	if (!bPJR_IsCarried)
	{
		// Soltar: seguro independientemente del orden de llegada.
		PJR_DetachFromCarrier();
		PJR_ApplyCarriedVisual();
	}
	else if (PJR_CarriedByCharacter)
	{
		// Los dos valores llegaron en el mismo paquete: adjuntar ya.
		PJR_AttachToCarrier(PJR_CarriedByCharacter);
		PJR_ApplyCarriedVisual();
	}
	// Si bPJR_IsCarried=true pero PJR_CarriedByCharacter todavia es null,
	// esperamos a OnRep_CarriedByCharacter para adjuntar.
}

// Fires en clientes cuando PJR_CarriedByCharacter cambia.
// Esto resuelve el caso en que el personaje llega en un paquete posterior a bPJR_IsCarried.
void APJR_CookPickupItem::OnRep_CarriedByCharacter()
{
	if (bPJR_IsCarried && PJR_CarriedByCharacter)
	{
		PJR_AttachToCarrier(PJR_CarriedByCharacter);
		PJR_ApplyCarriedVisual();
	}
}

void APJR_CookPickupItem::PJR_AttachToCarrier(ACharacter* Carrier)
{
	if (!Carrier) return;

	AttachToComponent(
		Carrier->GetRootComponent(),
		FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld, false)
	);

	SetActorRelativeLocation(PJR_CarryOffset);
	SetActorRelativeRotation(FRotator::ZeroRotator);
	SetActorScale3D(PJR_CarryScale);
}

void APJR_CookPickupItem::PJR_DetachFromCarrier()
{
	DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	SetActorScale3D(PJR_OriginalScale);
}

void APJR_CookPickupItem::PJR_ApplyCarriedFromEvent(bool bIsCarried, ACharacter* Carrier)
{
	// Aplicar estado local sin emitir broadcast (evita bucle infinito).
	bPJR_IsCarried        = bIsCarried;
	PJR_CarriedByCharacter = bIsCarried ? Carrier : nullptr;
	PJR_CarriedByPlayerId  = -1;

	if (bIsCarried && Carrier)
	{
		if (APlayerController* PC = Cast<APlayerController>(Carrier->GetController()))
		{
			PJR_CarriedByPlayerId = UGameplayStatics::GetPlayerControllerID(PC);
		}
		PJR_AttachToCarrier(Carrier);
	}
	else
	{
		PJR_DetachFromCarrier();
	}

	PJR_ApplyCarriedVisual();
}

