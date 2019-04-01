// Fill out your copyright notice in the Description page of Project Settings.

#include "SpaceshipMovementReplicator.h"
#include "UnrealNetwork.h"

// Sets default values for this component's properties
USpaceshipMovementReplicator::USpaceshipMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}


// Called when the game starts
void USpaceshipMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	MovementComponent = GetOwner()->FindComponentByClass<USpaceshipMovementComponent>();
	
}


// Called every frame
void USpaceshipMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComponent == nullptr) return;

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		FSpaceshipMove Move = MovementComponent->CreateMove(DeltaTime);
		MovementComponent->SimulateMove(Move);

		UnacknowledgeMoves.Add(Move);
		Server_SendMove(Move);
	}

	// we are the server and in control of the pawn
	if (GetOwnerRole() == ROLE_Authority && GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FSpaceshipMove Move = MovementComponent->CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	if (GetOwnerRole() == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
	}

}

void USpaceshipMovementReplicator::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(USpaceshipMovementReplicator, ServerState);
}

// Replicated function called when the variable replicated changes
void USpaceshipMovementReplicator::OnRep_ServerState()
{
	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComponent->SetCurrentForwardSpeed(ServerState.CurrentForwardSpeed);
	MovementComponent->SetCurrentPitchSpeed(ServerState.CurrentPitchSpeed);
	MovementComponent->SetCurrentRollSpeed(ServerState.CurrentRollSpeed);
	MovementComponent->SetCurrentStrafeSpeed(ServerState.CurrentStrafeSpeed);
	MovementComponent->SetCurrentYawSpeed(ServerState.CurrentYawSpeed);
	MovementComponent->SetRollRoll(ServerState.RollRoll);

	ClearAcknowledgeMoves(ServerState.LastMove);

	for (const FSpaceshipMove& Move : UnacknowledgeMoves)
	{
		MovementComponent->SimulateMove(Move);
	}
}

void USpaceshipMovementReplicator::ClearAcknowledgeMoves(FSpaceshipMove LastMove)
{
	TArray<FSpaceshipMove> NewMoves;

	for (const FSpaceshipMove& Move : UnacknowledgeMoves)
	{
		if (Move.Time > LastMove.Time)
		{
			NewMoves.Add(Move);
		}
	}

	UnacknowledgeMoves = NewMoves;
}

// Movement Server Implementation and validation. Validation is for anticheat
void USpaceshipMovementReplicator::Server_SendMove_Implementation(FSpaceshipMove Move)
{
	if (MovementComponent == nullptr) return;

	MovementComponent->SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.CurrentForwardSpeed = MovementComponent->GetCurrentForwardSpeed();
	ServerState.CurrentPitchSpeed = MovementComponent->GetCurrentPitchSpeed();
	ServerState.CurrentRollSpeed = MovementComponent->GetCurrentRollSpeed();
	ServerState.CurrentStrafeSpeed = MovementComponent->GetCurrentStrafeSpeed();
	ServerState.CurrentYawSpeed = MovementComponent->GetCurrentYawSpeed();
	ServerState.RollRoll = MovementComponent->GetRollRoll();
}

bool USpaceshipMovementReplicator::Server_SendMove_Validate(FSpaceshipMove Move)
{
	return true; //TODO make better validation
}
