// Fill out your copyright notice in the Description page of Project Settings.

#include "Spaceship.h"
#include "Camera/CameraComponent.h"
#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"
#include "GameFramework/GameStateBase.h"

// Sets default values
ASpaceship::ASpaceship()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MaxSpeed = 100000.f;
	TurnSpeed = 80.f;
	Acceleration = 6000;
	MinSpeed = 0.f;
}

// Called when the game starts or when spawned
void ASpaceship::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		NetUpdateFrequency = 1;
	}
}

// Variables to replicate
void ASpaceship::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASpaceship, ServerState);
}

//Function to get the role in string to debug
FString GetEnumText(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";
	case ROLE_SimulatedProxy:
		return "SimulatedProxy";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default:
		return "ERROR";
	}
}

// Called every frame
void ASpaceship::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsLocallyControlled())
	{
		FSpaceshipMove Move = CreateMove(DeltaTime);

		if (!HasAuthority) 
		{
			UnacknowledgeMoves.Add(Move);
			UE_LOG(LogTemp, Warning, TEXT("Queue lenght: %d"), UnacknowledgeMoves.Num()); // Debug console
		}		

		Server_SendMove(Move);

		SimulateMove(Move);// need to fix later
	}

	FRotator Banana = GetActorRotation();// Debug console
	UE_LOG(LogTemp, Warning, TEXT("BANANA: %s"), *Banana.ToString()); // Debug console

	UE_LOG(LogTemp, Warning, TEXT("Rotazione: %s"), *CameraInput.ToString());// Debug console

	// Getting the Role and showing it above the actor
	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(Role), this, FColor::White, DeltaTime);
}

// Called to bind functionality to input
void ASpaceship::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Respond every frame to the values our movements.
	PlayerInputComponent->BindAxis("ForwardSpaceship", this, &ASpaceship::MoveForwardInput);
	PlayerInputComponent->BindAxis("YawSpaceship", this, &ASpaceship::MoveYawInput);
	InputComponent->BindAxis("CameraPitchSpaceship", this, &ASpaceship::PitchCamera);
	InputComponent->BindAxis("CameraYawSpaceship", this, &ASpaceship::YawCamera);

	//TODO Implement projectile
	//InputComponent->BindAction("Fire", IE_Pressed, this, &ASpaceship::OnFire);
}

// Replicated function called when the variable replicated changes
void ASpaceship::OnRep_ServerState()
{
	SetActorTransform(ServerState.Transform);
	CurrentForwardSpeed = ServerState.CurrentForwardSpeed;
	CurrentPitchSpeed = ServerState.CurrentPitchSpeed;
	CurrentRollSpeed = ServerState.CurrentRollSpeed;
	CurrentStrafeSpeed = ServerState.CurrentStrafeSpeed;
	CurrentYawSpeed = ServerState.CurrentYawSpeed;
	RollRoll = ServerState.RollRoll;

	ClearAcknowledgeMoves(ServerState.LastMove);
}

FSpaceshipMove ASpaceship::CreateMove(float DeltaTime)
{
	FSpaceshipMove Move;
	Move.DeltaTime = DeltaTime;
	Move.Throttle = Throttle;
	Move.PitchRotationRatio = PitchRotationRatio;
	Move.RollRotationRatio = RollRotationRatio;
	Move.YawRotationRatio = YawRotationRatio;
	AGameStateBase* GameState = GetWorld()->GetGameState();

	Move.Time = GameState->GetServerWorldTimeSeconds();
	return Move;
}

void ASpaceship::SimulateMove(FSpaceshipMove Move)
{
	// TODO Refactoring
	// Move forward tick
	{
		bool bHasInput = !FMath::IsNearlyEqual(Move.Throttle, 0.f);
		float CurrentAcc = bHasInput ? (Move.Throttle * Acceleration) : (-0.2f * Acceleration);
		float NewForwardSpeed = CurrentForwardSpeed + (GetWorld()->GetDeltaSeconds() * CurrentAcc);
		CurrentForwardSpeed = FMath::Clamp(NewForwardSpeed, MinSpeed, MaxSpeed);
		const FVector LocalMove = FVector(CurrentForwardSpeed * Move.DeltaTime, 0.f, 0.f);

		// Move plan forwards (with sweep so we stop when we collide with things)
		AddActorLocalOffset(LocalMove, true);
	}
	// Rotation Pitch tick
	{
		CurrentPitchSpeed = FMath::FInterpTo(CurrentPitchSpeed, Move.PitchRotationRatio * TurnSpeed*0.5, GetWorld()->GetDeltaSeconds(), 1.f);  // PitchRotationRatio * TurnSpeed;

		FRotator rotationDelta(CurrentPitchSpeed * Move.DeltaTime, 0, 0);

		FTransform NewTrasform = GetTransform();
		NewTrasform.ConcatenateRotation(rotationDelta.Quaternion());
		NewTrasform.NormalizeRotation();
		SetActorTransform(NewTrasform);
	}
	// Rotation Yaw Tick
	{
		float Rotation = GetActorRotation().Roll;
		bool bIsTurning = (FMath::Abs(Move.YawRotationRatio) > 0.2f) && (FMath::IsWithin(Rotation, -90.f, 90.f));
		if (bIsTurning)
		{
			CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, Move.YawRotationRatio * TurnSpeed, GetWorld()->GetDeltaSeconds(), 1.f);//TurnSpeed * Val;
		}
		else
		{
			CurrentRollSpeed = FMath::FInterpTo(CurrentRollSpeed, GetActorRotation().Roll * -1.f, GetWorld()->GetDeltaSeconds(), 5.f);//GetActorRotation().Roll * -0.5;
		}
		CurrentYawSpeed = FMath::FInterpTo(CurrentYawSpeed, Move.YawRotationRatio * TurnSpeed, GetWorld()->GetDeltaSeconds(), 1.f); //Val*TurnSpeed;

		// Calculate change in rotation this frame
		FRotator DeltaRotation(0, 0, 0);
		DeltaRotation.Yaw = CurrentYawSpeed * Move.DeltaTime;

		UE_LOG(LogTemp, Warning, TEXT("Rotazione: %s"), *DeltaRotation.ToString());// Debug console

		// Rotate plane
		AddActorWorldRotation(FQuat(DeltaRotation), true);
	}

	// Rotation Roll Tick
	{
		RollRoll = FMath::FInterpTo(RollRoll, Move.RollRotationRatio * TurnSpeed * 3, GetWorld()->GetDeltaSeconds(), 1.f);
		FRotator DeltaRotationRoll(0, 0, 0);
		DeltaRotationRoll.Roll = (CurrentRollSpeed + RollRoll * 2)* Move.DeltaTime;
		AddActorLocalRotation(FQuat(DeltaRotationRoll), true);

		UE_LOG(LogTemp, Warning, TEXT("Rotazione: %s"), *DeltaRotationRoll.ToString());// Debug console
	}
}

void ASpaceship::ClearAcknowledgeMoves(FSpaceshipMove LastMove)
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

// Movement on client
void ASpaceship::MoveForwardInput(float Val)
{
	Throttle = Val;
}

void ASpaceship::MoveYawInput(float Val)
{
	RollRotationRatio = Val;
}

void ASpaceship::PitchCamera(float Val)
{
	PitchRotationRatio = Val;
	CameraInput.Y = FMath::FInterpTo(CameraInput.Y, PitchRotationRatio * TurnSpeed, GetWorld()->GetDeltaSeconds(), 1.f);
}

void ASpaceship::YawCamera(float Val)
{
	YawRotationRatio = Val;
	CameraInput.X = FMath::FInterpTo(CameraInput.X, YawRotationRatio * TurnSpeed, GetWorld()->GetDeltaSeconds(), 1.f);
}

// Movement Server Implementation and validation. Validation is for anticheat
void ASpaceship::Server_SendMove_Implementation(FSpaceshipMove Move)
{
	SimulateMove(Move);
	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.CurrentForwardSpeed = CurrentForwardSpeed;
	ServerState.CurrentPitchSpeed = CurrentPitchSpeed;
	ServerState.CurrentRollSpeed = CurrentRollSpeed;
	ServerState.CurrentStrafeSpeed = CurrentStrafeSpeed;
	ServerState.CurrentYawSpeed = CurrentYawSpeed;
	ServerState.RollRoll = RollRoll;	
}

bool ASpaceship::Server_SendMove_Validate(FSpaceshipMove Move)
{
	return true; //TODO make better validation
}


/*
bool ASpaceship::Server_MoveForwardInput_Validate(float Val)
{
	return true;
}

void ASpaceship::Server_MoveYawInput_Implementation(float Val)
{
	RollRotationRatio = Val;
}

bool ASpaceship::Server_MoveYawInput_Validate(float Val)
{
	return true;
}

void ASpaceship::Server_PitchCamera_Implementation(float Val)
{
	PitchRotationRatio = Val;
}

bool ASpaceship::Server_PitchCamera_Validate(float Val)
{
	return true;
}

void ASpaceship::Server_YawCamera_Implementation(float Val)
{
	YawRotationRatio = Val;	
}

bool ASpaceship::Server_YawCamera_Validate(float Val)
{
	return true;
}
*/
