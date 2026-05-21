// Copyright Epic Games, Inc. All Rights Reserved.

#include "PJR_CoopWorldController.h"
#include "Net/UnrealNetwork.h"
#include "PJR_CookPickupItem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"

APJR_CoopWorldController::APJR_CoopWorldController()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void APJR_CoopWorldController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APJR_CoopWorldController, Score);
	DOREPLIFETIME(APJR_CoopWorldController, TimeRemaining);
	DOREPLIFETIME(APJR_CoopWorldController, bMatchActive);
}

void APJR_CoopWorldController::BeginPlay()
{
	Super::BeginPlay();
	TimeRemaining = MatchDurationSeconds;
}

void APJR_CoopWorldController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bMatchActive || !PJR_IsMasterClient())
	{
		return;
	}

	TimeRemaining = FMath::Max(0.0f, TimeRemaining - DeltaSeconds);
	PJR_OnMatchTimeUpdated(TimeRemaining);

	if (TimeRemaining <= 0.0f)
	{
		bMatchActive = false;
		PJR_OnMatchFinished();
	}
}

bool APJR_CoopWorldController::PJR_IsMasterClient() const
{
	return PJR_QueryIsMasterClient();
}

// Implementacion base: devuelve bTreatOfflineAsMaster.
// Override en Blueprint para conectar con Photon Fusion.
bool APJR_CoopWorldController::PJR_QueryIsMasterClient_Implementation() const
{
	return bTreatOfflineAsMaster;
}

void APJR_CoopWorldController::PJR_StartMatch()
{
	if (!PJR_IsMasterClient())
	{
		return;
	}

	TimeRemaining = MatchDurationSeconds;
	bMatchActive = true;
	PJR_OnMatchTimeUpdated(TimeRemaining);
}

void APJR_CoopWorldController::PJR_StopMatch()
{
	if (!PJR_IsMasterClient())
	{
		return;
	}

	bMatchActive = false;
}

bool APJR_CoopWorldController::PJR_RequestAddScoreFromLocal(const int32 Delta)
{
	if (!PJR_IsMasterClient())
	{
		return false;
	}

	Score = FMath::Max(0, Score + Delta);
	PJR_OnScoreChanged(Score);
	return true;
}

bool APJR_CoopWorldController::PJR_RequestPickupFromLocal(AActor* ItemActor, AActor* RequestingPlayer)
{
	if (!ItemActor || !RequestingPlayer)
	{
		PJR_OnPickupRejected(ItemActor, RequestingPlayer, TEXT("Referencias nulas."));
		return false;
	}

	const float Distance = FVector::Dist(RequestingPlayer->GetActorLocation(), ItemActor->GetActorLocation());
	if (Distance > MaxPickupDistance)
	{
		PJR_OnPickupRejected(ItemActor, RequestingPlayer, TEXT("Demasiado lejos para recoger."));
		return false;
	}

	PJR_OnPickupApproved(ItemActor, RequestingPlayer);
	return true;
}

// Llamado en clientes cuando Score cambia. Delega la actualizacion visual al Blueprint.
void APJR_CoopWorldController::OnRep_Score()
{
	PJR_OnScoreChanged(Score);
}

// Llamado en clientes cuando bMatchActive cambia. Delega al Blueprint.
void APJR_CoopWorldController::OnRep_MatchActive()
{
	if (bMatchActive)
	{
		PJR_OnMatchTimeUpdated(TimeRemaining);
	}
	else
	{
		PJR_OnMatchFinished();
	}
}

// Implementacion base: activa SetCarried en el item con el ID del jugador que lo pide.
// El Blueprint hijo puede sobreescribir esto para anadir efectos visuales extra.
void APJR_CoopWorldController::PJR_OnPickupApproved_Implementation(AActor* ItemActor, AActor* RequestingPlayer)
{
	APJR_CookPickupItem* Item = Cast<APJR_CookPickupItem>(ItemActor);
	if (!Item)
	{
		return;
	}

	int32 PlayerId = -1;
	if (ACharacter* Char = Cast<ACharacter>(RequestingPlayer))
	{
		if (APlayerController* PC = Cast<APlayerController>(Char->GetController()))
		{
			PlayerId = UGameplayStatics::GetPlayerControllerID(PC);
		}
	}

	Item->PJR_SetCarried(true, PlayerId, Cast<ACharacter>(RequestingPlayer));
}

// Implementacion base: muestra el motivo de rechazo en el log de Unreal.
void APJR_CoopWorldController::PJR_OnPickupRejected_Implementation(AActor* ItemActor, AActor* RequestingPlayer, const FString& Reason)
{
	UE_LOG(LogTemp, Warning, TEXT("[CoopWorldController] Pickup rechazado: %s"), *Reason);
}
