// Fill out your copyright notice in the Description page of Project Settings.


#include "StartBlock.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

// Sets default values
AStartBlock::AStartBlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	// Road Mesh Constructor
	static ConstructorHelpers::FObjectFinder<UStaticMesh> roadMesh(TEXT("StaticMesh'/Game/Meshes/RoadStart.RoadStart'"));
	RoadMesh = roadMesh.Object;

	// Define Road Component
	RoadComponent = CreateDefaultSubobject<UStaticMeshComponent>("RoadComponent");
	RoadComponent->SetStaticMesh(RoadMesh);
}

// Called when the game starts or when spawned
void AStartBlock::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AStartBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

