// Copyright Epic Games, Inc. All Rights Reserved.

#include "IceWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UIceWheelFront::UIceWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 70.f;

	MaxWheelspinRotation = 30.0f;
	WheelRadius = 29.9f; 
	WheelWidth = 26.0f; 
	CorneringStiffness = 250.0f; // Reduced for easier cornering
	FrictionForceMultiplier = 2.5f; // Increased for better control
	SideSlipModifier = 1.0f;
	SkidThreshold = 15.0f; // Lowered for easier skidding
	SuspensionSmoothing = 1.0f;
	SuspensionMaxDrop = 3.0f; // Increased for more body roll
	SuspensionAxis = FVector(0.0f, 0.0f, -1.0f);
}