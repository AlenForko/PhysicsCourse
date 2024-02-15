// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CableComponent.h"
#include "GameFramework/Actor.h"
#include "GrapplingHook.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PHYSICSCOURSE_API UGrapplingHook : public USceneComponent
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	UGrapplingHook();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = GrapplingHook, meta=(AllowPrivateAccess = "true"))
	UCableComponent* CableComponent;
	
	void Grapple(FVector Start, FVector End);

	void EndGrapple();

	void ApplySwingForce();

	FVector ForwardForce() const;

	UPROPERTY(EditAnywhere)
	float ForceMultiplier = 200000;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	float NewCableLength;
	
	bool bIsGrappling = false;
	
	FVector GrabPoint;

	UPROPERTY()
	ACharacter* OwnerCharacter;
	
	float VelocityClampMin = 400;
	float VelocityClampMax = 2000;
};
