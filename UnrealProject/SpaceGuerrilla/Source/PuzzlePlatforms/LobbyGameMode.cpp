// Fill out your copyright notice in the Description page of Project Settings.

#include "LobbyGameMode.h"
#include "Engine/World.h"
#include "Public/TimerManager.h"
#include "PuzzlePlatformsGameInstance.h"
// called after a succesfull log in, we check the number of players and then travel after some time
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
	
	++NumberOfPlayers;
	if (NumberOfPlayers >= 2)
	{
		GetWorldTimerManager().SetTimer(GameStartTimer, this, &ALobbyGameMode::StartGame, 3.0);
	}
}
// when logout we change the number of players that are online
void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
	--NumberOfPlayers;
}
// we travel to the level and start the game through the game instance session
void ALobbyGameMode::StartGame()
{
	auto GameInstance = Cast<UPuzzlePlatformsGameInstance>(GetGameInstance());

	if (GameInstance == nullptr) return;

	GameInstance->StartSession();

	UWorld* World = GetWorld();
	if (!ensure(World != nullptr)) return;

	bUseSeamlessTravel = true;

	World->ServerTravel("/Game/PuzzlePlatforms/Maps/SpaceGuerrilla?listen");

}