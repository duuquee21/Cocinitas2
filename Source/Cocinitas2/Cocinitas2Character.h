// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Cocinitas2Character.generated.h"

class APJR_CookPickupItem;
class APJR_CoopWorldController;

/**
 *  A controllable top-down perspective character
 */
UCLASS(abstract)
class ACocinitas2Character : public ACharacter
{
	GENERATED_BODY()

private:

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

public:

	/** Constructor */
	ACocinitas2Character();

	/** Initialization */
	virtual void BeginPlay() override;

	/** Update */
	virtual void Tick(float DeltaSeconds) override;

	/** Returns the camera component **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }

	/** Returns the Camera Boom component **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	// Busca el ingrediente libre mas cercano y envia la peticion al servidor.
	UFUNCTION(BlueprintCallable, Category = "Coop|Pickup")
	void PJR_TryPickupNearest();

	// Suelta el ingrediente que este jugador lleva actualmente.
	UFUNCTION(BlueprintCallable, Category = "Coop|Pickup")
	void PJR_TryDropCarried();

private:
	// RPC servidor: valida y ejecuta el pickup con autoridad para que se replique a todos.
	UFUNCTION(Server, Reliable)
	void Server_RequestPickup(APJR_CookPickupItem* Item);

	// RPC servidor: suelta el item con autoridad.
	UFUNCTION(Server, Reliable)
	void Server_DropCarried(APJR_CookPickupItem* Item);

};

