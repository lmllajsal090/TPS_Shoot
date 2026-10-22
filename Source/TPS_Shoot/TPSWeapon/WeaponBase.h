// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "WeaponBase.generated.h"

class AAProjectileBase;
class UNiagaraComponent;
DECLARE_MULTICAST_DELEGATE(FOnWeaponStateChanged)
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
	
	void StartReload();
	
	int32 GetCurrentAmmo() const { return CurrentAmmo; }
	int32 GetMagazineCapacity() const { return MagazineCapacity; }
	int32 GetReserveAmmo() const { return ReserveAmmo; }
	bool IsReloading() const { return bIsReloading; }
	
	FOnWeaponStateChanged OnWeaponStateChanged;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Effects")
	TObjectPtr<UNiagaraComponent> MuzzleFlashComponent;
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|IK")
	FTransform GetLeftHandGripTransform() const;

	virtual FTransform GetLeftHandGripTransform_Implementation() const;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMesh;
	
	UFUNCTION()
	void OnRep_WeaponState();
	// 弹匣容量：配置数据
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Ammo", meta = (ClampMin = "1"))
	int32 MagazineCapacity = 30;

	// 当前弹药：运行时状态
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,VisibleInstanceOnly, Category = "Weapon|Ammo")
	int32 CurrentAmmo = 0;

	// 两次射击之间的最小间隔，单位为秒
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Fire", meta = (ClampMin = "0.01"))
	float FireInterval = 0.2f;

	// 下一次允许射击的游戏时间
	double NextAllowedFireTime = 0.0;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Damage",meta = (ClampMin = "0"))
	float Damage = 25.0f;
	
	void FinishReload();

	// 开局备用弹药，运行时逐步消耗
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,EditDefaultsOnly, Category = "Weapon|Ammo",
		meta = (ClampMin = "0"))
	int32 ReserveAmmo = 90;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Reload",
		meta = (ClampMin = "0.01"))
	float ReloadDuration = 2.0f;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,VisibleInstanceOnly, Category = "Weapon|Reload")
	bool bIsReloading = false;

	FTimerHandle ReloadTimerHandle;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Projectile")
	TSubclassOf<AAProjectileBase> ProjectileClass;
	
	
};
