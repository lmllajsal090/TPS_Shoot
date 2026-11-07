// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimerManager.h"
#include "Engine/NetSerialization.h"
#include "WeaponBase.generated.h"

class AAProjectileBase;
class UNiagaraComponent;
class USoundBase;
class USoundAttenuation;
class USoundConcurrency;
class UMaterialInterface;
class UPrimitiveComponent;
DECLARE_MULTICAST_DELEGATE(FOnWeaponStateChanged)


UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	Rifle  UMETA(DisplayName = "Rifle"),
	Pistol UMETA(DisplayName = "Pistol")
};

UENUM(BlueprintType)
enum class EFireMode : uint8
{
	SemiAutomatic  UMETA(DisplayName = "SemiAutomatic"),
	Automatic UMETA(DisplayName = "Automatic")
};

UCLASS()
class TPS_SHOOT_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeaponBase();
	
	FOnWeaponStateChanged OnWeaponStateChanged;
	
	void CancelReload();
	void Fire(const FVector& AimPoint);
	void StartReload();
	void HandleProjectileImpact(const FHitResult& Hit);
	
	UFUNCTION(BlueprintPure, Category = "Weapon|Reload")
	bool IsReloading() const { return bIsReloading; }
	
	int32 GetCurrentAmmo() const { return CurrentAmmo; }
	int32 GetMagazineCapacity() const { return MagazineCapacity; }
	int32 GetReserveAmmo() const { return ReserveAmmo; }
	FName GetHandSocketName() const { return HandSocketName; }
	
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Weapon|IK")
	FTransform GetLeftHandGripTransform() const;

	UFUNCTION(BlueprintPure, Category = "Weapon|Ammo")
	float GetTraceDistance() const { return TraceDistance; }
	
	virtual FTransform GetLeftHandGripTransform_Implementation() const;
	
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	EWeaponType GetWeaponType() const { return WeaponType; }
	
	UFUNCTION(BlueprintPure,Category="FireMode")
	EFireMode GetFireMode() const { return FireMode; }
	
	UFUNCTION(BlueprintPure,Category="FireInterval")
	float GetFireInterval() const { return FireInterval; }

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Impact")
	TObjectPtr<UMaterialInterface> ImpactDecalMaterial;

	// X is projection depth; Y/Z are the half-size on the surface, in centimeters.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Impact", meta = (ClampMin = "0.1", Units = "cm"))
	FVector ImpactDecalSize = FVector(1.5f, 3.5f, 3.5f);

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Impact", meta = (ClampMin = "0.1", Units = "s"))
	float ImpactDecalLifetime = 12.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Impact", meta = (ClampMin = "0.0", Units = "s"))
	float ImpactDecalFadeDuration = 3.f;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayImpactDecal(UPrimitiveComponent* HitComponent, FVector_NetQuantize ImpactPoint,
		FVector_NetQuantizeNormal ImpactNormal, float Roll);

	// Visual feedback in widget units; this does not change projectile spread.
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Crosshair", meta = (ClampMin = "0"))
	float CrosshairSpreadPerShot = 5.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Crosshair", meta = (ClampMin = "0"))
	float CrosshairMaxSpread = 14.f;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Crosshair", meta = (ClampMin = "0.01"))
	float CrosshairRecoverySpeed = 4.f;

	// 每发射击向上抬多少度。
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil",meta = (ClampMin = "0"))
	float RecoilPitch = 0.5f;

	// 每发左右随机偏移的最大角度。
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Recoil",meta = (ClampMin = "0"))
	float RecoilYawRange = 0.15f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Audio")
	TObjectPtr<USoundAttenuation> FireAttenuation;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Audio")
	TObjectPtr<USoundConcurrency> FireConcurrency;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Weapon|Effects")
	TObjectPtr<USoundBase> FireSound;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon|Fire")
	EFireMode FireMode=EFireMode::SemiAutomatic;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	EWeaponType WeaponType = EWeaponType::Rifle;
	
	// 该武器装备在角色手上时使用的 Socket
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Attachment")
	FName HandSocketName = FName("WeaponSocket");
	
	UPROPERTY(EditAnywhere,  Category = "Weapon|Ammo")
	float TraceDistance=1000.f;

	UPROPERTY(VisibleAnywhere,Category = "Weapon")
	FName WeaponMuzzleSocketName=FName("Muzzle");
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon|Effects")
	TObjectPtr<UNiagaraComponent> MuzzleFlashComponent;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayFireEffects();

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
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,EditDefaultsOnly, Category = "Weapon|Ammo",meta = (ClampMin = "0"))
	int32 ReserveAmmo = 90;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Reload",meta = (ClampMin = "0.01"))
	float ReloadDuration = 2.0f;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState,VisibleInstanceOnly, Category = "Weapon|Reload")
	bool bIsReloading = false;

	FTimerHandle ReloadTimerHandle;
	
	UPROPERTY(EditDefaultsOnly, Category = "Weapon|Projectile")
	TSubclassOf<AAProjectileBase> ProjectileClass;
	
	
};
