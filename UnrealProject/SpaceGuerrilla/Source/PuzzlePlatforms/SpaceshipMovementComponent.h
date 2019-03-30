// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpaceshipMovementComponent.generated.h"

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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PUZZLEPLATFORMS_API USpaceshipMovementComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpaceshipMovementComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FSpaceshipMove CreateMove(float DeltaTime);

	void SimulateMove(const FSpaceshipMove& Move);

	////////// Getting FIXED Variables
	float GetTurnSpeed() { return TurnSpeed; };



	///////// Setting and Getting NON fixed variables
	float GetCurrentYawSpeed() { return CurrentYawSpeed; };
	void SetCurrentYawSpeed(float Val) { CurrentYawSpeed = Val; };

	float GetCurrentPitchSpeed() { return CurrentPitchSpeed; };
	void SetCurrentPitchSpeed(float Val) { CurrentPitchSpeed = Val; };

	float GetCurrentRollSpeed() { return CurrentRollSpeed; };
	void SetCurrentRollSpeed(float Val) { CurrentRollSpeed = Val; };

	float GetCurrentStrafeSpeed() { return CurrentStrafeSpeed; };
	void SetCurrentStrafeSpeed(float Val) { CurrentStrafeSpeed = Val; };

	float GetRollRoll() { return RollRoll; };
	void SetRollRoll(float Val) { RollRoll = Val; };

	float GetCurrentForwardSpeed() { return CurrentForwardSpeed; };
	void SetCurrentForwardSpeed(float Val) { CurrentForwardSpeed = Val; };

	/////////////////////
	void SetThrottle(float Val) { Throttle = Val; };
	void SetPitchRotationRatio(float Val) { PitchRotationRatio = Val; };
	void SetYawRotationRatio(float Val) { YawRotationRatio = Val; };
	void SetRollRotationRatio(float Val) { RollRotationRatio = Val; };

private:	
	/////////////////////////////** Fixed variables **/////////////////////////////////////

	// Movement 
	UPROPERTY(Category = "Movement", EditAnywhere)
	float Acceleration;

	UPROPERTY(Category = "Movement", EditAnywhere)
	float TurnSpeed;

	UPROPERTY(Category = "Movement", EditAnywhere)
	float MaxSpeed;

	UPROPERTY(Category = "Movement", EditAnywhere)
	float MinSpeed;

	// Camera
	UPROPERTY(Category = "Camera", EditAnywhere)
	float CameraMaxPitch;

	UPROPERTY(Category = "Camera", EditAnywhere)
	float CameraMinPitch;

	UPROPERTY(Category = "Camera", EditAnywhere)
	float CameraMaxYaw;

	UPROPERTY(Category = "Camera", EditAnywhere)
	float CameraMinYaw;

	/////////////////////////////** NON Fixed variables **/////////////////////////////////////

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
};
