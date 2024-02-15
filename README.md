# PhysicsCourse

## Controls

- WASD - Movement
- Right Mouse Click (Hold) - Swing
- R - Restart Level
- ESC - Quit

## How it Started

### Implementing a Grappling Hook

The project began with the goal of implementing a grappling hook mechanic. While grappling hooks are typically seen in third-person view, I wanted to create a unique first-person experience for this physics course. During the initial development stages, the mechanic was clunky and lacked realistic movement.

As seen in the GIF below, the movement is clunky.

![First grapple](https://github.com/AlenForko/PhysicsCourse/blob/main/GIFS/Grapple.gif)

### Utilizing the UCableComponent

To tackle this challenge, I decided to utilize Unreal Engine's built-in cable component, **_UCableComponent_**. This component allowed me to attach the player to the GrabPoint and simulate the retracting motion necessary for a grappling mechanic.

### Custom Grappling Hook Component

To streamline the implementation and maintain clean code architecture, I created a separate scene component called the "Grappling Hook". This component handles the attachment of the grappling hook to the player and encapsulates all calculations related to the grappling hook mechanics. By isolating these calculations within the Grappling Hook component, the player script remains clean and focused on player movement.

### Transition to Swinging Mechanic

As development progressed, I experimented with expanding the grappling hook mechanic into a swinging mechanic. This transition involved significant adjustments to the physics and mechanics underlying the grappling hook implementation. Ultimately, I decided to pivot towards a swinging mechanic, abandoning the traditional grappling hook approach. This decision led to a more fluid and dynamic gameplay experience, aligning better with the course.

![Swing](https://github.com/AlenForko/PhysicsCourse/blob/main/GIFS/Swing.gif)

# Explanation of Swinging Mechanic Code

To understand how the swinging mechanic works, let's break down the relevant parts of the `UGrapplingHook` class:

## Grapple Function
>[!IMPORTANT]
>- This function is responsible for initiating the grappling action when the player activates the grappling hook.
>- It takes two parameters, Start and End, which represent the start and end points of the grapple.
>- It first checks if the owner character exists, ensuring that the grappling hook is attached to a valid character.
>- Using SweepSingleByChannel, it checks for collisions between the start and end points to find a valid hit result.
>- If a hit occurs (bHasHit is true) and the grappling hook is not already engaged (bIsGrappling is false), and if the player's location is below the hit object (to prevent grappling from above), it engages the grappling hook.
>- It sets the GrabPoint to the impact point of the hit result and makes the cable component visible to simulate the grappling action.
>- Finally, it converts the GrabPoint from local space to world space and updates the end location of the cable component accordingly.
<details>
<summary>CLICK TO REVEAL CODE -----></summary>

```c++
// Explanation of what this function does and how it's used
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
```
</details>

## EndGrapple Function

>[!IMPORTANT]
>- This function is responsible for ending the grappling action and resetting the grappling hook state.
>- It checks if the grappling hook is currently engaged (bIsGrappling is true).
>- If the grappling hook is engaged, it sets bIsGrappling to false to indicate that the grappling action has ended.
>- It also hides the cable component by setting its visibility to false, indicating that the grappling hook is no longer active.

<details>
<summary>CLICK TO REVEAL CODE -----></summary>


```c++
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
```
</details>

## Applying Swing Force Function

>[!IMPORTANT]
>- This function applies the swinging force to the player character when the grappling hook is engaged.
>- It first checks if the grappling hook is engaged (bIsGrappling is true).
>- If the player is above the grab point, indicating that they have swung past it, the grappling action is ended by calling the EndGrapple function.
>- It retrieves the player's velocity and location to calculate the swinging force.
>- The velocity is clamped to a specified range to ensure smooth swinging motion and speed.
>- The direction from the player to the grab point is calculated.
>- The dot product of the clamped velocity and the direction vector is calculated to determine the magnitude of the swinging force.
>- The direction vector is normalized to ensure consistent force application.
>- The force to apply for swinging is calculated and applied to the player character's movement.
>- Air control is adjusted to improve swinging control.

<details>
<summary>CLICK TO REVEAL CODE -----></summary>


```c++
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
	
    // Calculate the force to apply for swinging and negate the gravity so the player doesn't fall
    FVector Force = Dir * DotProduct * -2.f;

    // Apply the calculated force to the player character's movement
    OwnerCharacter->GetCharacterMovement()->AddForce(Force + ForwardForce());

    // Adjust air control to improve swinging control
    OwnerCharacter->GetCharacterMovement()->AirControl = 2.f;
}
```
</details>

## Forward Force Function

>[!IMPORTANT]
>- This function calculates the forward force to apply to the player character during swinging.
>- It retrieves the forward vector of the camera attached to the player character.
>- It extracts the forward component from the camera forward vector and normalizes it to ensure consistent force application.
>- The forward force to apply is calculated based on the normalized forward vector and a specified multiplier (in this case, 200000).
>- The calculated forward force is returned for use in the swinging mechanic.

<details>
<summary>CLICK TO REVEAL CODE -----></summary>


```c++
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
```

</details>

# Finished mechanic

With what I have done, I feel very happy with how the swing turned out. Of course there is always room for improvement and detailing, and I plan on improving this mechanic further and hopefully have it ready to be added into another game project.

Here is the finished mechanic... for now!

![LastSwing](https://github.com/AlenForko/PhysicsCourse/blob/main/GIFS/EndSwing.gif)
