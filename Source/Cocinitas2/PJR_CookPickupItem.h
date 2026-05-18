// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "PJR_CookPickupItem.generated.h"

/**
 * Actor que representa un ingrediente recogible en el mundo.
 * bPJR_IsCarried y PJR_CarriedByPlayerId son estado replicado.
 * Cuando cambian, el RepNotify reconstruye el visual en todos los clientes.
 */
UCLASS(Blueprintable)
class COCINITAS2_API APJR_CookPickupItem : public AActor
{
	GENERATED_BODY()

public:
	APJR_CookPickupItem();

	// Marca el ingrediente como cogido o suelto. Solo llama desde Master Client.
	UFUNCTION(BlueprintCallable, Category = "CookItem")
	void PJR_SetCarried(bool bNewIsCarried, int32 NewCarriedByPlayerId = -1);

	// Devuelve true si alguien lleva este ingrediente.
	UFUNCTION(BlueprintPure, Category = "CookItem")
	bool PJR_IsCarried() const { return bPJR_IsCarried; }

	// Devuelve el ID del jugador que lo lleva (-1 si nadie).
	UFUNCTION(BlueprintPure, Category = "CookItem")
	int32 PJR_GetCarriedByPlayerId() const { return PJR_CarriedByPlayerId; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Llamado en cada cliente cuando bPJR_IsCarried cambia.
	// Implementa aqui el cambio de visual (ocultar/mostrar mesh, cambiar material, etc.)
	UFUNCTION(BlueprintImplementableEvent, Category = "CookItem|Visual")
	void PJR_ApplyCarriedVisual();

	// True si alguien lleva este ingrediente.
	// ReplicatedUsing: cuando llega el nuevo valor, llama a OnRep_IsCarried.
	UPROPERTY(ReplicatedUsing = OnRep_IsCarried, BlueprintReadOnly, Category = "CookItem")
	bool bPJR_IsCarried = false;

	// ID del jugador que lleva el ingrediente. -1 si nadie.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CookItem")
	int32 PJR_CarriedByPlayerId = -1;

private:
	// Callback de replicacion: se ejecuta en clientes cuando bPJR_IsCarried llega.
	UFUNCTION()
	void OnRep_IsCarried();
};
