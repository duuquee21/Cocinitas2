// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "PJR_CookingZone.generated.h"

// Estados posibles de una estacion de cocina.
UENUM(BlueprintType)
enum class EPJRCookingZoneState : uint8
{
	Idle		UMETA(DisplayName = "Libre"),
	Processing	UMETA(DisplayName = "Procesando"),
	Ready		UMETA(DisplayName = "Listo"),
	Blocked		UMETA(DisplayName = "Bloqueado"),
};

/**
 * Actor que representa una estacion de procesado (cocina, corte, etc.).
 * Estado y item actual son estado replicado.
 * El visual (material, particulas) se reconstruye localmente en cada cliente via RepNotify.
 */
UCLASS(Blueprintable)
class COCINITAS2_API APJR_CookingZone : public AActor
{
	GENERATED_BODY()

public:
	APJR_CookingZone();

	// Cambia el estado de la estacion. Solo llama desde Master Client.
	UFUNCTION(BlueprintCallable, Category = "CookingZone")
	void PJR_SetState(EPJRCookingZoneState NewState);

	// Asigna el item que esta en la estacion. Pasa nullptr para vaciarlo.
	UFUNCTION(BlueprintCallable, Category = "CookingZone")
	void PJR_SetCurrentItem(AActor* NewItem);

	UFUNCTION(BlueprintPure, Category = "CookingZone")
	EPJRCookingZoneState PJR_GetState() const { return PJR_ZoneState; }

	UFUNCTION(BlueprintPure, Category = "CookingZone")
	AActor* PJR_GetCurrentItem() const { return PJR_CurrentItem; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// Implementa en Blueprint: cambia material/particulas segun PJR_ZoneState.
	UFUNCTION(BlueprintImplementableEvent, Category = "CookingZone|Visual")
	void PJR_ApplyStateVisual();

	// Estado de la estacion replicado con RepNotify.
	UPROPERTY(ReplicatedUsing = OnRep_ZoneState, BlueprintReadOnly, Category = "CookingZone")
	EPJRCookingZoneState PJR_ZoneState = EPJRCookingZoneState::Idle;

	// Item actual en la estacion (puede ser nullptr). Replicado.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "CookingZone")
	TObjectPtr<AActor> PJR_CurrentItem = nullptr;

private:
	UFUNCTION()
	void OnRep_ZoneState();
};
