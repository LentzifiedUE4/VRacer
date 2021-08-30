// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "VehiclePawn.generated.h"

UCLASS()
class VRACER_API AVehiclePawn : public APawn
{
	GENERATED_BODY()

		DECLARE_DYNAMIC_MULTICAST_DELEGATE(FResetRaceDelegate);

public:
	// Sets default values for this pawn's properties
	AVehiclePawn();

	// Actor Components
	UPROPERTY(VisibleAnywhere)
		class USceneComponent* SceneRoot;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USceneComponent* CameraRoot;
	UPROPERTY(VisibleAnywhere)
		class UCameraComponent* Camera;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class USkeletalMeshComponent* VehicleComponent;

	// Constructor Definitions
	UPROPERTY()
		USkeletalMesh* VehicleMesh;
	UPROPERTY()
		UParticleSystem* ImpactSparks;

	// Point Light Components
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UPointLightComponent* LightFL;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UPointLightComponent* LightBL;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UPointLightComponent* LightFR;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
		class UPointLightComponent* LightBR;

	// Variables
	UPROPERTY(BlueprintReadOnly)
		float SteerInput;
	UPROPERTY(BlueprintReadOnly)
		float BrakeInput;
	UPROPERTY(BlueprintReadOnly)
		float AccelInput;
	UPROPERTY(BlueprintReadWrite)
		float RaceTime;
	UPROPERTY(BlueprintReadOnly)
		bool bGroundContact;
	UPROPERTY(BlueprintReadWrite)
		bool bCanDrive;
	UPROPERTY(BlueprintReadWrite)
		bool bCanCount;
	UPROPERTY(BlueprintReadWrite)
		int32 LapCount;
	UPROPERTY(BlueprintReadWrite)
		FTransform CheckpointTransform;
	UPROPERTY(BlueprintReadWrite)
		FTransform SpawnTransform;

	// Timer Handles
	FTimerHandle RespawnTimer;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Vehicle Input Functions
	UFUNCTION()
		void CallSteer(float Amount);
	UFUNCTION()
		void CallBrake(float Amount);
	UFUNCTION()
		void CallAccelerate(float Amount);

	// Race Handling Functions
	UFUNCTION()
		void SpawnAtCheckpoint();
	UFUNCTION()
		void RestartRace();
	UFUNCTION()
		void EnablePhysics();

	// Vehicle Physics Functions
	UFUNCTION()
		void AddHoverForce();
	UFUNCTION()
		void BalanceVehicle();

	// Base Class Functions
	UFUNCTION()
		void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	// Broadcast Delegates
	UPROPERTY(BlueprintAssignable)
		FResetRaceDelegate ResetRaceDelegate;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
