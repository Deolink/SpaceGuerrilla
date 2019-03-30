// Fill out your copyright notice in the Description page of Project Settings.

#include "Spaceship.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "UnrealNetwork.h"


// Sets default values
ASpaceship::ASpaceship()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MovementComponent = CreateDefaultSubobject<USpaceshipMovementComponent>(TEXT("MovementComponent"));
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

	if (MovementComponent == nullptr) return;

	if (Role == ROLE_AutonomousProxy)
	{
		FSpaceshipMove Move = MovementComponent->CreateMove(DeltaTime);
		MovementComponent->SimulateMove(Move);

		UnacknowledgeMoves.Add(Move);
		Server_SendMove(Move);
	}

	// we are the server and in control of the pawn
	if (Role == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FSpaceshipMove Move = MovementComponent->CreateMove(DeltaTime);
		Server_SendMove(Move);
	}

	if (Role == ROLE_SimulatedProxy)
	{
		MovementComponent->SimulateMove(ServerState.LastMove);
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
	if (MovementComponent == nullptr) return;

	MovementComponent->SetThrottle(Val);
}

void ASpaceship::MoveYawInput(float Val)
{
	if (MovementComponent == nullptr) return;

	MovementComponent->SetRollRotationRatio(Val);	
}

void ASpaceship::PitchCamera(float Val)
{
	if (MovementComponent == nullptr) return;

	MovementComponent->SetPitchRotationRatio(Val);
	CameraInput.Y = FMath::FInterpTo(CameraInput.Y, Val * MovementComponent->GetTurnSpeed(), GetWorld()->GetDeltaSeconds(), 1.f);
}

void ASpaceship::YawCamera(float Val)
{
	if (MovementComponent == nullptr) return;

	MovementComponent->SetYawRotationRatio(Val);
	CameraInput.X = FMath::FInterpTo(CameraInput.X, Val * MovementComponent->GetTurnSpeed(), GetWorld()->GetDeltaSeconds(), 1.f);
}

// Movement Server Implementation and validation. Validation is for anticheat
void ASpaceship::Server_SendMove_Implementation(FSpaceshipMove Move)
{
	if (MovementComponent == nullptr) return;

	MovementComponent->SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.CurrentForwardSpeed = MovementComponent->GetCurrentForwardSpeed();
	ServerState.CurrentPitchSpeed = MovementComponent->GetCurrentPitchSpeed();
	ServerState.CurrentRollSpeed = MovementComponent->GetCurrentRollSpeed();
	ServerState.CurrentStrafeSpeed = MovementComponent->GetCurrentStrafeSpeed();
	ServerState.CurrentYawSpeed = MovementComponent->GetCurrentYawSpeed();
	ServerState.RollRoll = MovementComponent->GetRollRoll();
}

bool ASpaceship::Server_SendMove_Validate(FSpaceshipMove Move)
{
	return true; //TODO make better validation
}
