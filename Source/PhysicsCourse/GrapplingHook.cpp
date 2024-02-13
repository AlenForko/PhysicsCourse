#include "GrapplingHook.h"
#include "CableComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UGrapplingHook::UGrapplingHook()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	CableComponent = CreateDefaultSubobject<UCableComponent>("Cable Component");
	CableComponent->SetVisibility(false);
}

void UGrapplingHook::BeginPlay()
{
	Super::BeginPlay();

	//Get the owner class.
	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UGrapplingHook::Grapple(FVector Start, FVector End)
{
	if (!OwnerCharacter) return;
	
	FHitResult HitResult;
	bool bHasHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel2, FCollisionShape::MakeSphere(10.f));

	if(bHasHit && !bIsGrappling)
	{
		bIsGrappling = true;
		GrabPoint = HitResult.ImpactPoint;
		CableComponent->SetVisibility(true);
	}
	// Convert the GrabPoint from local to world space
	CableComponent->EndLocation = OwnerCharacter->GetActorTransform().InverseTransformPosition(GrabPoint);
}


void UGrapplingHook::EndGrapple()
{
	if(bIsGrappling)
	{
		bIsGrappling = false;
		CableComponent->SetVisibility(false);
	}
}

void UGrapplingHook::ApplySwingForce()
{
	if(!bIsGrappling) return;
	
	FVector Velocity = OwnerCharacter->GetVelocity();
	FVector CharacterLocation = OwnerCharacter->GetActorLocation();

	FVector Dir = CharacterLocation - GrabPoint;

	float DotProduct = FVector::DotProduct(Velocity, Dir);
	
	FVector Force = (Dir.GetSafeNormal() * DotProduct) * -2.f;
	
	OwnerCharacter->GetCharacterMovement()->AddForce(Force);
	OwnerCharacter->GetCharacterMovement()->AirControl = 2.f;
	
}