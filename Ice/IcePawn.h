// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "IcePawn.generated.h"

// Forward declarations
class UCameraComponent;
class USpringArmComponent;
class UInputAction;
class UChaosWheeledVehicleMovementComponent;
class UChaosVehicleWheel;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateVehicle, Log, All);

/**
 *  Vehicle Pawn class
 *  Handles common functionality for all vehicle types,
 *  including input handling and camera management.
 *  
 *  Specific vehicle configurations are handled in subclasses.
 */
UCLASS(abstract)
class AIcePawn : public AWheeledVehiclePawn
{
    GENERATED_BODY()

public:
    AIcePawn();

    virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
    virtual void Tick(float Delta) override;

    void Vehicle(float Delta);
    void Engine(float Delta);
    void DriftAssist(float Delta);

    FORCEINLINE USpringArmComponent* GetFrontSpringArm() const { return FrontSpringArm; }
    FORCEINLINE UCameraComponent* GetFrontCamera() const { return FrontCamera; }
    FORCEINLINE USpringArmComponent* GetBackSpringArm() const { return BackSpringArm; }
    FORCEINLINE UCameraComponent* GetBackCamera() const { return BackCamera; }
    FORCEINLINE const TObjectPtr<UChaosWheeledVehicleMovementComponent>& GetChaosVehicleMovement() const { return ChaosVehicleMovement; }

protected:
    void Steering(const FInputActionValue& Value);
    void Throttle(const FInputActionValue& Value);
    void Brake(const FInputActionValue& Value);
    void Clutch(const FInputActionValue& Value);
    void GearUp(const FInputActionValue& Value);
    void GearDown(const FInputActionValue& Value);
    void StartBrake(const FInputActionValue& Value);
    void StopBrake(const FInputActionValue& Value);
    void StartHandbrake(const FInputActionValue& Value);
    void StopHandbrake(const FInputActionValue& Value);
    void LookAround(const FInputActionValue& Value);
    void ToggleCamera(const FInputActionValue& Value);
    void ResetVehicle(const FInputActionValue& Value);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* FrontSpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* FrontCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    USpringArmComponent* BackSpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    UCameraComponent* BackCamera;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Vehicle, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* SteeringAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* ThrottleAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* BrakeAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* ClutchAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* GearUpAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* GearDownAction;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* HandbrakeAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* LookAroundAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* ToggleCameraAction;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
    UInputAction* ResetVehicleAction;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
    float MouseSensitivity;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Steering")
    float MouseSteeringValue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
    bool bShowDebugMessages;

private:
    bool bFrontCameraActive;
    float CurrentSteerAngle;
    float PreviousSteeringInput;
    float DriftR;
    float SteeringAngle;
    float SlipAngleRatio;
    float InSlipAngle;
    float DiffR;
    float SlipAngle;
    float SteerForce;
    float SteerMain;

    UPROPERTY(VisibleAnywhere, Category = "Steering")
    float SteeringValue;

    UPROPERTY(VisibleAnywhere, Category = "Engine")
    float RPMVehicleValue;

    UPROPERTY(VisibleAnywhere, Category = "Engine")
    float ThrottleValue;

    UPROPERTY(VisibleAnywhere, Category = "Engine")
    float SpeedKPHValue;

    UPROPERTY(VisibleAnywhere, Category = "Engine")
    float MaxEngineRPM;

    UPROPERTY(VisibleAnywhere, Category = "Engine")
    float EngineIdleRPM;
    
    UPROPERTY(VisibleAnywhere, Category = "Vehicle")
    int32 GearCurrent;

    UPROPERTY(VisibleAnywhere, Category = "Vehicle")
    int32 MaxGear;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float ClutchEngagementThreshold;

    UPROPERTY(EditAnywhere, Category = "Vehicle")
    float ClutchSlipFactor;

    UPROPERTY(VisibleAnywhere, Category = "Vehicle")
    bool bIsClutchEngaged;

    int32 DebugMessageId;
};