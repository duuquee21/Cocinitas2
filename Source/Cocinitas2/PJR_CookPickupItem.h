// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"
#include "PJR_CookPickupItem.generated.h"

/**
 * Actor que representa un ingrediente recogible en el mundo.
 * bPJR_IsCarried, PJR_CarriedByPlayerId y PJR_CarriedByCharacter son estado replicado.
 * Cuando cambian, el RepNotify adjunta/desadjunta el item al personaje portador.
 */
UCLASS(Blueprintable)
class COCINITAS2_API APJR_CookPickupItem : public AActor
{
	GENERATED_BODY()

public:
	APJR_CookPickupItem();

	virtual void BeginPlay() override;

	// Marca el ingrediente como cogido o suelto. Solo llama desde Master Client.
	// CarriedBy debe apuntar al personaje portador (o nullptr al soltar).
	UFUNCTION(BlueprintCallable, Category = "CookItem")
	void PJR_SetCarried(bool bNewIsCarried, int32 NewCarriedByPlayerId = -1, ACharacter* CarriedBy = nullptr);

	// Devuelve true si alguien lleva este ingrediente.
	UFUNCTION(BlueprintPure, Category = "CookItem")
	bool PJR_IsCarried() const { return bPJR_IsCarried; }

	// Devuelve el ID del jugador que lo lleva (-1 si nadie).
	UFUNCTION(BlueprintPure, Category = "CookItem")
	int32 PJR_GetCarriedByPlayerId() const { return PJR_CarriedByPlayerId; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// Offset en espacio local del personaje donde aparece el item al ser llevado.
	// X = adelante, Z = arriba.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CookItem|Carry")
	FVector PJR_CarryOffset = FVector(80.f, 0.f, 60.f);

	// Escala del item mientras es llevado.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "CookItem|Carry")
	FVector PJR_CarryScale = FVector(0.5f, 0.5f, 0.5f);

protected:
	// Llamado en cada cliente cuando bPJR_IsCarried cambia.
	// Implementa aqui feedback visual extra (sonido, partículas, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category = "CookItem|Visual")
	void PJR_ApplyCarriedVisual();

	// Llamado en el cliente que ejecuta el pickup/drop.
	// IMPLEMENTAR EN BLUEPRINT con Photon Raise Event para notificar al resto de clientes.
	// Parametros: bIsCarried (true=coger, false=soltar), PlayerId (ID Photon del portador).
	UFUNCTION(BlueprintImplementableEvent, Category = "CookItem|Network")
	void PJR_BroadcastCarriedState(bool bIsCarried, int32 PlayerId);

public:
	// Llamado en cada cliente al recibir el Photon Event.
	// Aplica el estado localmente SIN volver a emitir broadcast (evita bucle infinito).
	// Carrier = personaje portador resuelto desde el jugador Photon (nullptr al soltar).
	UFUNCTION(BlueprintCallable, Category = "CookItem|Network")
	void PJR_ApplyCarriedFromEvent(bool bIsCarried, ACharacter* Carrier);

protected:
	// True si alguien lleva este ingrediente.
	UPROPERTY(ReplicatedUsing = OnRep_IsCarried, BlueprintReadOnly, Category = "CookItem")
	bool bPJR_IsCarried = false;

	// ID del jugador que lleva el ingrediente. -1 si nadie.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CookItem")
	int32 PJR_CarriedByPlayerId = -1;

	// Referencia al personaje portador. RepNotify propio para evitar problemas de orden de llegada.
	UPROPERTY(ReplicatedUsing = OnRep_CarriedByCharacter, BlueprintReadOnly, Category = "CookItem")
	TObjectPtr<ACharacter> PJR_CarriedByCharacter = nullptr;

private:
	// Escala original guardada en BeginPlay (una sola vez, no dentro del attach).
	FVector PJR_OriginalScale = FVector::OneVector;

	// Fires cuando bPJR_IsCarried cambia: gestiona el SOLTAR y el caso en que el personaje ya este disponible.
	UFUNCTION()
	void OnRep_IsCarried();

	// Fires cuando PJR_CarriedByCharacter cambia: gestiona el COGER una vez la referencia esta disponible.
	UFUNCTION()
	void OnRep_CarriedByCharacter();

	// Adjunta este item al personaje portador con offset y escala reducida.
	void PJR_AttachToCarrier(ACharacter* Carrier);

	// Desadjunta este item del personaje y lo devuelve al mundo.
	void PJR_DetachFromCarrier();
};
