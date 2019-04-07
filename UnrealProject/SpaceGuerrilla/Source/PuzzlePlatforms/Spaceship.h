// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SpaceshipMovementComponent.h"
#include "SpaceshipMovementReplicator.h"
#include "Spaceship.generated.h"




UCLASS()
class PUZZLEPLATFORMS_API ASpaceship : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ASpaceship();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	


	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;	

	FVector2D MovementInput;
	FVector2D CameraInput;

	//Client movement
	
	// Client input implementation
	void MoveForwardInput(float Val);
	void MoveYawInput(float Val);
	void PitchCamera(float Val);
	void YawCamera(float Val);

	// Not used variables
	UPROPERTY(EditAnywhere, Category = "Movement")
	float YawValue;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float RollValue;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float Boost;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector MuzzleOffset;	

	// Components
	UPROPERTY(VisibleAnywhere)
	USpaceshipMovementComponent* MovementComponent;

	UPROPERTY(VisibleAnywhere)
	USpaceshipMovementReplicator* MovementReplicator;

	
	
	
};
