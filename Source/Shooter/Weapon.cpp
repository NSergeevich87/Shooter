// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon() : bFalling{false}, ThrowWeaponTime{0.7f}
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetItemState() == EItemState::EIS_Falling && bFalling)
	{
		const FRotator WeaponRotation{ 0.f, GetSkeletalMesh()->GetComponentRotation().Yaw, 0.f };
		GetSkeletalMesh()->SetWorldRotation(WeaponRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}
}

void AWeapon::ThrowWeapon()
{
	FRotator WeaponRotation{ 0.f, GetSkeletalMesh()->GetComponentRotation().Yaw, 0.f };
	GetSkeletalMesh()->SetWorldRotation(WeaponRotation, false, nullptr, ETeleportType::TeleportPhysics);

	const FVector MeshForward{ GetSkeletalMesh()->GetForwardVector() };
	const FVector MeshRight{ GetSkeletalMesh()->GetRightVector() };

	FVector ImpulseDirection = MeshRight.RotateAngleAxis(-20.f, MeshForward);

	float RandomRotation{ FMath::FRandRange(1.f, 60.f) };

	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector{ 0.f, 0.f, 1.f });
	ImpulseDirection *= 20'000.f;
	GetSkeletalMesh()->AddImpulse(ImpulseDirection);

	bFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
}
