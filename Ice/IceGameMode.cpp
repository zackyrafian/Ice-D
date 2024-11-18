// Copyright Epic Games, Inc. All Rights Reserved.

#include "IceGameMode.h"
#include "IcePlayerController.h"

AIceGameMode::AIceGameMode()
{
	PlayerControllerClass = AIcePlayerController::StaticClass();
}
