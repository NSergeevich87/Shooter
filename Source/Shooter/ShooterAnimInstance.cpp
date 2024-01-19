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
	TIPCharacterYaw(0.f),
	TIPCharacterYawLastFrame(0.f),
	RootYawOffset(0.f),
	Pitch(0.f),
	bReloading(false),
	OffsetState(EOffsetState::EOS_Hip),
	CharacterRotation(FRotator(0.f)),
	CharacterRotationLastFrame(FRotator(0.f)),
	YawDelta(0.f),
	RecoilWeight(1.0f)
{
}

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}

	if (ShooterCharacter)
	{
		bCrouch = ShooterCharacter->GetCrouching();
		bReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;

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

		if (ShooterCharacter->GetAimingStatus())
		{
			OffsetState = EOffsetState::EOS_Aiming;
		}
		else if (bIsInAir)
		{
			OffsetState = EOffsetState::EOS_InAir;
		}
		else if (bReloading)
		{
			OffsetState = EOffsetState::EOS_Reloading;
		}
		else
		{
			OffsetState = EOffsetState::EOS_Hip;
		}
	}

	TurnInPlace();
	Lean(DeltaTime);
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;

	if (Speed > 0 || bIsInAir)
	{
		// Don't want to turn in place; Character is moving
		RootYawOffset = 0.f;

		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TIPCharacterYawLastFrame = TIPCharacterYaw;

		RotationCurve = 0.f;
		RotationCurveLastFrame = 0.f;
	}
	else
	{
		TIPCharacterYawLastFrame = TIPCharacterYaw;
		TIPCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		const float TIPYawDelta = TIPCharacterYaw - TIPCharacterYawLastFrame;

		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - TIPYawDelta);

		const float Turning = GetCurveValue(TEXT("Turning"));

		if (Turning > 0)
		{
			bTurningInPlace = true;

			RotationCurveLastFrame = RotationCurve;
			RotationCurve = GetCurveValue(TEXT("Rotation"));

			const float DeltaRotation = RotationCurve - RotationCurveLastFrame;

			// RootYawOffset > 0 -> turn_Left; RootYawOffset < 0 -> turn_Right
			RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

			const float ABSRootYawOffset = FMath::Abs(RootYawOffset);

			if (ABSRootYawOffset > 90.f)
			{
				const float YawExcess = ABSRootYawOffset - 90.f;
				RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
			}
		}
		else
		{
			bTurningInPlace = false;
		}
	}

	if (bTurningInPlace)
	{
		if (bReloading)
		{
			RecoilWeight = 1.0f;
		}
		else
		{
			RecoilWeight = 0.f;
		}
	}
	else
	{
		if (bCrouch)
		{
			if (bReloading)
			{
				RecoilWeight = 1.0f;
			}
			else
			{
				RecoilWeight = 0.1f;
			}
		}
		else
		{
			if (isAiming || bReloading)
			{
				RecoilWeight = 1.0f;
			}
			else
			{
				RecoilWeight = 0.5f;
			}
		}
	}
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if (ShooterCharacter == nullptr) return;

	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = ShooterCharacter->GetActorRotation();
	const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);

	const double Target{ DeltaRotation.Yaw / DeltaTime };
	const double Interp { FMath::FInterpTo(YawDelta, Target, DeltaTime, 6.f) };
	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);

	if (GEngine) GEngine->AddOnScreenDebugMessage(
		2,
		-1,
		FColor::Cyan,
		FString::Printf(TEXT("YawDelta: %f"), YawDelta)
	);

	if (GEngine) GEngine->AddOnScreenDebugMessage(
		2, 
		-1, 
		FColor::Emerald, 
		FString::Printf(TEXT("DeltaYaw: %f"), DeltaRotation.Yaw));
}
