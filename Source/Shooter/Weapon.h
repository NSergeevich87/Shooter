// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Weapon.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;
	AWeapon();

private:
	FTimerHandle TrowWeaponTimer;
	float TrowWeaponTime;
	bool bFalling;

protected:
	void StopFalling();

public:
	// Add an impulse to the Weapon
	void ThrowWeapon();
};
