// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "IcePlayerController.generated.h"

class UInputMappingContext;
class AIcePawn;
class UIceUI;

/**
 *  Vehicle Player Controller class
 *  Handles input mapping and user interface
 */
UCLASS(abstract)
class ICE_API AIcePlayerController : public APlayerController
{
	GENERATED_BODY()

protected:

	

	

	/** Input Mapping Context to be used for player input */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* InputMappingContext;

	/** Pointer to the controlled vehicle pawn */
	UPROPERTY()
	AIcePawn* VehiclePawn;

	/** Type of the UI to spawn */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = UI)
	TSubclassOf<UIceUI> VehicleUIClass;

	/** Pointer to the UI widget */
	TObjectPtr<UIceUI> VehicleUI;

	// Begin Actor interface
protected:

	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

public:

	virtual void Tick(float Delta) override;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void TogglePauseMenu();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void PauseGame();

	UFUNCTION(BlueprintCallable, Category = "UI")
	void ResumePauseMenu();
protected:

	virtual void OnPossess(APawn* InPawn) override;

	// End PlayerController interface

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> PauseMenuClass;

	UPROPERTY()
	class UUserWidget* PauseMenuWidget;

	bool bIsPaused = false;
};