// Copyright Epic Games, Inc. All Rights Reserved.

#include "BossRushGameMode.h"
#include "BossRushCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABossRushGameMode::ABossRushGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
