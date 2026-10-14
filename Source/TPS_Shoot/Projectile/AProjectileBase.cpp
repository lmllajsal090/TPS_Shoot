// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/AProjectileBase.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"

// Sets default values
AAProjectileBase::AAProjectileBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProjectileMesh =CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	SphereCollision = CreateDefaultSubobject<USphereComponent>("SphereCollision");
	ProjectileMesh->SetupAttachment(SphereCollision);
	SetRootComponent(SphereCollision);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->SetUpdatedComponent(SphereCollision);
}

// Called when the game starts or when spawned
void AAProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
}


