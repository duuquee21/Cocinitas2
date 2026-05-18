// Copyright Epic Games, Inc. All Rights Reserved.

#include "Cocinitas2Character.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "PJR_CookPickupItem.h"
#include "PJR_CoopWorldController.h"

ACocinitas2Character::ACocinitas2Character()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create the camera boom component
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	// Create the camera component
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));

	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

void ACocinitas2Character::BeginPlay()
{
	Super::BeginPlay();

	// stub
}

void ACocinitas2Character::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	// stub
}

void ACocinitas2Character::PJR_TryPickupNearest()
{
	UWorld* World = GetWorld();
	if (!World) return;

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, TEXT("[E] TryPickupNearest ejecutado"));

	// Busca el CoopWorldController en el nivel
	TArray<AActor*> Controllers;
	UGameplayStatics::GetAllActorsOfClass(World, APJR_CoopWorldController::StaticClass(), Controllers);
	if (Controllers.Num() == 0)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("[E] No hay CoopWorldController en el nivel!"));
		return;
	}
	APJR_CoopWorldController* WorldController = Cast<APJR_CoopWorldController>(Controllers[0]);
	if (!WorldController) return;

	// Encuentra el ingrediente libre mas cercano
	TArray<AActor*> Items;
	UGameplayStatics::GetAllActorsOfClass(World, APJR_CookPickupItem::StaticClass(), Items);

	if (Items.Num() == 0)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red, TEXT("[E] No hay CookPickupItems en el nivel!"));
		return;
	}

	APJR_CookPickupItem* NearestItem = nullptr;
	float NearestDist = MAX_FLT;
	const FVector MyLocation = GetActorLocation();

	for (AActor* Actor : Items)
	{
		APJR_CookPickupItem* Item = Cast<APJR_CookPickupItem>(Actor);
		// Ignora items nulos o que ya lleva alguien
		if (!Item || Item->PJR_IsCarried()) continue;

		const float Dist = FVector::Dist(MyLocation, Item->GetActorLocation());
		if (Dist < NearestDist)
		{
			NearestDist = Dist;
			NearestItem = Item;
		}
	}

	if (!NearestItem)
	{
		if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, TEXT("[E] Todos los items ya estan siendo llevados"));
		return;
	}

	if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
		FString::Printf(TEXT("[E] Item encontrado a %.1f unidades. MaxDist=%.1f"), NearestDist, 180.0f));

	// Pide al CoopWorldController que valide y apruebe el pickup
	WorldController->PJR_RequestPickupFromLocal(NearestItem, this);
}

void ACocinitas2Character::PJR_TryDropCarried()
{
	UWorld* World = GetWorld();
	if (!World) return;

	// Obtiene el ID de este jugador local
	int32 MyPlayerId = -1;
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		MyPlayerId = UGameplayStatics::GetPlayerControllerID(PC);
	}

	// Busca el item que lleva este jugador y lo suelta
	TArray<AActor*> Items;
	UGameplayStatics::GetAllActorsOfClass(World, APJR_CookPickupItem::StaticClass(), Items);

	for (AActor* Actor : Items)
	{
		APJR_CookPickupItem* Item = Cast<APJR_CookPickupItem>(Actor);
		if (!Item) continue;
		if (Item->PJR_IsCarried() && Item->PJR_GetCarriedByPlayerId() == MyPlayerId)
		{
			Item->PJR_SetCarried(false, -1);
			return;
		}
	}
}
