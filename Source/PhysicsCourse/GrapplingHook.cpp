#include "GrapplingHook.h"
#include "CableComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UGrapplingHook::UGrapplingHook()
{
	PrimaryComponentTick.bCanEverTick = true;
	
	CableComponent = CreateDefaultSubobject<UCableComponent>("Cable Component");
	CableComponent->SetVisibility(false);
}

void UGrapplingHook::BeginPlay()
{
	Super::BeginPlay();

	//Get the owner class.
	OwnerCharacter = Cast<ACharacter>(GetOwner());
}

void UGrapplingHook::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ApplySwingForce();
}

void UGrapplingHook::Grapple(FVector Start, FVector End)
{
	if (!OwnerCharacter) return;
	
	FHitResult HitResult;
	bool bHasHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel2, FCollisionShape::MakeSphere(10.f));

	if(bHasHit && !bIsGrappling && OwnerCharacter->GetActorLocation().Z < HitResult.GetActor()->GetActorLocation().Z)
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

	// Get out of the swing if above the grab point
	if (bIsGrappling && OwnerCharacter->GetActorLocation().Z >= GrabPoint.Z)
	{
		EndGrapple();
	}
	
	FVector Velocity = OwnerCharacter->GetVelocity();
	FVector CharacterLocation = OwnerCharacter->GetActorLocation();
	
	FVector ClampedVelocity = Velocity.GetClampedToSize(VelocityClampMin, VelocityClampMax);
	
	FVector Dir = CharacterLocation - GrabPoint;
	
	float DotProduct = FVector::DotProduct(ClampedVelocity, Dir);

	Dir.Normalize(0.0001);
	
	FVector Force = Dir * DotProduct * -2.f;

	OwnerCharacter->GetCharacterMovement()->AddForce(Force + ForwardForce());
	OwnerCharacter->GetCharacterMovement()->AirControl = 2.f;
}

FVector UGrapplingHook::ForwardForce() const
{
	FVector CameraForwardVector = OwnerCharacter->GetActorForwardVector();

	FVector Forward = FVector(CameraForwardVector.X, CameraForwardVector.Y, 0);
	Forward.Normalize(0.0001);

	// Tweak the number based on how much force to add in the forward direction.
	FVector ForwardForce = Forward * 200000;
	
	return ForwardForce;
}
