// Fill out your copyright notice in the Description page of Project Settings.

#include "InGameMenu.h"

#include "Components/Button.h"

bool UInGameMenu::Initialize()
{
	bool Success = Super::Initialize();
	if (!Success) return false;
	// Setting buttons on click
	if (!ensure(CancelButton != nullptr)) return false;
	CancelButton->OnClicked.AddDynamic(this, &UInGameMenu::CancelPressed);
	if (!ensure(QuitButton != nullptr)) return false;
	QuitButton->OnClicked.AddDynamic(this, &UInGameMenu::QuitPressed);

	return true;
}
// close the in game menu
void UInGameMenu::CancelPressed()
{
	Teardown();
}
// close the connection and getting out of the game calling the interface LoadMainMenu
void UInGameMenu::QuitPressed()
{
	if (MenuInterface != nullptr) {
		Teardown();
		MenuInterface->LoadMainMenu();
	}
}
