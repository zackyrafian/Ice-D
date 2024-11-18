// Copyright Epic Games, Inc. All Rights Reserved.


#include "IcePlayerController.h"
#include "IcePawn.h"
#include "IceUI.h"
#include "EnhancedInputSubsystems.h"
#include "ChaosWheeledVehicleMovementComponent.h"

void AIcePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	// spawn the UI widget and add it to the viewport
	VehicleUI = CreateWidget<UIceUI>(this, VehicleUIClass);

	check(VehicleUI);

	VehicleUI->AddToViewport();
}

void AIcePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	// get the enhanced input subsystem
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		// add the mapping context so we get controls
		Subsystem->AddMappingContext(InputMappingContext, 0);
	}

	InputComponent->BindAction("PauseGame", IE_Pressed, this, &AIcePlayerController::TogglePauseMenu);

}

void AIcePlayerController::Tick(float Delta)
{
	Super::Tick(Delta);

	if (IsValid(VehiclePawn) && IsValid(VehicleUI))
	{
		VehicleUI->UpdateSpeed(VehiclePawn->GetChaosVehicleMovement()->GetForwardSpeed());
		VehicleUI->UpdateGear(VehiclePawn->GetChaosVehicleMovement()->GetCurrentGear());
		VehicleUI->UpdateRPM(VehiclePawn->GetChaosVehicleMovement()->GetEngineRotationSpeed());
	}
}

void AIcePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	// VehiclePawn = CastChecked<AIcePawn>(InPawn);
	VehiclePawn = CastChecked<AIcePawn>(InPawn);
}

void AIcePlayerController::TogglePauseMenu()
{
	if (bIsPaused)
	{
		ResumePauseMenu();
	}
	else
	{
		PauseGame();
	}
}

void AIcePlayerController::PauseGame()
{
	bIsPaused = true;
	SetPause(true);
	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());
    
	if (PauseMenuClass && !PauseMenuWidget)
	{
		PauseMenuWidget = CreateWidget<UUserWidget>(this, PauseMenuClass);
	}
    
	if (PauseMenuWidget && !PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->AddToViewport();
	}
}

void AIcePlayerController::ResumePauseMenu()
{
	bIsPaused = false;
	SetPause(false);
	bShowMouseCursor = false;
	SetInputMode(FInputModeGameOnly());
    
	if (PauseMenuWidget && PauseMenuWidget->IsInViewport())
	{
		PauseMenuWidget->RemoveFromViewport();
	}
}