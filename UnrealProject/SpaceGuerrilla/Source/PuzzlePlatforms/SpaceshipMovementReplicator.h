// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpaceshipMovementComponent.h"
#include "SpaceshipMovementReplicator.generated.h"


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

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PUZZLEPLATFORMS_API USpaceshipMovementReplicator : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpaceshipMovementReplicator();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	USpaceshipMovementComponent* MovementComponent;

	void ClearAcknowledgeMoves(FSpaceshipMove LastMove);
	
	void UpdateServerState(const FSpaceshipMove& Move);
	void ClientTick(float DeltaTime);


	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FSpaceshipMove Move);

	// ServerState
	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FSpaceshipState ServerState;

	TArray<FSpaceshipMove> UnacknowledgeMoves;

	float ClientTimeSinceUpdate;
	float ClientTimeBetweenUpdates;
	FVector ClientStartLocation;

	// Function to call when replicated transform changes
	UFUNCTION()
	void OnRep_ServerState();

	void SimulatedProxy_OnRep_ServerState();
	void AutonomousProxy_OnRep_ServerState();
};
