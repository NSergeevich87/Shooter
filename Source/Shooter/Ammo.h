// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "AmmoType.h"
#include "Ammo.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API AAmmo : public AItem
{
	GENERATED_BODY()
	
public:
	AAmmo();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	/**
	 * Override of SetItemProperties so we can set AmmoMesh properties
	 * @param State 
	*/
	virtual void SetItemProperties(EItemState State) override;

	UFUNCTION()
	void AmmoSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
		);

	virtual void InitializeCustomDepth() override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))	
	UStaticMeshComponent* AmmoMesh{};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	UTexture2D* AmmoTypeTexture;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Ammo, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AmmoCollisionSphere;

public:
	FORCEINLINE UStaticMeshComponent* GetAmmoMesh() const { return AmmoMesh; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }

	virtual void EnableCustomDepth() override;
	virtual void DisableCustomDepth() override;
};
