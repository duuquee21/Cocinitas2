// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"
#include "PJR_CoopWorldController.generated.h"

/**
 * Actor que actua como autoridad logica del mundo en partidas cooperativas.
 * Solo el Master Client ejecuta la logica critica (score, tiempo, validacion de pickups).
 * La comprobacion de Master Client se delega a Blueprint mediante PJR_QueryIsMasterClient.
 */
UCLASS(Blueprintable)
class COCINITAS2_API APJR_CoopWorldController : public AActor
{
	GENERATED_BODY()

public:
	APJR_CoopWorldController();

	virtual void Tick(float DeltaSeconds) override;

	// Devuelve true si este cliente es el Master Client (o si estamos offline y bTreatOfflineAsMaster es true).
	UFUNCTION(BlueprintPure, Category = "Coop|Authority")
	bool PJR_IsMasterClient() const;

	// Arranca la partida (solo Master Client).
	UFUNCTION(BlueprintCallable, Category = "Coop|Match")
	void PJR_StartMatch();

	// Para la partida (solo Master Client).
	UFUNCTION(BlueprintCallable, Category = "Coop|Match")
	void PJR_StopMatch();

	// Pide sumar puntos. Solo se ejecuta si somos Master Client.
	UFUNCTION(BlueprintCallable, Category = "Coop|Score")
	bool PJR_RequestAddScoreFromLocal(int32 Delta);

	// Pide recoger un objeto. El Master Client valida distancia y referencias.
	UFUNCTION(BlueprintCallable, Category = "Coop|World")
	bool PJR_RequestPickupFromLocal(AActor* ItemActor, AActor* RequestingPlayer);

protected:
	virtual void BeginPlay() override;

	/**
	 * Override esto en tu Blueprint hijo para usar los nodos de Photon Fusion.
	 * La implementacion base devuelve bTreatOfflineAsMaster.
	 * En BP: Get FusionOnlineSubsystem -> Is Master Client -> Return.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintPure, Category = "Coop|Authority")
	bool PJR_QueryIsMasterClient() const;
	virtual bool PJR_QueryIsMasterClient_Implementation() const;

	// Llamado cuando se aprueba un pickup.
	// Implementacion C++ activa PJR_SetCarried en el item automaticamente.
	// Puedes sobreescribir en Blueprint para anadir feedback visual extra.
	UFUNCTION(BlueprintNativeEvent, Category = "Coop|Feedback")
	void PJR_OnPickupApproved(AActor* ItemActor, AActor* RequestingPlayer);
	virtual void PJR_OnPickupApproved_Implementation(AActor* ItemActor, AActor* RequestingPlayer);

	// Llamado cuando se rechaza un pickup.
	// Implementacion C++ loguea el motivo en pantalla.
	UFUNCTION(BlueprintNativeEvent, Category = "Coop|Feedback")
	void PJR_OnPickupRejected(AActor* ItemActor, AActor* RequestingPlayer, const FString& Reason);
	virtual void PJR_OnPickupRejected_Implementation(AActor* ItemActor, AActor* RequestingPlayer, const FString& Reason);

	// Llamado cada tick con el tiempo restante actualizado (solo en Master Client).
	UFUNCTION(BlueprintImplementableEvent, Category = "Coop|Match")
	void PJR_OnMatchTimeUpdated(float NewTimeRemaining);

	// Llamado cuando el tiempo llega a cero.
	UFUNCTION(BlueprintImplementableEvent, Category = "Coop|Match")
	void PJR_OnMatchFinished();

	// Llamado cuando cambia el score.
	UFUNCTION(BlueprintImplementableEvent, Category = "Coop|Score")
	void PJR_OnScoreChanged(int32 NewScore);

	// Si true, cuando no hay conexion a Photon se comporta como Master Client (util para pruebas offline).
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coop|Authority")
	bool bTreatOfflineAsMaster = true;

	// Duracion total de la partida en segundos.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coop|Match", meta = (ClampMin = "1.0"))
	float MatchDurationSeconds = 300.0f;

	// OnRep llamado en los clientes cuando Score cambia.
	UFUNCTION()
	void OnRep_Score();

	// OnRep llamado en los clientes cuando la partida arranca o para.
	UFUNCTION()
	void OnRep_MatchActive();

	// Score replicado. OnRep actualiza el HUD en cada cliente.
	UPROPERTY(ReplicatedUsing = OnRep_Score, BlueprintReadOnly, Category = "Coop|Score")
	int32 Score = 0;

	// Tiempo restante replicado. Se lee directamente desde HUD.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Coop|Match")
	float TimeRemaining = 0.0f;

	// Estado de partida replicado. OnRep actualiza UI/logica local.
	UPROPERTY(ReplicatedUsing = OnRep_MatchActive, BlueprintReadOnly, Category = "Coop|Match")
	bool bMatchActive = false;

	// Distancia maxima para recoger un objeto.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Coop|Validation", meta = (ClampMin = "10.0"))
	float MaxPickupDistance = 180.0f;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
