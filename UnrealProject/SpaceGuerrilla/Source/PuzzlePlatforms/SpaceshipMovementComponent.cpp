// Fill out your copyright notice in the Description page of Project Settings.

#include "SpaceshipMovementComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/SpringArmComponent.h"
// Sets default values for this component's properties
USpaceshipMovementComponent::USpaceshipMovementComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	MaxSpeed = 100000.f;
	TurnSpeed = 80.f;
	Acceleration = 6000;
	MinSpeed = 0.f;
	// ...
}


// Called when the game starts
void USpaceshipMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void USpaceshipMovementComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// So we can move in a single player scenario, decoupling the movement component and the replicator
	if(GetOwnerRole() == ROLE_AutonomousProxy || GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		LastMove = CreateMove(DeltaTime);
		SimulateMove(LastMove);
	}	
}

FSpaceshipMove USpaceshipMovementComponent::CreateMove(float DeltaTime)
{
	// Setting movement variables from the input that our spaceship receive
	FSpaceshipMove Move;
	Move.DeltaTime = DeltaTime;
	Move.Throttle = Throttle;
	Move.PitchRotationRatio = PitchRotationRatio;
	Move.RollRotationRatio = RollRotationRatio;
	Move.YawRotationRatio = YawRotationRatio;
	AGameStateBase* GameState = GetWorld()->GetGameState();
	Move.Time = GameState->GetServerWorldTimeSeconds();
	// return the move updated
	return Move;
}

// We simulate the move we receive in the parameter
void USpaceshipMovementComponent::SimulateMove(const FSpaceshipMove& Move)
{
	// Alessio's movement
	// TODO Refactoring
	// Move forward tick
	{
		bool bHasInput = !FMath::IsNearlyEqual(Move.Throttle, 0.f);
		float CurrentAcc = bHasInput ? (Move.Throttle * Acceleration) : (-0.2f * Acceleration);
		float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
		CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
		const FVector LocalMove = FVector(CurrentForwardSpeed * Move.DeltaTime, 0.f, 0.f);

		// Move plan forwards (with sweep so we stop when we collide with things)
		GetOwner()->AddActorLocalOffset(LocalMove, true);
	}
	// Rotation Pitch tick
	{
		CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, Move.PitchRotationRatio * TurnSpeed*0.5, GetWorld()->GetDeltaSeconds(), 1.f);  // PitchRotationRatio * TurnSpeed;

		FRotator rotationDelta(CurrentPitchSpeed * Move.DeltaTime, 0, 0);

		FTransform NewTrasform = GetOwner()->GetTransform();
		NewTrasform.ConcatenateRotation(rotationDelta.Quaternion());
		NewTrasform.NormalizeRotation();
		GetOwner()->SetActorTransform(NewTrasform);
	}
	// Rotation Yaw Tick
	{
		float Rotation = GetOwner()->GetActorRotation().Roll;
		bool bIsTurning = (FMath::Abs(Move.YawRotationRatio) > 0.2f) && (FMath::IsWithin(Rotation, -90.f, 90.f));
		if (bIsTurning)
		{
			CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, Move.YawRotationRatio * TurnSpeed, GetWorld()->GetDeltaSeconds(), 1.f);//TurnSpeed * Val;
		}
		else
		{
			CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, GetOwner()->GetActorRotation().Roll * -1.f, GetWorld()->GetDeltaSeconds(), 5.f);//GetActorRotation().Roll * -0.5;
		}
		CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, Move.YawRotationRatio * TurnSpeed, GetWorld()->GetDeltaSeconds(), 1.f); //Val*TurnSpeed;

		// Calculate change in rotation this frame
		FRotator DeltaRotation(0, 0, 0);
		DeltaRotation.Yaw = CurrentYawSpeed * Move.DeltaTime;

		//UE_LOG(LogTemp, Warning, TEXT("Rotazione: %s"), *DeltaRotation.ToString());// Debug console

		// Rotate plane
		GetOwner()->AddActorWorldRotation(FQuat(DeltaRotation), true);
	}

	// Rotation Roll Tick
	{
		RollRoll = FMath::FInterpTo(RollRoll, Move.RollRotationRatio * TurnSpeed * 3, GetWorld()->GetDeltaSeconds(), 1.f);
		FRotator DeltaRotationRoll(0, 0, 0);
		DeltaRotationRoll.Roll = (CurrentRollSpeed + RollRoll * 2)* Move.DeltaTime;
		GetOwner()->AddActorLocalRotation(FQuat(DeltaRotationRoll), true);

		//UE_LOG(LogTemp, Warning, TEXT("Rotazione: %s"), *DeltaRotationRoll.ToString());// Debug console
	}
}