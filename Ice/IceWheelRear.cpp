// Copyright Epic Games, Inc. All Rights Reserved.

#include "IceWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UIceWheelRear::UIceWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
	
	MaxWheelspinRotation = 30.0f;
	WheelRadius = 29.9f; 
	WheelWidth = 26.0f;  
	CorneringStiffness = 50.0f; 
	FrictionForceMultiplier = 2.0f;
	SideSlipModifier = 1.0f;
	SkidThreshold = 10.0f; 

	RollbarScaling = 20.0f;
	SuspensionSmoothing = 2.0f;
	SuspensionMaxDrop = 3.0f; // Increased for more body roll
	SuspensionAxis = FVector(0.0f, 0.0f, -1.0f);
}