// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponBase.generated.h"

UCLASS()
class TPS_SHOOT_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();
	
	UPROPERTY(EditAnywhere,  Category = "Weapon|Ammo")
	float TraceDistance=1000.f;

	void Fire(const FVector& AimPoint);
	
	UPROPERTY(VisibleAnywhere,Category = "Weapon")
	FName WeaponMuzzleSocketName=FName("Muzzle");
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;


	// 弹匣容量：配置数据
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Ammo", meta = (ClampMin = "1"))
	int32 MagazineCapacity = 30;

	// 当前弹药：运行时状态
	UPROPERTY(VisibleInstanceOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmo = 0;

	// 两次射击之间的最小间隔，单位为秒
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire", meta = (ClampMin = "0.01"))
	float FireInterval = 0.2f;

	// 下一次允许射击的游戏时间
	double NextAllowedFireTime = 0.0;
};
