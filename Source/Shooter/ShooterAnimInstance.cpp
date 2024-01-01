// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() : 
	Speed(0.f),
	bIsInAir(false),
	bIsAccelerating(false),
	offsetYawRotation(0.f),
	lastOffsetYawRotation(0.f),
	isAiming(false),
	CharacterYaw(0.f),
	CharacterYawLastFrame(0.f),
	RootYawOffset(0.f)
{
}

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

		isAiming = ShooterCharacter->GetAimingStatus();
	}

	TurnInPlace();
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;
	if (Speed > 0)
	{
		// Don't want to turn in place; Character is moving
	}
	else
	{
		CharacterYawLastFrame = CharacterYaw;
		CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float YawDelta = CharacterYaw - CharacterYawLastFrame;

		RootYawOffset -= YawDelta;

		if (GEngine) GEngine->AddOnScreenDebugMessage(
			1,
			-1,
			FColor::Blue,
			FString::Printf(TEXT("Character Yaw: %f"), CharacterYaw)
		);

		if (GEngine) GEngine->AddOnScreenDebugMessage(
			2,
			-1,
			FColor::Red,
			FString::Printf(TEXT("Root Yaw Offset: %f"), RootYawOffset)
		);
	}
}
