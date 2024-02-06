// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsCourseGameMode.h"
#include "PhysicsCourseCharacter.h"
#include "UObject/ConstructorHelpers.h"

APhysicsCourseGameMode::APhysicsCourseGameMode()
	: Super()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnClassFinder(TEXT("/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"));
	DefaultPawnClass = PlayerPawnClassFinder.Class;

}
