// Fill out your copyright notice in the Description page of Project Settings.

#include "GrapplingHook.h"
#include "CableComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"


// Sets default values
UGrapplingHook::UGrapplingHook()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryComponentTick.bCanEverTick = true;
	
	CableComponent = CreateDefaultSubobject<UCableComponent>("Cable Component");
	CableComponent->SetVisibility(false);
}

void UGrapplingHook::BeginPlay()
{
	Super::BeginPlay();
	
	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UGrapplingHook::Grapple(FVector Start, FVector End)
{
	if (!OwnerCharacter) return;
	
	FHitResult HitResult;
	bool bHasHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel2, FCollisionShape::MakeSphere(100.f));

	if(bHasHit && !bIsGrappling)
	{
		bIsGrappling = true;
		OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GrabPoint = HitResult.ImpactPoint;
		CableComponent->SetVisibility(true);
	}
	CableComponent->EndLocation = OwnerCharacter->GetActorTransform().InverseTransformPosition(GrabPoint);

	FString GrabPointString = FString::Printf(TEXT("GrabPoint: %s"), *GrabPoint.ToString());
	GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Red, *GrabPointString);
}

void UGrapplingHook::EndGrapple()
{
	if(bIsGrappling)
	{
		bIsGrappling = false;
	
		if(!OwnerCharacter->GetCharacterMovement()->IsFalling())
		{
			OwnerCharacter->GetCharacterMovement()->SetMovementMode(MOVE_Falling);
			CableComponent->SetVisibility(false);
		}
	}
}




