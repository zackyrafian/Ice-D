// Copyright Epic Games, Inc. All Rights Reserved.


#include "IceUI.h"

void UIceUI::UpdateSpeed(float NewSpeed)
{
	// format the speed to KPH or MPH
	float FormattedSpeed = FMath::Abs(NewSpeed) * (bIsMPH ? 0.022f : 0.036f);

	// call the Blueprint handler
	OnSpeedUpdate(FormattedSpeed);
}

void UIceUI::UpdateRPM(float NewRPM)
{
	OnRPMUpdate(NewRPM);
}

void UIceUI::UpdateGear(int32 NewGear)
{
	OnGearUpdate(NewGear);
}
