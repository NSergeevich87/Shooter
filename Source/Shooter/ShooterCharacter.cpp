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


// Sets default values
AShooterCharacter::AShooterCharacter() :
	TernRightRate(45.f),
	TernUpRate(45.f),
	isAiming(false),
	BaseCameraView(0.f),
	ZoomCameraView(40.f)
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create a camera boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.f; //The camera follows at this distance behind the character
	CameraBoom->bUsePawnControlRotation = true; //Rotate the arm based on the controller
	CameraBoom->SocketOffset = FVector{ 0.f, 50.f, 50.f };

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

	if (FollowCamera)
	{
		BaseCameraView = GetFollowCamera()->FieldOfView;
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
}

bool AShooterCharacter::GetBeamEndLocation(
	const FVector& MuzzelSocketLocation, 
	FVector& OutBeamLocation)
{
	//Get current size of the screen
	FVector2D ScreenSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ScreenSize);
	}
	//Get screen space location of crosshairs
	FVector2D CrosshairLocation{ ScreenSize.X / 2.f, ScreenSize.Y / 2.f };
	CrosshairLocation.Y -= 85;
	CrosshairLocation.X += 35;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	//Get world position and direction
	bool isDeprojectScreen = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (isDeprojectScreen)
	{
		FHitResult HitResult;
		const FVector Start = CrosshairWorldPosition;
		const FVector End = CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f;
		// Set beam end point to line trace and point
		OutBeamLocation = End;

		// Trace outward from crosshairs world location
		GetWorld()->LineTraceSingleByChannel(
			HitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		if (HitResult.bBlockingHit)
		{
			OutBeamLocation = HitResult.Location;
		}

		// Perform a second trace, this time from the gun barrel
		FHitResult ObjectHit;
		FVector StartObjectHit{ MuzzelSocketLocation };
		FVector EndObjectHit{ OutBeamLocation };
		GetWorld()->LineTraceSingleByChannel(
			ObjectHit,
			StartObjectHit,
			EndObjectHit,
			ECollisionChannel::ECC_Visibility);

		if (ObjectHit.bBlockingHit)
		{
			OutBeamLocation = ObjectHit.Location;
		}

		return true;
	}

	return false;
}

void AShooterCharacter::ZoomCameraPressed()
{
	isAiming = true;
	GetFollowCamera()->SetFieldOfView(ZoomCameraView);
}

void AShooterCharacter::ZoomCameraReleased()
{
	isAiming = false;
	GetFollowCamera()->SetFieldOfView(BaseCameraView);
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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
	PlayerInputComponent->BindAxis(TEXT("MouseUp"), this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis(TEXT("MouseRight"), this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction(TEXT("Jump"), IE_Released, this, &ACharacter::StopJumping);
	PlayerInputComponent->BindAction(TEXT("MainFire"), IE_Pressed, this, &AShooterCharacter::FireWeapon);
	PlayerInputComponent->BindAction(TEXT("AimingButton"), IE_Pressed, this, &AShooterCharacter::ZoomCameraPressed);
	PlayerInputComponent->BindAction(TEXT("AimingButton"), IE_Released, this, &AShooterCharacter::ZoomCameraReleased);
}

