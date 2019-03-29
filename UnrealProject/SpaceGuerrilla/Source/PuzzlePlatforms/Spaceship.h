// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Spaceship.generated.h"

USTRUCT()
struct FSpaceshipMove
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float Throttle;

	UPROPERTY()
	float PitchRotationRatio;

	UPROPERTY()
	float YawRotationRatio;

	UPROPERTY()
	float RollRotationRatio;

	UPROPERTY()
	float DeltaTime;

	UPROPERTY()
	float Time;
};

USTRUCT()
struct FSpaceshipState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	float CurrentYawSpeed;

	UPROPERTY()
	float CurrentPitchSpeed;

	UPROPERTY()
	float CurrentRollSpeed;

	UPROPERTY()
	float CurrentStrafeSpeed;

	UPROPERTY()
	float RollRoll;

	UPROPERTY()
	float CurrentForwardSpeed;

	UPROPERTY()
	FTransform Transform;

	UPROPERTY()
	FSpaceshipMove LastMove;
};


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

	//Server input implementation
	//Input functions trasferiti in controller

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FSpaceshipMove Move);

	FSpaceshipMove CreateMove(float DeltaTime);

	void SimulateMove(FSpaceshipMove Move);

	void ClearAcknowledgeMoves(FSpaceshipMove LastMove);

	//UFUNCTION(Server, Reliable, WithValidation)
	//void Server_MoveForwardInput(float Val);

	//UFUNCTION(Server, Reliable, WithValidation)
	//void Server_MoveYawInput(float Val);
	
	//UFUNCTION(Server, Reliable, WithValidation)
	//void Server_PitchCamera(float Val);

	//UFUNCTION(Server, Reliable, WithValidation)
	//void Server_YawCamera(float Val);

	//UPROPERTY(EditAnywhere, Category = "Movement")
	//float PitchValue;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float YawValue;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float RollValue;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float Acceleration;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float TurnSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MinSpeed;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float Boost;

	UPROPERTY(Category = Camera, BlueprintReadWrite, EditAnywhere)
	float CameraMaxPitch;

	UPROPERTY(Category = Camera, BlueprintReadWrite, EditAnywhere)
	float CameraMinPitch;

	UPROPERTY(Category = Camera, BlueprintReadWrite, EditAnywhere)
	float CameraMaxYaw;
	UPROPERTY(Category = Camera, BlueprintReadWrite, EditAnywhere)
	float CameraMinYaw;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gameplay)
	FVector MuzzleOffset;


	// ServerState
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FSpaceshipState ServerState;
	

	// Now these are all variables of the clients, We replicate them in the structs
	float CurrentYawSpeed;	
	float CurrentPitchSpeed;	
	float CurrentRollSpeed;	
	float CurrentStrafeSpeed;	
	float RollRoll;	
	float CurrentForwardSpeed;	
	//
	float Throttle;	
	float PitchRotationRatio;	
	float YawRotationRatio;	
	float RollRotationRatio;

	TArray<FSpaceshipMove> UnacknowledgeMoves;

	// Function to call when replicated transform changes
	UFUNCTION()
	void OnRep_ServerState();
};
