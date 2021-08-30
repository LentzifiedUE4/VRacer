// Fill out your copyright notice in the Description page of Project Settings.


#include "VehiclePawn.h"
#include "Camera/CameraComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Components/PointLightComponent.h"
#include "MotionControllerComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/GameEngine.h"
#include "Engine.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

// Sets default values
AVehiclePawn::AVehiclePawn()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Vehicle Mesh Constructor
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> vehicleMesh(TEXT("SkeletalMesh'/Game/Meshes/F1Car.F1Car'"));
	VehicleMesh = vehicleMesh.Object;

	// Vehicle Impact Sparks Constructor
	static ConstructorHelpers::FObjectFinder<UParticleSystem> impactSparks(TEXT("ParticleSystem'/Game/Particles/ImpactSparks.ImpactSparks'"));
	ImpactSparks = impactSparks.Object;

	// Define Scene Root
	SceneRoot = CreateDefaultSubobject<USceneComponent>("SceneRoot");
	SetRootComponent(SceneRoot);

	// Define Vehicle Component
	VehicleComponent = CreateDefaultSubobject<USkeletalMeshComponent>("VehicleComponent");
	VehicleComponent->SetSkeletalMesh(VehicleMesh);
	VehicleComponent->SetupAttachment(SceneRoot);
	VehicleComponent->SetSimulatePhysics(true);
	VehicleComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	// Bind Vehicle Hit
	VehicleComponent->OnComponentHit.AddDynamic(this, &AVehiclePawn::OnHit);

	// Define Point Lights
	LightFR = CreateDefaultSubobject<UPointLightComponent>("LightFR");
	LightBR = CreateDefaultSubobject<UPointLightComponent>("LightBR");
	LightFL = CreateDefaultSubobject<UPointLightComponent>("LightFL");
	LightBL = CreateDefaultSubobject<UPointLightComponent>("LightBL");

	// Attach Point Lights to Vehicle
	LightFR->SetupAttachment(VehicleComponent);
	LightBR->SetupAttachment(VehicleComponent);
	LightFL->SetupAttachment(VehicleComponent);
	LightBL->SetupAttachment(VehicleComponent);

	// Set Base Light Color
	LightFR->SetLightColor(FLinearColor(0.f, 1.f, 1.f));
	LightBR->SetLightColor(FLinearColor(0.f, 1.f, 1.f));
	LightFL->SetLightColor(FLinearColor(0.f, 1.f, 1.f));
	LightBL->SetLightColor(FLinearColor(0.f, 1.f, 1.f));

	// Create VR Root
	CameraRoot = CreateDefaultSubobject<USceneComponent>("CameraRoot");
	CameraRoot->SetupAttachment(VehicleComponent);

	// Create VR Camera
	Camera = CreateDefaultSubobject<UCameraComponent>("Camera");
	Camera->SetupAttachment(CameraRoot);
}

// Called when the game starts or when spawned
void AVehiclePawn::BeginPlay()
{
	Super::BeginPlay();

	// Set Initial Damping
	VehicleComponent->SetAngularDamping(1.5f);

	// Attach Lights to Flame Target Bones
	LightFR->AttachToComponent(VehicleComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), "FrontFlameTarget_R");
	LightBR->AttachToComponent(VehicleComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), "BackFlameTarget_R");
	LightFL->AttachToComponent(VehicleComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), "FrontFlameTarget_L");
	LightBL->AttachToComponent(VehicleComponent, FAttachmentTransformRules(EAttachmentRule::SnapToTarget, true), "BackFlameTarget_L");

	// Initialize Checkpoint Transform
	CheckpointTransform = SpawnTransform = VehicleComponent->GetComponentTransform();

	//GEngine->AddOnScreenDebugMessage(4, 10.f, FColor::Red, FString::Printf(TEXT("Weight: %f"), VehicleComponent->GetMass()));
}

// Call Steer Input
void AVehiclePawn::CallSteer(float Amount)
{
	if (bCanDrive)
	{
		// Steer Vehicle
		SteerInput = Amount * 450;
		VehicleComponent->AddAngularImpulseInDegrees(FVector(0.f, 0.f, SteerInput * (500.f * VehicleComponent->GetMass())) * VehicleComponent->GetUpVector());
	}
	else
	{
		// Zero Steer Input Variable
		SteerInput = 0.f;
	}

	//GEngine->AddOnScreenDebugMessage(1, 1.f, FColor::Red, FString::Printf(TEXT("Steering: %f"), Amount));
}

// Call Brake Input
void AVehiclePawn::CallBrake(float Amount)
{
	if (bCanDrive)
	{
		// Set Brake Input Variable
		BrakeInput = Amount;

		if (bGroundContact)
		{
			// Adjust Vehicle Damping and Add Reverse Force
			VehicleComponent->SetLinearDamping(1.5f + (Amount * 3.f));
			VehicleComponent->AddForce((VehicleComponent->GetForwardVector() * (-5000.f * VehicleComponent->GetMass())) * Amount);
		}
		else
		{
			// Reduce Vehicle Damping During Air Time
			VehicleComponent->SetLinearDamping(0.1f);
		}
	}
	else
	{
		// Zero Brake Input Variable
		BrakeInput = 0.f;
	}

	//GEngine->AddOnScreenDebugMessage(2, 1.f, FColor::Green, FString::Printf(TEXT("Brakes: %f"), Amount));
}

void AVehiclePawn::CallAccelerate(float Amount)
{
	if (bCanDrive)
	{
		// Set Accel Input Variable
		AccelInput = Amount;

		// Add Forward Force if Grounded
		if (bGroundContact)
		{
			VehicleComponent->AddForce((VehicleComponent->GetForwardVector() * (20000.f * VehicleComponent->GetMass())) * Amount);
		}

		// Set Light Intensities
		float LightIntensity = (Amount * 6000.f) + 1500.f;

		LightFR->SetIntensity(LightIntensity);
		LightFL->SetIntensity(LightIntensity);
		LightBR->SetIntensity(LightIntensity);
		LightBL->SetIntensity(LightIntensity);
	}
	else
	{
		// Zero Accel Input Variable
		AccelInput = 0.f;
	}

	//GEngine->AddOnScreenDebugMessage(3, 1.f, FColor::Blue, FString::Printf(TEXT("Acceleration: %f"), Amount));
}

// Respawn Vehicle At Last Checkpoint
void AVehiclePawn::SpawnAtCheckpoint()
{
	if (bCanDrive)
	{
		// Teleport Vehicle to Checkpoint Transform
		VehicleComponent->SetSimulatePhysics(false);
		VehicleComponent->SetWorldTransform(CheckpointTransform);

		// Delayed Physics Re-Enable to Prevent Weird Physics
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &AVehiclePawn::EnablePhysics, 0.1f);
	}
}

// Restart Race From Beginning
void AVehiclePawn::RestartRace()
{
	if (bCanDrive)
	{
		// Teleport Vehicle to Start
		VehicleComponent->SetSimulatePhysics(false);
		VehicleComponent->SetWorldTransform(SpawnTransform);

		// Reset Race Handling Variables
		bCanDrive = bCanCount = false;
		RaceTime = 0.f;

		// Broadcast Restart for BP Use
		ResetRaceDelegate.Broadcast();
		
		// Delayed Physics Re-Enable to Prevent Weird Physics
		GetWorldTimerManager().SetTimer(RespawnTimer, this, &AVehiclePawn::EnablePhysics, 0.1f);
	}
}

// Re-Enable Vehicle Physics After Teleport
void AVehiclePawn::EnablePhysics()
{
	VehicleComponent->SetSimulatePhysics(true);
}

// Set Vehicle Hover Force
void AVehiclePawn::AddHoverForce()
{
	// Define Cast Vector Arrays
	TArray<FVector> CastOrigins;
	TArray<FVector> CastTargets;

	// Bind Cast Origins to Force Root Bones
	CastOrigins.Add(VehicleComponent->GetBoneLocation("FrontForceRoot_L"));
	CastOrigins.Add(VehicleComponent->GetBoneLocation("BackForceRoot_L"));
	CastOrigins.Add(VehicleComponent->GetBoneLocation("FrontForceRoot_R"));
	CastOrigins.Add(VehicleComponent->GetBoneLocation("BackForceRoot_R"));

	// Bind Cast Targets to Below Force Root Bones
	CastTargets.Add(VehicleComponent->GetBoneLocation("FrontForceRoot_L") - FVector(0.f, 0.f, 500.f));
	CastTargets.Add(VehicleComponent->GetBoneLocation("BackForceRoot_L") - FVector(0.f, 0.f, 500.f));
	CastTargets.Add(VehicleComponent->GetBoneLocation("FrontForceRoot_R") - FVector(0.f, 0.f, 500.f));
	CastTargets.Add(VehicleComponent->GetBoneLocation("BackForceRoot_R") - FVector(0.f, 0.f, 500.f));

	// Initialize Ground Contact Variable
	bGroundContact = false;

	for (int32 i = 0; i < CastOrigins.Num(); i++)
	{
		//DrawDebugLine(GetWorld(), CastOrigins[i], CastTargets[i], FColor::Red, false);

		// Set up Trace Params
		FCollisionQueryParams TraceParams;
		TraceParams.AddIgnoredActor(this);
		FHitResult* TraceResult = new FHitResult();

		// Define Cast Bool
		bool bHit = GetWorld()->LineTraceSingleByChannel(*TraceResult, CastOrigins[i], CastTargets[i], ECC_Visibility, TraceParams);

		if (bHit)
		{
			// Calculate and Add Force to Vehicle
			float Force = (1.f / (FVector::Dist(TraceResult->Location, CastOrigins[i]) / 2)) * (60000.f * VehicleComponent->GetMass());
			VehicleComponent->AddForceAtLocation(TraceResult->Normal * Force, CastOrigins[i]);

			// Set Ground Contact
			bGroundContact = true;

			//GEngine->AddOnScreenDebugMessage(i, 1.f, FColor::Red, FString::Printf(TEXT("%f"), Force));
		}
	}
}

// Set Vehicle Angular Damping by Ground Contact
void AVehiclePawn::BalanceVehicle()
{
	if (bGroundContact)
	{
		VehicleComponent->SetAngularDamping(1.5f);
	}
	else
	{
		VehicleComponent->SetAngularDamping(4.f);
	}
}

// Vehicle Hit
void AVehiclePawn::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Define Hit Transform
	FTransform HitTransform;
	HitTransform.SetLocation(Hit.Location);
	HitTransform.SetRotation(Hit.Normal.ToOrientationQuat());

	// Emit Sparks at Impact
	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactSparks, HitTransform);

	//GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green, "Impact");
}

// Called every frame
void AVehiclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Add Hover Force if Properly Oriented
	if (VehicleComponent->GetRelativeRotation().Roll < 65.f && VehicleComponent->GetRelativeRotation().Roll > -65.f)
	{
		AddHoverForce();
	}
	else
	{
		bGroundContact = false;
	}

	// Call Vehicle Balance
	BalanceVehicle();

	// Increment Race Time
	if (bCanCount)
	{
		RaceTime += DeltaTime;
	}
}

// Called to bind functionality to input
void AVehiclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind Wheel Axis Inputs
	PlayerInputComponent->BindAxis("Steer", this, &AVehiclePawn::CallSteer);
	PlayerInputComponent->BindAxis("Brake", this, &AVehiclePawn::CallBrake);
	PlayerInputComponent->BindAxis("Accelerate", this, &AVehiclePawn::CallAccelerate);

	// Bind Wheel Action Inputs
	PlayerInputComponent->BindAction("SpawnAtCheckpoint", IE_Pressed, this, &AVehiclePawn::SpawnAtCheckpoint);
	PlayerInputComponent->BindAction("RestartRace", IE_Pressed, this, &AVehiclePawn::RestartRace);
}

