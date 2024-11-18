// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Ice/IcePawn.h"
#include "InputActionValue.h"
#include "VehiclePawnTest.generated.h"

class UCameraComponent;
class UInputAction;

UCLASS()
class ICE_API AVehiclePawnTest : public AIcePawn
{
	GENERATED_BODY()

public:
	AVehiclePawnTest();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void PostInitializeComponents() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mesh")
	FString NewMeshName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* PhotoModeCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photo Mode")
	bool bPhotoModeActive;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* PhotoModeAction;

	void TogglePhotoMode();
	void TakePhoto();

private:
	void ChangeMesh();
};