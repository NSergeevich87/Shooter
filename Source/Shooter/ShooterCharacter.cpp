  // Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Warning, TEXT("Begin Game"));

	int myNum{ 42 };
	float myFloat{ 3.14159f };
	char myChar{ 'N' };
	bool myBool{ true };
	FString myString{TEXT( "Nikita Nikiforov" )};

	UE_LOG(LogTemp, Warning, TEXT("mynum, myfloat, mychar, mybool and myname: %d, %f, %c, %d, %s"), myNum, myFloat, myChar, myBool, *myString);
	UE_LOG(LogTemp, Warning, TEXT("Name of the Character is: %s"), *GetName());
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

}

