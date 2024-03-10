// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
AItem::AItem() : 
	ItemName(FString("Default Name")), 
	ItemCount(0), 
	itemRarity(EItemRarity::EIR_Common), 
	itemState(EItemState::EIS_Pickup),
	//Item interp move variables
	ItemInterpStartLocation(FVector{0.f}),
	CameraTargetLocation(FVector{0.f}),
	bInterping(false),
	ZCurveTime(0.7f),
	ItemInterpX(0.f),
	ItemInterpY(0.f),
	InterpInitialYawOffset(0.f),
	ItemTipe(EItemType::EIT_MAX),
	InterpLocIndex(0),
	MaterialIndex(0)
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ItemSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemSkeletalMesh"));
	SetRootComponent(ItemSkeletalMesh);

	ItemCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("ItemCollisionBox"));
	ItemCollisionBox->SetupAttachment(ItemSkeletalMesh);
	ItemCollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ItemCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickUpComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickUpComponent"));
	PickUpComponent->SetupAttachment(GetRootComponent());

	sphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	sphereComponent->SetupAttachment(GetRootComponent());
}

void AItem::SetActiveStars()
{
	for (int32 i = 0; i <= 5; i++)
	{
		ActiveStars.Add(false);
	}
	 
	switch (itemRarity)
	{
	case EItemRarity::EIR_Damaged:
		ActiveStars[1] = true;
		break;
	case EItemRarity::EIR_Common:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		break;
	case EItemRarity::EIR_Uncommon:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		break;
	case EItemRarity::EIR_Rare:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		break;
	case EItemRarity::EIR_Legendary:
		ActiveStars[1] = true;
		ActiveStars[2] = true;
		ActiveStars[3] = true;
		ActiveStars[4] = true;
		ActiveStars[5] = true;
		break;
	}
}

void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
	case EItemState::EIS_Pickup:
		ItemSkeletalMesh->SetSimulatePhysics(false);
		ItemSkeletalMesh->SetEnableGravity(false);
		ItemSkeletalMesh->SetVisibility(true);
		ItemSkeletalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		sphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		sphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		ItemCollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemCollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		ItemCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EItemState::EIS_EquipInterping:
		PickUpComponent->SetVisibility(false);

		ItemSkeletalMesh->SetSimulatePhysics(false);
		ItemSkeletalMesh->SetEnableGravity(false);
		ItemSkeletalMesh->SetVisibility(true);
		ItemSkeletalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		sphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		sphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		ItemCollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_PickedUp:
		break;
	case EItemState::EIS_Equipped:
		PickUpComponent->SetVisibility(false);

		ItemSkeletalMesh->SetSimulatePhysics(false);
		ItemSkeletalMesh->SetEnableGravity(false);
		ItemSkeletalMesh->SetVisibility(true);
		ItemSkeletalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		sphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		sphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		ItemCollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Falling:
		ItemSkeletalMesh->SetSimulatePhysics(true);
		ItemSkeletalMesh->SetEnableGravity(true);
		ItemSkeletalMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		ItemSkeletalMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemSkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

		sphereComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		sphereComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		ItemCollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_MAX:
		break;
	default:
		break;
	}
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();

	if (PickUpComponent)
	{
		PickUpComponent->SetVisibility(false);
	}
	
	SetActiveStars();     

	sphereComponent->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereBeginOverlap);
	sphereComponent->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	//Set item properties based on ItemState
	SetItemProperties(itemState);

	InitializeCustomDepth();
}

void AItem::OnSphereBeginOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlapedItemCount(1);
		}
	}
} 

void AItem::OnSphereEndOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
		if (ShooterCharacter)
		{
			ShooterCharacter->IncrementOverlapedItemCount(-1);
		}
	}
}

// Called every frame
void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ItemInterp(DeltaTime);
}

void AItem::PlayPickupSound()
{
	if (Character)
	{
		if (Character->ShouldPlayPickupSound())
		{
			Character->StartPickupSoundTimer();
			if (PickupSound)
			{
				UGameplayStatics::PlaySound2D(this, PickupSound);
			}
		}
	}
}

void AItem::PlayEquipSound()
{
	if (Character)
	{
		if (Character->ShouldPlayEquipSound())
		{
			Character->StartEquipSoundTimer();
			if (EquipSound)
			{
				UGameplayStatics::PlaySound2D(this, EquipSound);
			}
		}
	}
}

void AItem::SetItemState(EItemState newItemState)
{
	itemState = newItemState;
	SetItemProperties(newItemState);
}

void AItem::StartItemCurve(AShooterCharacter* ch)
{
	Character = ch;

	InterpLocIndex = Character->GetInterpLocationIndex();
	Character->IncrementInterpLocItemCount(InterpLocIndex, 1);

	PlayPickupSound();

	ItemInterpStartLocation = GetActorLocation();
	bInterping = true;
	SetItemState(EItemState::EIS_EquipInterping);

	GetWorldTimerManager().SetTimer(ItemInterpTimer, this, &AItem::FinishInterping, ZCurveTime);
	
	//get offset yaw rotation between camera and item
	const float CameraRotationYaw = Character->GetFollowCamera()->GetComponentRotation().Yaw;
	const float ItemRotationYaw = GetActorRotation().Yaw;
	InterpInitialYawOffset = ItemRotationYaw - CameraRotationYaw;
}

void AItem::EnableCustomDepth()
{
	ItemSkeletalMesh->SetRenderCustomDepth(true);
}

void AItem::DisableCustomDepth()
{
	ItemSkeletalMesh->SetRenderCustomDepth(false);
}

void AItem::FinishInterping()
{
	bInterping = false;
	if (Character)
	{
		Character->IncrementInterpLocItemCount(InterpLocIndex, -1);
		Character->GetPickupItem(this);
	}
	SetActorScale3D(FVector(1.f));

	DisableGlowMaterial();
	DisableCustomDepth();
}

void AItem::ItemInterp(float DeltaTime)
{
	if (!bInterping) return;
	
	if (Character && ItemZCurve)
	{
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
		const float CurveValue = ItemZCurve->GetFloatValue(ElapsedTime);

		FVector ItemLocation = ItemInterpStartLocation;
		const FVector CameraInterpLocation{ GetInterpLocation() };//{ Character->GetCameraInterpLocation() };
		// Получим дельту по оси Z
		const FVector ItemToCamera{ FVector(0.f, 0.f, (CameraInterpLocation - ItemLocation).Z) };
		const float DeltaZ = ItemToCamera.Size();

		// Add X and Y coordinate
		const FVector CurrentLocation{ GetActorLocation() };
		const float InterpXValue = FMath::FInterpTo(
			CurrentLocation.X,
			CameraInterpLocation.X,
			DeltaTime,
			30.f);
		const float InterpYValue = FMath::FInterpTo(
			CurrentLocation.Y,
			CameraInterpLocation.Y,
			DeltaTime,
			30.f);
		ItemLocation.X = InterpXValue;
		ItemLocation.Y = InterpYValue;

		// Добавим значение кривой в компоненту Z вектора начального положения
		ItemLocation.Z += CurveValue * DeltaZ;

		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

		const FRotator CameraRotation{ Character->GetFollowCamera()->GetComponentRotation() };
		FRotator ItemRotation{ 0.f, CameraRotation.Yaw + InterpInitialYawOffset, 0.f };
		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		if (ItemScaleCurve)
		{
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
			SetActorScale3D(FVector(ScaleCurveValue, ScaleCurveValue, ScaleCurveValue));
		}
		
	}
}

FVector AItem::GetInterpLocation()
{
	if (Character == nullptr) return FVector(0.f);

	switch (ItemTipe)
	{
	case EItemType::EIT_Ammo:
		return Character->GetInterpLocation(InterpLocIndex).SceneComponent->GetComponentLocation();
		break;
	case EItemType::EIT_Weapon:
		return Character->GetInterpLocation(0).SceneComponent->GetComponentLocation();
		break;
	}

	return FVector();
}

void AItem::InitializeCustomDepth()
{
	DisableCustomDepth();
}

void AItem::OnConstruction(const FTransform& Transform)
{
	if (MaterialInstance)
	{
		DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
		ItemSkeletalMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);
	}

	EnableGlowMaterial();
}

void AItem::EnableGlowMaterial()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 0);
	}
}

void AItem::DisableGlowMaterial()
{
	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), 1);
	}
}
