// Copyright Epic Games, Inc. All Rights Reserved.

#include "PhysicsCourseCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "CableComponent.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

//////////////////////////////////////////////////////////////////////////
// APhysicsCourseCharacter

APhysicsCourseCharacter::APhysicsCourseCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;
	
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
		
	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));

	//Create a cable component and disable the visibility at start
	CableComponent = CreateDefaultSubobject<UCableComponent>("GrapplingCable");
	CableComponent->SetupAttachment(FirstPersonCameraComponent);
	CableComponent->SetVisibility(false);
}

void APhysicsCourseCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

}

void APhysicsCourseCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if(isGrappling)
	{
		CableComponent->EndLocation = GetActorTransform().InverseTransformPosition(GrabPoint);
		GetCharacterMovement()->AddForce((GrabPoint - GetActorLocation()).GetSafeNormal() * 100000.f);
	}
}

//////////////////////////////////////////////////////////////////////////// Input

void APhysicsCourseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APhysicsCourseCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APhysicsCourseCharacter::Look);

		//Grapple
		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Triggered, this, &APhysicsCourseCharacter::Grapple);
		EnhancedInputComponent->BindAction(GrappleAction, ETriggerEvent::Completed, this, &APhysicsCourseCharacter::EndGrapple);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void APhysicsCourseCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void APhysicsCourseCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APhysicsCourseCharacter::Grapple()
{
	FVector Start = GetCapsuleComponent()->GetComponentLocation();
	FVector End = Start + GrapplingLineDistance * UKismetMathLibrary::GetForwardVector(GetFirstPersonCameraComponent()->GetComponentRotation());

	FHitResult HitResult;

	bool HasHit = GetWorld()->SweepSingleByChannel(HitResult, Start, End, FQuat::Identity, ECC_GameTraceChannel2, FCollisionShape::MakeSphere(100.f));

	if(HasHit)
	{
		isGrappling = true;
		GetCharacterMovement()->SetMovementMode(MOVE_Flying);
		GrabPoint = HitResult.ImpactPoint;
		CableComponent->SetVisibility(true);

		CableDistance = End.Length();
	}
}

void APhysicsCourseCharacter::EndGrapple()
{
	isGrappling = false;
	if(!GetCharacterMovement()->IsFalling())
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Falling);
		CableComponent->SetVisibility(false);
	}
}

void APhysicsCourseCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool APhysicsCourseCharacter::GetHasRifle()
{
	return bHasRifle;
}