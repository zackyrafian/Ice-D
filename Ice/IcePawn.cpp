// Copyright Epic Games, Inc. All Rights Reserved.

#include "IcePawn.h"


#include "IceWheelFront.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "IceWheelRear.h"
#include "Components/SkeletalMeshComponent.h"

#define LOCTEXT_NAMESPACE "VehiclePawn"
#define D(x) if(GEngine){GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, TEXT(x));}

DEFINE_LOG_CATEGORY(LogTemplateVehicle);

AIcePawn::AIcePawn()
{
	// construct the front camera boom
	FrontSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Front Spring Arm"));
	FrontSpringArm->SetupAttachment(GetMesh());
	FrontSpringArm->TargetArmLength = 0.0f;
	FrontSpringArm->bDoCollisionTest = false;
	FrontSpringArm->bEnableCameraRotationLag = true;
	FrontSpringArm->CameraRotationLagSpeed = 15.0f;
	FrontSpringArm->SetRelativeLocation(FVector(30.0f, 0.0f, 120.0f));

	FrontCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Front Camera"));
	FrontCamera->SetupAttachment(FrontSpringArm);
	FrontCamera->bAutoActivate = false;

	// construct the back camera boom
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = false;
	BackSpringArm->bInheritPitch = false;
	BackSpringArm->bInheritRoll = false;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 2.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	// Configure the car mesh
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));

	// get the Chaos Wheeled movement component
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());

	// Configure Debug
	bShowDebugMessages = true;
	DebugMessageId = -1;

}

void AIcePawn::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Triggered, this, &AIcePawn::Steering);
		EnhancedInputComponent->BindAction(SteeringAction, ETriggerEvent::Completed, this, &AIcePawn::Steering);

		// throttle 
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AIcePawn::Throttle);
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AIcePawn::Throttle);

		// Clutch
		EnhancedInputComponent->BindAction(ClutchAction, ETriggerEvent::Triggered, this, &AIcePawn::Clutch);
		EnhancedInputComponent->BindAction(ClutchAction, ETriggerEvent::Completed, this, &AIcePawn::Clutch);

		// GearUp
		EnhancedInputComponent->BindAction(GearUpAction, ETriggerEvent::Started, this, &AIcePawn::GearUp);
	
		// GearDown
		EnhancedInputComponent->BindAction(GearDownAction, ETriggerEvent::Started, this, &AIcePawn::GearDown);
		
		// break 
		EnhancedInputComponent->BindAction(BrakeAction, ETriggerEvent::Triggered, this, &AIcePawn::Brake);
		
		// handbrake 
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Started, this, &AIcePawn::StartHandbrake);
		EnhancedInputComponent->BindAction(HandbrakeAction, ETriggerEvent::Completed, this, &AIcePawn::StopHandbrake);

		// look around 
		EnhancedInputComponent->BindAction(LookAroundAction, ETriggerEvent::Triggered, this, &AIcePawn::LookAround);

		// toggle camera 
		EnhancedInputComponent->BindAction(ToggleCameraAction, ETriggerEvent::Triggered, this, &AIcePawn::ToggleCamera);

		// reset the vehicle 
		EnhancedInputComponent->BindAction(ResetVehicleAction, ETriggerEvent::Triggered, this, &AIcePawn::ResetVehicle);
	}
	else
	{
		UE_LOG(LogTemplateVehicle, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void AIcePawn::Tick(float Delta)
{
	Super::Tick(Delta);
	bool bMovingOnGround = ChaosVehicleMovement->IsMovingOnGround();
	GetMesh()->SetAngularDamping(bMovingOnGround ? 0.0f : 3.0f);

	// Camera Vehicle 
	FRotator VehicleRotation = GetActorRotation();
	FVector VehicleLocation = GetActorLocation();
	FVector CameraLocation = VehicleLocation - (GetActorForwardVector() * -20.0f) + FVector(0.0f, 0.0f, 15.0f); 
	BackSpringArm->SetWorldLocation(CameraLocation);
	FRotator CameraRotation = VehicleRotation;  
	float MaxCameraYaw = 15.0f; 

	float YawDifference = CameraRotation.Yaw - BackSpringArm->GetComponentRotation().Yaw;

	if (FMath::Abs(YawDifference) > MaxCameraYaw)
	{
		CameraRotation.Yaw = BackSpringArm->GetComponentRotation().Yaw + FMath::Clamp(YawDifference, -MaxCameraYaw, MaxCameraYaw);
	}
	BackSpringArm->SetWorldRotation(CameraRotation);

	// FPS	
	float FPS = 1.0f / Delta;
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, FString::Printf(TEXT("FPS : %f"), FPS));

	DriftAssist(Delta);
	Vehicle(Delta);
	Engine(Delta);

	// RPM Engine
	RPMVehicleValue = ChaosVehicleMovement->GetEngineRotationSpeed();
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("RPM : %f"), RPMVehicleValue));
	// CurrentGear
	GearCurrent = ChaosVehicleMovement->GetCurrentGear();
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("Gear %d: "), GearCurrent));
	// GearMax 
	const TArray<float>& GearRatios = ChaosVehicleMovement->TransmissionSetup.ForwardGearRatios;
	MaxGear = GearRatios.Num();
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("MaxGear: %d"), MaxGear));
	// GetMaxRPM
	MaxEngineRPM = ChaosVehicleMovement->GetEngineMaxRotationSpeed();
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("MaxEngineRPM: %f"), MaxEngineRPM));
	// GetIdleRPM
	EngineIdleRPM =  ChaosVehicleMovement->EngineSetup.EngineIdleRPM;
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("IdleRPM: %f"), EngineIdleRPM));
	// GetSpeed
	SpeedKPHValue = ChaosVehicleMovement->GetForwardSpeedMPH() * 1.60934;
	
	// float CameraYaw = BackSpringArm->GetRelativeRotation().Yaw;
	// CameraYaw = FMath::FInterpTo(CameraYaw, 0.0f, Delta, 1.0f);
	//
	// BackSpringArm->SetRelativeRotation(FRotator(0.0f, CameraYaw, 0.0f));
}


void AIcePawn::Steering(const FInputActionValue& Value)
{
	float SteerInput = Value.Get<float>();
	SteerMain = SteerInput;
	float SteerSpeed = 8.0f ;
	float DeltaTime = GetWorld()->GetDeltaSeconds();

	if (FMath::Abs(SteerInput) > KINDA_SMALL_NUMBER)
	{
		SteeringValue += SteerInput * DeltaTime * SteerSpeed;
		SteeringValue = FMath::Clamp(SteeringValue, -1.0f, 1.0f);
	}
}

void AIcePawn::DriftAssist(float Delta)
{
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Cyan, FString::Printf(TEXT("CurrentSteeringValue : %f"), SteeringValue));

	FVector Velocity = GetVelocity();
	FVector RightVector = GetActorRightVector();
	FVector ForwardVector = GetActorForwardVector();

	float SidewaysSpeed = FVector::DotProduct(Velocity, RightVector);
	FVector DriftForce = RightVector * SidewaysSpeed * -1.0f;

	FVector CorrectedVelocity = Velocity + DriftForce * Delta;

	float DriftAngle = FMath::Atan2(SidewaysSpeed, FVector::DotProduct(Velocity, ForwardVector));

	float SteeringAdjustment = DriftAngle * 0.5f;
	float AdjustedSteering = SteeringValue + SteeringAdjustment;

	static float CounterSteerIntensity = 0.0f;
	static float CounterSteerRate = 2.0f; 
    
	float MaxCounterSteer = 0.5f;  
	float TargetIntensity = FMath::Clamp(SidewaysSpeed / 400.0f, -MaxCounterSteer, MaxCounterSteer); 

	CounterSteerIntensity = FMath::FInterpTo(CounterSteerIntensity, TargetIntensity, Delta, CounterSteerRate);

	AdjustedSteering += CounterSteerIntensity;

	AdjustedSteering = FMath::Clamp(AdjustedSteering, -1.0f, 1.0f);

	ChaosVehicleMovement->SetSteeringInput(AdjustedSteering);
	ChaosVehicleMovement->Velocity = CorrectedVelocity;

	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("SidewaysSpeed : %f"), SidewaysSpeed));
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Orange, FString::Printf(TEXT("CounterSteerIntensity : %f"), CounterSteerIntensity));
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("AdjustedSteering : %f"), AdjustedSteering));
}

void AIcePawn::Vehicle(float Delta)
{
	// Physic Drift
	float sttR0 = 0.0f;
	float sttR1 = 0.0f;

	if (ChaosVehicleMovement->Wheels.Num() > 1) 
	{
		UChaosVehicleWheel* FrontLeftWheel = ChaosVehicleMovement->Wheels[0]; 
		UChaosVehicleWheel* FrontRightWheel = ChaosVehicleMovement->Wheels[1]; 
	
		if (FrontLeftWheel)
		{
			sttR0 = FrontLeftWheel->GetSteerAngle(); 
		}
	
		if (FrontRightWheel)
		{
			sttR1 = FrontRightWheel->GetSteerAngle(); 
		}
	}
	float steerAngle = (sttR0 + sttR1) / 2.0f;
	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("SteerAngle: %f"),steerAngle));
	SteeringAngle = steerAngle;

	// ChaosVehiclePowerCutFix
	float targetGear = ChaosVehicleMovement->GetTargetGear();
	bool isTorqueCombineActive = (targetGear == 1);

	if (isTorqueCombineActive)
	{
		UChaosVehicleWheel* RearLeftWheel = ChaosVehicleMovement->Wheels[2];
		UChaosVehicleWheel* RearRightWheel = ChaosVehicleMovement->Wheels[3];
	}

	float TorqueMultiplier = 40.0f;

	float GearTorque[] = {400.0f, 310.0f, 240.0f, 180.0f, 100.0f, 50.0f};
	float ReverseGearTorque = 100.0f; 

	int32 CurrentGear = ChaosVehicleMovement->GetTargetGear();
	float TorqueValue = ThrottleValue * TorqueMultiplier;

	if (CurrentGear > 0 && CurrentGear <= 6)  
	{
		TorqueValue *= GearTorque[CurrentGear - 1];
		ChaosVehicleMovement->SetDriveTorque(TorqueValue, 2);
		ChaosVehicleMovement->SetDriveTorque(TorqueValue, 3);
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("Gear %d: Torque: %f"), CurrentGear, TorqueValue));
	}
	else if (CurrentGear == -1) 
	{
		TorqueValue *= ReverseGearTorque;
		ChaosVehicleMovement->SetDriveTorque(TorqueValue, 2);
		ChaosVehicleMovement->SetDriveTorque(TorqueValue, 3);
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("Reverse Gear: Torque: %f"), TorqueValue));
	}
	else  
	{
		ChaosVehicleMovement->SetDriveTorque(0.0f, 2);
		ChaosVehicleMovement->SetDriveTorque(0.0f, 3);
		GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Red, TEXT("Neutral or Invalid Gear"));
	}

	// if (!bIsClutchEngaged)
	// {
	// 	float ThrottleInput = ChaosVehicleMovement->GetThrottleInput();
	// 	float TargetRPM = FMath::Lerp(EngineIdleRPM, MaxEngineRPM, ThrottleInput);
	// 	RPMVehicleValue = FMath::FInterpTo(RPMVehicleValue, TargetRPM, Delta, 2.0f);
	// }
	// else
	// {
	// 	float WheelSpeed = SpeedKPHValue;
	// 	RPMVehicleValue = WheelSpeed * ChaosVehicleMovement->GetCurrentGear() * 60.0f;
	// }
	// RPMVehicleValue = FMath::Clamp(RPMVehicleValue, EngineIdleRPM, MaxEngineRPM);

}

void AIcePawn::Engine(float Delta)
{
}

void AIcePawn::Throttle(const FInputActionValue& Value)
{
	ThrottleValue = Value.Get<float>();
	ChaosVehicleMovement->SetThrottleInput(ThrottleValue);
}

void AIcePawn::Brake(const FInputActionValue& Value)
{
	float BreakValue = Value.Get<float>();
	ChaosVehicleMovement->SetBrakeInput(BreakValue);
}

void AIcePawn::Clutch(const FInputActionValue& Value)
{
	float ClutchBitePoint = 0.2f;

	float ClutchFullEngagementPoint = 0.8f;

	float ClutchDragTorque = 50.0f;

	float ClutchEngagementRate = 5.0f;

	float CurrentClutchEngagement = 1.0f;

    float ClutchValue = Value.Get<float>();
    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("Clutch Value: %f"), ClutchValue));

    // Hitung tingkat keterlibatan kopling
    float TargetEngagement;
    if (ClutchValue < ClutchBitePoint)
    {
        TargetEngagement = 0.0f;
    }
    else if (ClutchValue > ClutchFullEngagementPoint)
    {
        TargetEngagement = 1.0f;
    }
    else
    {
        TargetEngagement = (ClutchValue - ClutchBitePoint) / (ClutchFullEngagementPoint - ClutchBitePoint);
    }

    // Perhalus pergerakan kopling
    float DeltaTime = GetWorld()->GetDeltaSeconds();
    CurrentClutchEngagement = FMath::FInterpTo(CurrentClutchEngagement, TargetEngagement, DeltaTime, ClutchEngagementRate);

    // Hitung slip kopling
    float EngineSpeed = RPMVehicleValue;
    float WheelSpeed = SpeedKPHValue * ChaosVehicleMovement->GetCurrentGear() * 60.0f;
    float SlipSpeed = EngineSpeed - WheelSpeed;

    // Hitung torsi kopling
    float ClutchTorque = CurrentClutchEngagement * SlipSpeed * ClutchDragTorque;

    // Terapkan efek kopling pada RPM mesin
    if (CurrentClutchEngagement < 1.0f)
    {
        bIsClutchEngaged = false;
        
        float ThrottleInput = ChaosVehicleMovement->GetThrottleInput();
        float TargetRPM = FMath::Lerp(EngineIdleRPM, MaxEngineRPM, ThrottleInput);
        
        // Gunakan ClutchTorque untuk mempengaruhi perubahan RPM
        float RPMChange = (TargetRPM - RPMVehicleValue) * DeltaTime * 2.0f - ClutchTorque * DeltaTime;
        RPMVehicleValue += RPMChange;
    }
    else
    {
        bIsClutchEngaged = true;
        RPMVehicleValue = WheelSpeed;
    }

    // Batasi RPM agar tetap dalam rentang yang valid
    RPMVehicleValue = FMath::Clamp(RPMVehicleValue, EngineIdleRPM, MaxEngineRPM);

    // Terapkan efek kopling pada input throttle
    float AdjustedThrottle = ThrottleValue * CurrentClutchEngagement;
    ChaosVehicleMovement->SetThrottleInput(AdjustedThrottle);

    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Yellow, FString::Printf(TEXT("Clutch Engagement: %f"), CurrentClutchEngagement));
    GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Orange, FString::Printf(TEXT("Clutch Torque: %f"), ClutchTorque));
}

// void AIcePawn::Clutch(const FInputActionValue& Value)
// {
// 	float ClutchValue = Value.Get<float>();
// 	GEngine->AddOnScreenDebugMessage(-1, 0.0f, FColor::Green, FString::Printf(TEXT("Clutch Value: %f"), ClutchValue));
// 	
// 	if (ClutchValue < ClutchEngagementThreshold)
// 	{
// 		bIsClutchEngaged = false;
//         
// 		float SlipFactor = FMath::Lerp(1.0f, ClutchSlipFactor, ClutchValue / ClutchEngagementThreshold);
// 		float WheelSpeed = SpeedKPHValue;
// 		float TargetEngineRPM = WheelSpeed * ChaosVehicleMovement->GetCurrentGear() * 60.0f;
//         
// 		RPMVehicleValue = FMath::Lerp(RPMVehicleValue, TargetEngineRPM, SlipFactor);
// 	}
// 	else
// 	{
// 		bIsClutchEngaged = true;
// 		ChaosVehicleMovement->SetThrottleInput(0.0f);
// 	}
// }

void AIcePawn::GearUp(const FInputActionValue& Value)
{
	if (ChaosVehicleMovement)
	{
		int32 CurrentGear = ChaosVehicleMovement->GetCurrentGear();

		if (CurrentGear < MaxGear - 1)
		{
			int32 TargetGear = CurrentGear + 1;
			ChaosVehicleMovement->SetTargetGear(TargetGear, true);
		}
	}
}

void AIcePawn::GearDown(const FInputActionValue& Value)
{
	if (ChaosVehicleMovement)
	{
		int32 CurrentGear = ChaosVehicleMovement->GetCurrentGear();

		if (CurrentGear > 0) 
		{
			int32 TargetGear = CurrentGear - 1;
			ChaosVehicleMovement->SetTargetGear(TargetGear, true);
		}
	}
}


void AIcePawn::StartHandbrake(const FInputActionValue& Value)
{
	ChaosVehicleMovement->SetHandbrakeInput(true);

}

void AIcePawn::StopHandbrake(const FInputActionValue& Value)
{
	ChaosVehicleMovement->SetHandbrakeInput(false);
}

void AIcePawn::LookAround(const FInputActionValue& Value)
{
	// get the flat angle value for the input 
	float LookValue = Value.Get<float>();

	// add the input
	BackSpringArm->AddLocalRotation(FRotator(0.0f, LookValue, 0.0f));
}

void AIcePawn::ToggleCamera(const FInputActionValue& Value)
{
	// toggle the active camera flag
	bFrontCameraActive = !bFrontCameraActive;

	FrontCamera->SetActive(bFrontCameraActive);
	BackCamera->SetActive(!bFrontCameraActive);
}

void AIcePawn::ResetVehicle(const FInputActionValue& Value)
{
	// reset to a location slightly above our current one
	FVector ResetLocation = GetActorLocation() + FVector(0.0f, 0.0f, 50.0f);

	// reset to our yaw. Ignore pitch and roll
	FRotator ResetRotation = GetActorRotation();
	ResetRotation.Pitch = 0.0f;
	ResetRotation.Roll = 0.0f;
	
	// teleport the actor to the reset spot and reset physics
	SetActorTransform(FTransform(ResetRotation, ResetLocation, FVector::OneVector), false, nullptr, ETeleportType::TeleportPhysics);

	GetMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	GetMesh()->SetPhysicsLinearVelocity(FVector::ZeroVector);

	UE_LOG(LogTemplateVehicle, Error, TEXT("Reset Vehicle"));
}

#undef LOCTEXT_NAMESPACE