// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	else
	{
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;
		Speed = Velocity.Size();

		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
		{
			bIsAccelerating = true;
		}
		else
		{
			bIsAccelerating = false;
		}

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MoveRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		offsetYawRotation = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, MoveRotation).Yaw;

		if (ShooterCharacter->GetVelocity().Size() > 0.f)
		{
			lastOffsetYawRotation = offsetYawRotation;
		}

		FString offsetYawRotationnMessage = FString::Printf(TEXT("OffsetYaw is: %f"), offsetYawRotation);
		//FString message = FString::Printf(TEXT("Aim rotation is: %f"), baseAimRotation.Yaw);
		//FString moveMessage = FString::Printf(TEXT("Movement rotation is: %f"), moveAimRotation.Yaw);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, offsetYawRotationnMessage);
		}
	}
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}
