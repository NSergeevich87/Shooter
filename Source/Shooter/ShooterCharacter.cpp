  // Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"
#include "Item.h"
#include "Components/WidgetComponent.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Ammo.h"


// Sets default values
AShooterCharacter::AShooterCharacter() :
	//Чувствительность смещения по осям при помощи мыши
	MouseHipTurnSenceRate(1.0f),
	MouseHipUpSenceRate(1.0f),
	MouseAimTurnSenceRate(0.4f),
	MouseAimUpSenceRate(0.4f),
	//Чувствительность смещения поворота по осям при помощи стрелок или джойстика
	TernRightRate(45.f),
	TernUpRate(45.f),
	isAiming(false),
	//Чувствительность мыши при прицеливании и без
	HipTernSenceRate(90.f),
	HipUpSenceRate(90.f),
	AimTernSenceRate(20.f),
	AimUpSenceRate(20.f),
	//Field of View settings
	BaseCameraView(0.f),
	ZoomCameraView(25.f),
	NormalCameraView(0.f),
	ZoomCameraSpeed(20.f),
	//Crosshair spread factors
	CrosshairSpreadMultiplier(0.f),
	CrosshairSpeedFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	CrosshairShootFactor(0.f),
	//Variables of shooting factor
	isFire(false),
	ShootingTime(0.05f),
	//Автоматическая стрельба
	isShouldFire(true),
	isFireButtonPressed(false),
	AutomaticFireRate(0.1f),
	//Item trace variables
	isShouldTraceForItems(false),
	//CameraInterp Variables
	CameraInterpDistance(250.f),
	CameraInterpElevation(65.f),
	//Starting Ammo amounts
	Starting9mmAmmo(85),
	StartingARAmmo(120),
	//Default Combat State
	CombatState(ECombatState::ECS_Unoccupied),
	bCrouching(false),
	BaseMovementSpeed(650.f),
	CrouchMovementSpeed(300.f),
	StandingCapsuleHalfHight(88.f),
	CrouchingCapsuleHalfHight(44.f),
	BaseFrictionValue(2.f),
	CrouchingFrictionValue(100.f),
	bAimingButtonPressed(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create a camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 220.f; //The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; //Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector{ 0.f, 50.f, 70.f };

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; 

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);

	HandSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("LeftHandComp"));

	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon Interpolation Comp"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());

	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Comp 1"));
	InterpComp1->SetupAttachment(GetFollowCamera());

	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Comp 2"));
	InterpComp2->SetupAttachment(GetFollowCamera());

	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Comp 3"));
	InterpComp3->SetupAttachment(GetFollowCamera());

	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Comp 4"));
	InterpComp4->SetupAttachment(GetFollowCamera());

	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Comp 5"));
	InterpComp5->SetupAttachment(GetFollowCamera());

	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Comp 6"));
	InterpComp6->SetupAttachment(GetFollowCamera());
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	EquipWeapon(SpawnDefaultWeapon());

	if (FollowCamera)
	{
		BaseCameraView = GetFollowCamera()->FieldOfView;
		NormalCameraView = BaseCameraView;
	}

	InitializeAmmoMap();

	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;

	InitializeInterpLocations();
}

void AShooterCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.f))
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.f))
	{
		const FRotator Rotation{ Controller->GetControlRotation() };
		const FRotator YawRotation{ 0, Rotation.Yaw, 0 };

		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::TurnMouseRight(float Rate)
{
	float CurrentSence{};
	if (isAiming)
	{
		CurrentSence = MouseAimTurnSenceRate;
	}
	else
	{
		CurrentSence = MouseHipTurnSenceRate;
	}
	AddControllerYawInput(Rate * CurrentSence);
}

void AShooterCharacter::TurnMouseUp(float Rate)
{
	float CurrentSence{};
	if (isAiming)
	{
		CurrentSence = MouseAimUpSenceRate;
	}
	else
	{
		CurrentSence = MouseHipUpSenceRate;
	}
	AddControllerPitchInput(Rate * CurrentSence);
}

void AShooterCharacter::TerningAtUp(float Rate)
{
	AddControllerPitchInput(Rate * TernUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::TerningAtRight(float Rate)
{
	AddControllerYawInput(Rate * TernRightRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::FireWeapon()
{
	if (EquiptedWeapon == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (WeaponHasAmmo())
	{
		PlayFireSound();
		SendBullet();
		PlayGunFireMontage();
		//Start bullet fire timer for crosshairs
		isCrosshairShooting();
		//Start decrement ammo
		EquiptedWeapon->DecrementAmmo();

		StartFireTimer();
	}
}

void AShooterCharacter::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;

	GetWorldTimerManager().SetTimer(
		ShootingTimerHandle, 
		this, 
		&AShooterCharacter::AutoFireReset, 
		AutomaticFireRate
	);
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (WeaponHasAmmo())
	{
		if (isFireButtonPressed)
		{
			FireWeapon();
		}
	}
	else
	{
		ReloadWeapon();
	}
}

void AShooterCharacter::FireButtonPressed()
{
	isFireButtonPressed = true;
	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	isFireButtonPressed = false;
}

bool AShooterCharacter::GetBeamEndLocation(
	const FVector& MuzzelSocketLocation, 
	FVector& OutBeamLocation)
{
	//Check for crosshair trace hit
	FHitResult CrosshairHitResult;
	bool isCrosshairHit = GetInfoCrosshair(CrosshairHitResult, OutBeamLocation);

	if (isCrosshairHit)
	{
		//tentative beam location - stell need to trace from gun
		OutBeamLocation = CrosshairHitResult.Location;
	}
	else
	{
		//outveamlocation is the end location for the line trace
	}

	// Perform a second trace, this time from the gun barrel
	FHitResult ObjectHit;
	const FVector StartObjectHit{ MuzzelSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzelSocketLocation };
	const FVector EndObjectHit{ MuzzelSocketLocation + StartToEnd * 1.25f };
	GetWorld()->LineTraceSingleByChannel(
		ObjectHit,
		StartObjectHit,
		EndObjectHit,
		ECollisionChannel::ECC_Visibility);

	if (ObjectHit.bBlockingHit)
	{
		//ObjectHit.Location = OutBeamLocation;
		OutBeamLocation = ObjectHit.Location;
		return true;
	}

	return false;

}

void AShooterCharacter::ZoomCameraPressed()
{
	bAimingButtonPressed = true;
	if (CombatState != ECombatState::ECS_Reloading)
	{
		Aim();
	}
}

void AShooterCharacter::ZoomCameraReleased()
{
	bAimingButtonPressed = false;
	StopAiming();
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	if (isAiming)
	{
		NormalCameraView = FMath::FInterpTo(
			NormalCameraView,
			ZoomCameraView,
			DeltaTime,
			ZoomCameraSpeed
		);
		GetFollowCamera()->SetFieldOfView(NormalCameraView);
	}
	else
	{
		NormalCameraView = FMath::FInterpTo(
			NormalCameraView,
			BaseCameraView,
			DeltaTime,
			ZoomCameraSpeed
		);
	}
	GetFollowCamera()->SetFieldOfView(NormalCameraView);
}

void AShooterCharacter::SetLookSence() //Метод чувствительности для джойстика или стрелок на клавиатуре
{
	if (isAiming)
	{
		TernRightRate = AimTernSenceRate;
		TernUpRate = AimUpSenceRate;
	}
	else
	{
		TernRightRate = HipTernSenceRate;
		TernUpRate = HipUpSenceRate;
	}
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{ 0.f, 600.f };
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;

	//Calculate Crosshair Spread when moving
	CrosshairSpeedFactor = FMath::GetMappedRangeValueClamped(
		WalkSpeedRange,
		VelocityMultiplierRange,
		Velocity.Size()
	);

	//Calculate Crosshair spread when jumping
	if (GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 15.f);
	}

	//Calculate Crosshair spread when aiming
	if (isAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.2f, DeltaTime, 1.f);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
	}

	//Calculate crosshair spread when shooting
	if (isFire)
	{
		CrosshairShootFactor = FMath::FInterpTo(CrosshairShootFactor, 0.3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootFactor = FMath::FInterpTo(CrosshairShootFactor, 0.f, DeltaTime, 60.f);
	}

	//Calculate Spread Multiplier
	CrosshairSpreadMultiplier = 0.2f + 
		CrosshairSpeedFactor +
		CrosshairInAirFactor -
		CrosshairAimFactor +
		CrosshairShootFactor;

	//UE_LOG(LogTemp, Warning, TEXT("CrosshairSpeedFactor: %f"), CrosshairSpeedFactor);
}

void AShooterCharacter::isCrosshairShooting()
{
	isFire = true;

	GetWorldTimerManager().SetTimer(
		CrosshairShootTimerHandle,
		this, 
		&AShooterCharacter::notCrosshairShooting, 
		ShootingTime);
}

void AShooterCharacter::notCrosshairShooting()
{
	isFire = false;
}

bool AShooterCharacter::GetInfoCrosshair(FHitResult& hitResult, FVector& OutHitLocation)
{
	//Получаем размер экрана
	FVector2D ScreenSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ScreenSize);
	}
	//Найдем расположение кроссхаира
	FVector2D CrosshairLocation (ScreenSize.X / 2.f, ScreenSize.Y / 2.f);
	//Объявим два вектора расположения и направления кроссхаира на сцене (world)
	FVector CrosshairWorldLocation;
	FVector CrosshairWorldDirection;
	//Получим значения векторов используя метод DeprojectScreenToWorld()
	bool isDeprojectScreen = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldLocation,
		CrosshairWorldDirection
	);
	//Если DeprojectScreen сработал получим направление
	if (isDeprojectScreen)
	{
		const FVector Start{ CrosshairWorldLocation };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(hitResult, Start, End, ECollisionChannel::ECC_Visibility);
		if (hitResult.bBlockingHit)
		{
			OutHitLocation = hitResult.Location;
			return true;
		}
	}
	return false;
}

void AShooterCharacter::TraceForItems()
{
	if (isShouldTraceForItems)
	{
		FHitResult ItemTraceResult;
		FVector HitLocation;
		GetInfoCrosshair(ItemTraceResult, HitLocation);
		if (ItemTraceResult.bBlockingHit)
		{
			HitItem = Cast<AItem>(ItemTraceResult.GetActor());
			if (HitItem && HitItem->GetPickupComponent())
			{
				HitItem->GetPickupComponent()->SetVisibility(true);
			}
			if (LastFrameTraceHitItem)
			{
				if (LastFrameTraceHitItem != HitItem)
				{
					LastFrameTraceHitItem->GetPickupComponent()->SetVisibility(false);
				}
			}

			LastFrameTraceHitItem = HitItem;
		}
	}
	else if (LastFrameTraceHitItem)
	{
		LastFrameTraceHitItem->GetPickupComponent()->SetVisibility(false);
	}
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeapon)
	{
		AWeapon* SpawnedWepon = GetWorld()->SpawnActor<AWeapon>(DefaultWeapon);
		return SpawnedWepon;
	}
	else
	{
		return nullptr;
	}
}

void AShooterCharacter::EquipWeapon(AWeapon* EquipWeapon)
{
	if (EquipWeapon)
	{
		const USkeletalMeshSocket* WeaponSlot = GetMesh()->GetSocketByName(FName("RightHandsocket"));

		if (WeaponSlot)
		{
			WeaponSlot->AttachActor(EquipWeapon, GetMesh());
		}

		EquiptedWeapon = EquipWeapon;
		EquiptedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquiptedWeapon)
	{
		FDetachmentTransformRules DetachmentTransformRules(EDetachmentRule::KeepWorld, true);
		EquiptedWeapon->GetSkeletalMesh()->DetachFromComponent(DetachmentTransformRules);

		EquiptedWeapon->SetItemState(EItemState::EIS_Falling);
		EquiptedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SelectButtonPressed()
{
	if (HitItem)
	{
		HitItem->StartItemCurve(this);
	}
}

void AShooterCharacter::SelectButtonReleased()
{
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	HitItem = nullptr;
	LastFrameTraceHitItem = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, Starting9mmAmmo);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingARAmmo);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquiptedWeapon == nullptr) return false;
	return EquiptedWeapon->GetAmmo() > 0;
}

void AShooterCharacter::PlayFireSound()
{
	//PlaySound
	if (FireSound) UGameplayStatics::PlaySound2D(this, FireSound);
}

void AShooterCharacter::SendBullet()
{
	//Send Bullet
	const USkeletalMeshSocket* ParticleShotSocket = EquiptedWeapon->GetSkeletalMesh()->GetSocketByName(TEXT("BarrelSocket"));
	if (ParticleShotSocket)
	{
		const FTransform SocketTransform = ParticleShotSocket->GetSocketTransform(EquiptedWeapon->GetSkeletalMesh());

		if (ParticleShot)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticleShot, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(
			SocketTransform.GetLocation(), BeamEnd);
		if (bBeamEnd)
		{
			if (InpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(
					GetWorld(),
					InpactParticle,
					BeamEnd);
			}

			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				BeamParticle,
				SocketTransform);
			if (Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}

void AShooterCharacter::PlayGunFireMontage()
{
	//Play Gun Fire Montage
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && WeaponFire)
	{
		AnimInstance->Montage_Play(WeaponFire);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	if (EquiptedWeapon == nullptr) return;
	
	// Do we have ammo of correct type?
	if (CarryingAmmo() && !EquiptedWeapon->ClipIsFull()) 
	{
		if (isAiming)
		{
			StopAiming();
		}

		CombatState = ECombatState::ECS_Reloading;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && ReloadMontage)
		{
			AnimInstance->Montage_Play(ReloadMontage);
			AnimInstance->Montage_JumpToSection(EquiptedWeapon->GetReloadMontageSection());
		}
	}
}

bool AShooterCharacter::CarryingAmmo()
{
	if (EquiptedWeapon == nullptr) return false;

	auto AmmoType = EquiptedWeapon->GetAmmoType();
	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}

	return false;
}

void AShooterCharacter::GrabClip()
{
	if (EquiptedWeapon == nullptr) return;
	if (HandSceneComponent == nullptr) return;

	int32 ClipBoneIndex = EquiptedWeapon->GetSkeletalMesh()->GetBoneIndex(EquiptedWeapon->GetClipBoneName());
	ClipTransform = EquiptedWeapon->GetSkeletalMesh()->GetBoneTransform(ClipBoneIndex);

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName(TEXT("hand_l")));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquiptedWeapon->SetMovingClip(true);
}

void AShooterCharacter::ReplaceClip()
{
	EquiptedWeapon->SetMovingClip(false);
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (!GetCharacterMovement()->IsFalling())
	{
		bCrouching = !bCrouching;
	}
	if (bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		GetCharacterMovement()->GroundFriction = CrouchingFrictionValue;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		GetCharacterMovement()->GroundFriction = BaseFrictionValue;
	}
}

void AShooterCharacter::Jump()
{
	if (bCrouching)
	{
		bCrouching = false;
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
	else
	{
		ACharacter::Jump();
	}
	
}

void AShooterCharacter::InterpCapsuleHalfHight(float DeltaTime)
{
	float TargetCapsuleHalfHight{};

	if (bCrouching)
	{
		TargetCapsuleHalfHight = CrouchingCapsuleHalfHight;
	}
	else
	{
		TargetCapsuleHalfHight = StandingCapsuleHalfHight;
	}

	const float InterpHalfHight{ FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHight, DeltaTime, 20.f)};

	const float DeltaCapsuleHalfHeight{ InterpHalfHight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight() };
	const FVector MeshOffset{ 0.f, 0.f, -DeltaCapsuleHalfHeight };
	GetMesh()->AddLocalOffset(MeshOffset); 

	GetCapsuleComponent()->SetCapsuleHalfHeight(InterpHalfHight);
}

/**
 * @brief 
*/
void AShooterCharacter::Aim()
{
	isAiming = true;
	GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
}

/**
 * @brief 
*/
void AShooterCharacter::StopAiming()
{
	isAiming = false;
	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	}
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		int32 AmmoCount{ AmmoMap[Ammo->GetAmmoType()] };
		AmmoCount += Ammo->GetItemCount();
		AmmoMap[Ammo->GetAmmoType()] = AmmoCount;
	}

	if (EquiptedWeapon->GetAmmoType() == Ammo->GetAmmoType())
	{
		if (EquiptedWeapon->GetAmmo() == 0)
		{
			ReloadWeapon();
		}
	}

	Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocations()
{
	FInterpLocation WeaponLocation{ WeaponInterpComp, 0 };
	InterpLocations.Add(WeaponLocation);

	FInterpLocation InterpLoc1{ InterpComp1, 0 };
	InterpLocations.Add(InterpLoc1);

	FInterpLocation InterpLoc2{ InterpComp2, 0 };
	InterpLocations.Add(InterpLoc2);

	FInterpLocation InterpLoc3{ InterpComp3, 0 };
	InterpLocations.Add(InterpLoc3);

	FInterpLocation InterpLoc4{ InterpComp4, 0 };
	InterpLocations.Add(InterpLoc4);

	FInterpLocation InterpLoc5{ InterpComp5, 0 };
	InterpLocations.Add(InterpLoc5);

	FInterpLocation InterpLoc6{ InterpComp6, 0 };
	InterpLocations.Add(InterpLoc6);
}
// No longer needed; AItem has GetInterpLocation
//FVector AShooterCharacter::GetCameraInterpLocation()
//{
//	FVector CameraWorldLocation{ FollowCamera->GetComponentLocation() };
//	FVector CameraForward{ FollowCamera->GetForwardVector() };
//	//cameralocation + forvardVector * addfv + {0, 0, adduv};
//	return CameraWorldLocation + CameraForward * CameraInterpDistance + FVector{ 0.f, 0.f, CameraInterpElevation };
//}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	if (Item->GetEquipSound())
	{
		UGameplayStatics::PlaySound2D(this, Item->GetEquipSound());
	}

	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		SwapWeapon(Weapon);
	}

	auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
	}
}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if (Index <= InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FInterpLocation();
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex{1};
	int32 LowestCount{ INT_MAX };

	for (int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

void AShooterCharacter::IncrementInterpLocItemCount(int32 Index, int32 Amount)
{
	if (Amount < -1 || Amount > 1) return;
	if (InterpLocations.Num() >= Index)
	{
		InterpLocations[Index].ItemCount += Amount;
	}
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime);

	CalculateCrosshairSpread(DeltaTime);

	//SetLookSence(); //Метод чувствительности для джойстика или стрелок на клавиатуре

	TraceForItems();

	InterpCapsuleHalfHight(DeltaTime);
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("TernUp"), this, &AShooterCharacter::TerningAtUp);
	PlayerInputComponent->BindAxis(TEXT("TernRight"), this, &AShooterCharacter::TerningAtRight);
	PlayerInputComponent->BindAxis(TEXT("MouseUp"), this, &AShooterCharacter::TurnMouseUp);
	PlayerInputComponent->BindAxis(TEXT("MouseRight"), this, &AShooterCharacter::TurnMouseRight);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AShooterCharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction(TEXT("MainFire"), IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(TEXT("MainFire"), IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction(TEXT("AimingButton"), IE_Pressed, this, &AShooterCharacter::ZoomCameraPressed);
	PlayerInputComponent->BindAction(TEXT("AimingButton"), IE_Released, this, &AShooterCharacter::ZoomCameraReleased);

	PlayerInputComponent->BindAction(TEXT("Select"), IE_Pressed, this, &AShooterCharacter::SelectButtonPressed);
	PlayerInputComponent->BindAction(TEXT("Select"), IE_Released, this, &AShooterCharacter::SelectButtonReleased);

	PlayerInputComponent->BindAction(TEXT("Reloading"), IE_Pressed, this, &AShooterCharacter::ReloadButtonPressed);

	PlayerInputComponent->BindAction(TEXT("Crouch"), IE_Pressed, this, &AShooterCharacter::CrouchButtonPressed);
}

void AShooterCharacter::FinishReloading()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (bAimingButtonPressed)
	{
		Aim(); 
	}

	// TODO: Update AmmoMap
	if (EquiptedWeapon == nullptr) return;

	const auto AmmoType = EquiptedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		int32 CarriedAmmo = AmmoMap[AmmoType];
		const int32 WeaponAmmoCapacity = EquiptedWeapon->GetMagazineCapacity();
		const int32 MagazineEmptySpace = WeaponAmmoCapacity - EquiptedWeapon->GetAmmo();

		if (CarriedAmmo <= MagazineEmptySpace)
		{
			EquiptedWeapon->ReloadWeapon(CarriedAmmo);
			CarriedAmmo = 0;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
		else
		{
			EquiptedWeapon->ReloadWeapon(MagazineEmptySpace);
			CarriedAmmo -= MagazineEmptySpace;
			AmmoMap.Add(AmmoType, CarriedAmmo);
		}
	}
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncrementOverlapedItemCount(int8 Amount)
{
	if (OverlapedItemCount + Amount <= 0)
	{
		OverlapedItemCount = 0;
		isShouldTraceForItems = false;
	}
	else
	{
		OverlapedItemCount += Amount;
		isShouldTraceForItems = true;
	}
}

