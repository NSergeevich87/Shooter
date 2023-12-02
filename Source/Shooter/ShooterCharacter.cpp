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


// Sets default values
AShooterCharacter::AShooterCharacter() :
	//Чувствительность смещения по осям при помощи мыши
	MouseHipTurnSenceRate(1.0f),
	MouseHipUpSenceRate(1.0f),
	MouseAimTurnSenceRate(0.3f),
	MouseAimUpSenceRate(0.3f),
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
	ZoomCameraView(35.f),
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
	AutomaticFireRate(0.2f),
	//Item trace variables
	isShouldTraceForItems(false)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create a camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 220.f; //The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; //Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector{ 0.f, 50.f, 75.f };

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

}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	SpawnDefaultWeapon();

	if (FollowCamera)
	{
		BaseCameraView = GetFollowCamera()->FieldOfView;
		NormalCameraView = BaseCameraView;
	}
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
	isFireButtonPressed = true;

	if (FireSound) UGameplayStatics::PlaySound2D(this, FireSound);

	const USkeletalMeshSocket* ParticleShotSocket = GetMesh()->GetSocketByName(TEXT("ParticleFlash"));
	if (ParticleShotSocket)
	{
		const FTransform SocketTransform = ParticleShotSocket->GetSocketTransform(GetMesh());

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

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && WeaponFire)
	{
		AnimInstance->Montage_Play(WeaponFire);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	//Start bullet fire timer for crosshairs
	isCrosshairShooting();
}

void AShooterCharacter::StartFireTimer()
{
	if (isShouldFire)
	{
		FireWeapon();
		isShouldFire = false;
		GetWorldTimerManager().SetTimer(ShootingTimerHandle, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
	}
}

void AShooterCharacter::AutoFireReset()
{
	isShouldFire = true;
	if (isFireButtonPressed)
	{
		StartFireTimer();
	}
}

void AShooterCharacter::FireButtonPressed()
{
	isFireButtonPressed = true;
	StartFireTimer();
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
	isAiming = true;
}

void AShooterCharacter::ZoomCameraReleased()
{
	isAiming = false;
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

	UE_LOG(LogTemp, Warning, TEXT("CrosshairSpeedFactor: %f"), CrosshairSpeedFactor);
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
			AItem* HitItem = Cast<AItem>(ItemTraceResult.GetActor());
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

void AShooterCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeapon)
	{
		AWeapon* SpawnedWepon = GetWorld()->SpawnActor<AWeapon>(DefaultWeapon);

		const USkeletalMeshSocket* WeaponSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (WeaponSocket)
		{
			WeaponSocket->AttachActor(SpawnedWepon, GetMesh());
		}

		EquiptedWeapon = SpawnedWepon;
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
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction(TEXT("MainFire"), IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction(TEXT("MainFire"), IE_Released, this, &AShooterCharacter::FireButtonReleased);

	PlayerInputComponent->BindAction(TEXT("AimingButton"), IE_Pressed, this, &AShooterCharacter::ZoomCameraPressed);
	PlayerInputComponent->BindAction(TEXT("AimingButton"), IE_Released, this, &AShooterCharacter::ZoomCameraReleased);
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

