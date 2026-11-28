// Fill out your copyright notice in the Description page of Project Settings.

/**
 * 职责：游戏内投射物 Actor，管理移动、碰撞、生命周期及单次有效碰撞事件。
 * 职责边界：不管理武器槽、弹药或输入；目标生命状态由其伤害处理链维护。
 */

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AProjectileBase.generated.h"

class UStaticMeshComponent;
class UProjectileMovementComponent;
class USphereComponent;
UCLASS()
class TPS_SHOOT_API AAProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAProjectileBase();

	void SetDamage(float InDamage)
	{
		ProjectileDamage = FMath::Max(0.0f, InDamage);
	}
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UStaticMeshComponent>ProjectileMesh;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<UProjectileMovementComponent>ProjectileMovement;
	
	UPROPERTY(EditAnywhere)
	TObjectPtr<USphereComponent>SphereCollision;

	UPROPERTY(VisibleInstanceOnly, Category = "Projectile|Damage")
	float ProjectileDamage = 0.0f;

	// 一颗子弹只处理一次有效命中。
	bool bHasHit = false;
	
	UFUNCTION()
	void OnHit( UPrimitiveComponent* HitComponent,AActor* OtherActor,UPrimitiveComponent* OtherComp,FVector NormalImpulse,const FHitResult& Hit);
};
