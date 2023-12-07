// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/SphereComponent.h"
#include "ShooterCharacter.h"

// Sets default values
AItem::AItem() : ItemName(FString("Default Name")), ItemCount(0), itemRarity(EItemRarity::EIR_Common), itemState(EItemState::EIS_Pickup)
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
		break;
	case EItemState::EIS_PickedUp:
		break;
	case EItemState::EIS_Equipped:
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

}

void AItem::SetItemState(EItemState newItemState)
{
	itemState = newItemState;
	SetItemProperties(newItemState);
}

