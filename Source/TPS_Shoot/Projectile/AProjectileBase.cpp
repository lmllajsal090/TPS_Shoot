// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile/AProjectileBase.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AAProjectileBase::AAProjectileBase()
{
	bReplicates = true;
	SetReplicateMovement(true);
	
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
	
	if (!HasAuthority())
	{
		ProjectileMovement->Deactivate();
		ProjectileMovement->SetComponentTickEnabled(false);

		SetActorEnableCollision(false);
		SetLifeSpan(0.f);

		return;
	}
	
	SphereCollision->OnComponentHit.AddDynamic(
		this,
		&AAProjectileBase::OnHit
	);

	if (GetOwner())
	{
		SphereCollision->IgnoreActorWhenMoving(GetOwner(), true);
	}

	if (GetInstigator())
	{
		SphereCollision->IgnoreActorWhenMoving(GetInstigator(), true);
	}
	
}

void AAProjectileBase::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority() || bHasHit)
	{
		return;
	}
	
	if (!OtherActor||OtherActor==this||OtherActor==GetOwner()||OtherActor==GetInstigator())
	{
		return;
	}

	// 在请求伤害之前标记，避免重复处理。
	bHasHit = true;

	UGameplayStatics::ApplyPointDamage(
		OtherActor,
		ProjectileDamage,
		GetActorForwardVector(),
		Hit,
		GetInstigatorController(),
		this,
		nullptr
	);
	Destroy();
}




