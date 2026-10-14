// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/AProjectileBase.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"

// Sets default values
AAProjectileBase::AAProjectileBase()
{
	InitialLifeSpan = 3.0f;
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	ProjectileMesh =CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ProjectileMesh"));
	SphereCollision = CreateDefaultSubobject<USphereComponent>("SphereCollision");
	ProjectileMesh->SetupAttachment(SphereCollision);
	SetRootComponent(SphereCollision);
	
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->SetUpdatedComponent(SphereCollision);
	
	if (GetOwner())
	{
		SphereCollision->IgnoreActorWhenMoving(GetOwner(), true);
	}
	
	if (GetInstigator())
	{
		SphereCollision->IgnoreActorWhenMoving(GetInstigator(), true);
	}
}

// Called when the game starts or when spawned
void AAProjectileBase::BeginPlay()
{
	Super::BeginPlay();
	
}


