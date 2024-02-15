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
	// Check if the owner character exists
	if (!OwnerCharacter) return;

	// Initialize variables
	FHitResult HitResult;
	bool bHasHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel2, FCollisionShape::MakeSphere(10.f));

	// Check if a hit occurred and if the grappling hook is not already engaged, and if the player is below the hit object
	if(bHasHit && !bIsGrappling && OwnerCharacter->GetActorLocation().Z < HitResult.GetActor()->GetActorLocation().Z)
	{
		// Engage the grappling hook
		bIsGrappling = true;
		GrabPoint = HitResult.ImpactPoint;
		CableComponent->SetVisibility(true);
	}
	
	// Convert the GrabPoint from local to world space
	CableComponent->EndLocation = OwnerCharacter->GetActorTransform().InverseTransformPosition(GrabPoint);
}


void UGrapplingHook::EndGrapple()
{
	// Check if the grappling hook is engaged
	if(bIsGrappling)
	{
		// Disengage the grappling hook
		bIsGrappling = false;
		// Hide the cable component
		CableComponent->SetVisibility(false);
	}
}

void UGrapplingHook::ApplySwingForce()
{	
	// Check if the grappling hook is engaged
	if(!bIsGrappling) return;

	// Get out of the swing if above the grab point
	if (bIsGrappling && OwnerCharacter->GetActorLocation().Z >= GrabPoint.Z)
	{
		EndGrapple();
	}
	
	// Get player's velocity and location
	FVector Velocity = OwnerCharacter->GetVelocity();
	FVector CharacterLocation = OwnerCharacter->GetActorLocation();
	
	// Clamp the velocity to a specified range
	FVector ClampedVelocity = Velocity.GetClampedToSize(VelocityClampMin, VelocityClampMax);
	
	// Calculate the direction from the player to the grab point
	FVector Dir = CharacterLocation - GrabPoint;
	
	// Calculate the dot product of the clamped velocity and the direction vector
	float DotProduct = FVector::DotProduct(ClampedVelocity, Dir);

	// Normalize the direction vector
	Dir.Normalize(0.0001);
	
	// Calculate the force to apply for swinging
	FVector Force = Dir * DotProduct * -2.f;

	// Apply the calculated force to the player character's movement
	OwnerCharacter->GetCharacterMovement()->AddForce(Force + ForwardForce());

	// Adjust air control to improve swinging control
	OwnerCharacter->GetCharacterMovement()->AirControl = 2.f;
}

FVector UGrapplingHook::ForwardForce() const
{
	// Get player forward vector
	FVector PlayerForwardVector = OwnerCharacter->GetActorForwardVector();

	// Extract the forward component from the camera forward vector
	FVector Forward = FVector(PlayerForwardVector.X, PlayerForwardVector.Y, 0);
	
	// Normalize the forward vector
	Forward.Normalize(0.0001);

	// Calculate the forward force to apply 
	FVector ForwardForce = Forward * ForceMultiplier;
	
	return ForwardForce;
}
