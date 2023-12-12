// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnMouseRight(float Rate); //Метод чувствительности для мыши
	void TurnMouseUp(float Rate); //Метод чувствительности для мыши
	void TerningAtUp(float Rate); //Метод чувствительности для джойстика или стрелок на клавиатуре
	void TerningAtRight(float Rate); //Метод чувствительности для джойстика или стрелок на клавиатуре
	void FireWeapon();
	
	void FireButtonPressed();
	void FireButtonReleased();
	void StartFireTimer();
	UFUNCTION()
	void AutoFireReset();

	bool GetBeamEndLocation(const FVector& MuzzelSocketLocation, FVector& OutBeamLocation);
	void ZoomCameraPressed();
	void ZoomCameraReleased();
	void CameraInterpZoom(float DeltaTime);
	void SetLookSence(); //Метод чувствительности для джойстика или стрелок на клавиатуре
	void CalculateCrosshairSpread(float DeltaTime); //Метод расчета кроссхаир спреда
	//Calculate methods of CrosshairShootFactor
	void isCrosshairShooting();
	UFUNCTION()
	void notCrosshairShooting();

	//метод для получения информации с чем пересекается кроссхаир
	bool GetInfoCrosshair(FHitResult &hitResult, FVector &OutHitLocation);

	void TraceForItems();

	class AWeapon* SpawnDefaultWeapon();

	void EquipWeapon(AWeapon* EquipWeapon);

	void DropWeapon();
	void SelectButtonPressed();
	void SelectButtonReleased();

	void SwapWeapon(AWeapon* WeaponToSwap);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float TernRightRate{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float TernUpRate{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipTernSenceRate{};
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float HipUpSenceRate{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipUpSenceRate{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseHipTurnSenceRate{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimUpSenceRate{};

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"), meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float MouseAimTurnSenceRate{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimTernSenceRate{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	float AimUpSenceRate{};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class USoundCue* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UParticleSystem* ParticleShot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage* WeaponFire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* InpactParticle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	UParticleSystem* BeamParticle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Combat, meta = (AllowPrivateAccess = "true"))
	bool isAiming{};

	float BaseCameraView{};
	float ZoomCameraView{};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowPrivateAccess = "true"))
	float ZoomCameraSpeed{};

	float NormalCameraView{};

	//Crosshair spread variables
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpreadMultiplier{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairSpeedFactor{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairInAirFactor{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairAimFactor{};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Crosshairs, meta = (AllowPrivateAccess = "true"))
	float CrosshairShootFactor{};

	float ShootingTime{};
	FTimerHandle CrosshairShootTimerHandle;
	bool isFire{};

	bool isFireButtonPressed{};
	bool isShouldFire{};
	float AutomaticFireRate{};
	FTimerHandle ShootingTimerHandle;

	bool isShouldTraceForItems{};
	int8 OverlapedItemCount;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = Items, meta = (AllowPrivateAccess = "true"))
	class AItem* LastFrameTraceHitItem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = Combat, meta = (AllowPrivateAccess = "true"))
	AWeapon* EquiptedWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, category = Combat, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<AWeapon> DefaultWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = Combat, meta = (AllowPrivateAccess = "true"))
	AItem* HitItem;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpDistance;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, category = Items, meta = (AllowPrivateAccess = "true"))
	float CameraInterpElevation;

	/*UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, category = Items, meta = (AllowPrivateAccess = "true"))
	class UCurveFloat* ItemZCurve;  */         

public:
	FORCEINLINE USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE bool GetAimingStatus() const { return isAiming; }

	UFUNCTION(BlueprintCallable)
	float GetCrosshairSpreadMultiplier() const;

	FORCEINLINE int8 GetOverlapedItemCount() const { return OverlapedItemCount; }
	void IncrementOverlapedItemCount(int8 Amount);

	FVector GetCameraInterpLocation();

	void GetPickupItem(AItem* Item);
};
