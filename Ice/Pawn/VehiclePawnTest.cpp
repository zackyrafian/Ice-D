// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawnTest.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "Ice/IceWheelFront.h"
#include "Ice/IceWheelRear.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"

AVehiclePawnTest::AVehiclePawnTest()
{
	
	GetChaosVehicleMovement()->WheelSetups.SetNum(4);
	GetChaosVehicleMovement()->WheelSetups[0].WheelClass = UIceWheelFront::StaticClass();
	GetChaosVehicleMovement()->WheelSetups[0].BoneName = FName("FL");
	GetChaosVehicleMovement()->WheelSetups[0].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

	GetChaosVehicleMovement()->WheelSetups[1].WheelClass = UIceWheelFront::StaticClass();
	GetChaosVehicleMovement()->WheelSetups[1].BoneName = FName("FR");
	GetChaosVehicleMovement()->WheelSetups[1].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

	GetChaosVehicleMovement()->WheelSetups[2].WheelClass = UIceWheelRear::StaticClass();
	GetChaosVehicleMovement()->WheelSetups[2].BoneName = FName("RL");
	GetChaosVehicleMovement()->WheelSetups[2].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

	GetChaosVehicleMovement()->WheelSetups[3].WheelClass = UIceWheelRear::StaticClass();
	GetChaosVehicleMovement()->WheelSetups[3].BoneName = FName("RR");
	GetChaosVehicleMovement()->WheelSetups[3].AdditionalOffset = FVector(0.0f, 0.0f, 0.0f);

	GetChaosVehicleMovement()->EngineSetup.TorqueCurve.EditorCurveData.Reset();
	GetChaosVehicleMovement()->EngineSetup.TorqueCurve.EditorCurveData.AddKey(0.0f, 0.0f);      // Idle
	GetChaosVehicleMovement()->EngineSetup.TorqueCurve.EditorCurveData.AddKey(1000.0f, 150.0f); // Low RPM
	GetChaosVehicleMovement()->EngineSetup.TorqueCurve.EditorCurveData.AddKey(2000.0f, 220.0f); // Mid-low RPM
	GetChaosVehicleMovement()->EngineSetup.TorqueCurve.EditorCurveData.AddKey(3000.0f, 260.0f); // Mid RPM, boost kicks in
	GetChaosVehicleMovement()->EngineSetup.TorqueCurve.EditorCurveData.AddKey(4000.0f, 285.0f); // Peak torque
	GetChaosVehicleMovement()->EngineSetup.TorqueCurve.EditorCurveData.AddKey(5000.0f, 280.0f); // High RPM
	GetChaosVehicleMovement()->EngineSetup.TorqueCurve.EditorCurveData.AddKey(6000.0f, 270.0f); // Very high RPM
	GetChaosVehicleMovement()->EngineSetup.TorqueCurve.EditorCurveData.AddKey(7000.0f, 250.0f); // Redline
	
	GetChaosVehicleMovement()->EngineSetup.MaxTorque = 400.0f;
	GetChaosVehicleMovement()->EngineSetup.MaxRPM = 9500.0f;
	GetChaosVehicleMovement()->EngineSetup.EngineIdleRPM = 1200.0f;
	GetChaosVehicleMovement()->EngineSetup.EngineBrakeEffect = 0.2f;
	GetChaosVehicleMovement()->EngineSetup.EngineRevUpMOI = 5.0f;
	GetChaosVehicleMovement()->EngineSetup.EngineRevDownRate = 600.0f;

	GetChaosVehicleMovement()->TransmissionSetup.bUseAutomaticGears = false;
	GetChaosVehicleMovement()->TransmissionSetup.bUseAutoReverse = false;

	GetChaosVehicleMovement()->TransmissionSetup.FinalRatio = 3.7f;

	GetChaosVehicleMovement()->TransmissionSetup.ChangeUpRPM = 6500.0f;
	GetChaosVehicleMovement()->TransmissionSetup.ChangeDownRPM = 2500.0f;

	GetChaosVehicleMovement()->TransmissionSetup.GearChangeTime = 0.1f;

	GetChaosVehicleMovement()->TransmissionSetup.TransmissionEfficiency = 0.85f;

	GetChaosVehicleMovement()->TransmissionSetup.ForwardGearRatios.SetNum(5);
	GetChaosVehicleMovement()->TransmissionSetup.ForwardGearRatios[0] = 3.626f; // 1st
	GetChaosVehicleMovement()->TransmissionSetup.ForwardGearRatios[1] = 2.188f; // 2nd
	GetChaosVehicleMovement()->TransmissionSetup.ForwardGearRatios[2] = 1.541f; // 3rd
	GetChaosVehicleMovement()->TransmissionSetup.ForwardGearRatios[3] = 1.213f; // 4th
	GetChaosVehicleMovement()->TransmissionSetup.ForwardGearRatios[4] = 1.0f;   // 5th

	GetChaosVehicleMovement()->TransmissionSetup.ReverseGearRatios.SetNum(1);
	GetChaosVehicleMovement()->TransmissionSetup.ReverseGearRatios[0] = 3.687f;

	NewMeshName = TEXT("/Game/Vehicles/S14/s14-without-UE4-SUB.s14-without-UE4-SUB");
	
	PhotoModeCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("PhotoModeCamera"));
	if (RootComponent)
	{
		PhotoModeCamera->SetupAttachment(RootComponent);
		PhotoModeCamera->SetRelativeLocation(FVector(-300.0f, 0.0f, 200.0f));
		PhotoModeCamera->SetRelativeRotation(FRotator(-15.0f, 0.0f, 0.0f));
		PhotoModeCamera->bUsePawnControlRotation = false;
	}

	bPhotoModeActive = false;
}


void AVehiclePawnTest::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
    
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (PhotoModeAction)
		{
			EnhancedInputComponent->BindAction(PhotoModeAction, ETriggerEvent::Started, this, &AVehiclePawnTest::TakePhoto);
			UE_LOG(LogTemp, Warning, TEXT("PhotoModeAction bound successfully"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("PhotoModeAction is null"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast to UEnhancedInputComponent"));
	}
}



void AVehiclePawnTest::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	USkeletalMeshComponent* MeshComponent = GetMesh();
	if (MeshComponent && MeshComponent->GetSkeletalMeshAsset())
	{
		FString CurrentMeshName = MeshComponent->GetSkeletalMeshAsset()->GetName();
		UE_LOG(LogTemp, Warning, TEXT("Current Skeletal Mesh Name: %s"), *CurrentMeshName);
	}

	ChangeMesh();
}

void AVehiclePawnTest::ChangeMesh()
{
	if (!NewMeshName.IsEmpty())
	{
		USkeletalMeshComponent* MeshComponent = GetMesh();
		if (MeshComponent)
		{
			// Gunakan NewMeshName yang sudah ada, jangan deklarasikan ulang
			USkeletalMesh* NewMesh = LoadObject<USkeletalMesh>(nullptr, *NewMeshName);
			if (NewMesh)
			{
				MeshComponent->SetSkeletalMesh(NewMesh);
				UE_LOG(LogTemp, Warning, TEXT("Successfully changed mesh to: %s"), *NewMeshName);
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to load new mesh: %s"), *NewMeshName);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NewMeshName is empty. No mesh change attempted."));
	}
}
void AVehiclePawnTest::TogglePhotoMode()
{
	bPhotoModeActive = !bPhotoModeActive;
	UE_LOG(LogTemp, Warning, TEXT("Photo Mode: %s"), bPhotoModeActive ? TEXT("Activated") : TEXT("Deactivated"));

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PhotoModeCamera)
	{
		if (bPhotoModeActive)
		{
			PC->SetViewTargetWithBlend(this, 0.5f);
			PhotoModeCamera->Activate();
			if (GetChaosVehicleMovement())
			{
				GetChaosVehicleMovement()->SetHandbrakeInput(true);
			}
		}
		else
		{
			PC->SetViewTarget(this);
			PhotoModeCamera->Deactivate();
			if (GetChaosVehicleMovement())
			{
				GetChaosVehicleMovement()->SetHandbrakeInput(false);
			}
		}
	}
}



void AVehiclePawnTest::TakePhoto()
{
	UE_LOG(LogTemp, Warning, TEXT("TakePhoto function called"));
    
	if (!bPhotoModeActive)
	{
		TogglePhotoMode();
	}
    
	FString ScreenshotPath = FPaths::ConvertRelativePathToFull(FPaths::ProjectSavedDir() / TEXT("Screenshots"));
	FString FileName = FString::Printf(TEXT("Screenshot_%s.png"), *FDateTime::Now().ToString());
	FString FullPath = ScreenshotPath / FileName;

	UE_LOG(LogTemp, Warning, TEXT("Attempting to save screenshot to: %s"), *FullPath);

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (PC)
	{
		PC->ConsoleCommand(*FString::Printf(TEXT("HighResShot 1920x1080 filename=\"%s\""), *FullPath));
		UE_LOG(LogTemp, Warning, TEXT("Screenshot command issued"));
        
		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, [this, FullPath]()
		{
			if (FPaths::FileExists(FullPath))
			{
				UE_LOG(LogTemp, Warning, TEXT("Screenshot successfully saved to: %s"), *FullPath);
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, FString::Printf(TEXT("Screenshot taken and saved to: %s"), *FullPath));
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to save screenshot to: %s"), *FullPath);
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Failed to save screenshot"));
				}
			}
		}, 2.0f, false);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PlayerController is null"));
	}
}