// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

AWeapon::AWeapon() :
	bFalling{ false },
	ThrowWeaponTime{ 0.7f },
	Ammo(30),
	MagazineCapacity(30),
	WeaponType{ EWeaponType::EWT_SubmachineGun },
	AmmoType(EAmmoType::EAT_9mm),
	ReloadMontageSection(FName(TEXT("Reload SMG"))),
	ClipBoneName(TEXT("smg_clip"))
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

void AWeapon::DecrementAmmo()
{
	if (Ammo - 1 <= 0)
	{
		Ammo = 0;
	}
	else
	{
		--Ammo;
	}
}

void AWeapon::ReloadWeapon(int32 Amount)
{
	checkf(Ammo + Amount <= MagazineCapacity, TEXT("Attempted to reload with more than magazine capacity!"));
	Ammo += Amount;
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
}
