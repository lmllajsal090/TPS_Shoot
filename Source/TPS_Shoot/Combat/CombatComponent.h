// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "CombatComponent.generated.h"


class AWeaponBase;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPS_SHOOT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	
	// 对外接口：请求切换武器
	void SwitchWeapon(int32 SlotIndex);

	// 对外接口：获取当前武器
	AWeaponBase* GetCurrentWeapon() const;
	
	void RegisterInitialWeapon(AWeaponBase* Weapon);
	
	void Fire(const FVector& AimPoint);
	void Reload();
	void StartFire();
	void StopFire();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerFire(FVector_NetQuantize AimPoint);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:	
	UPROPERTY()
	TArray<AWeaponBase*> WeaponSlots;
	
	bool bIsSwitchingWeapon = false;
	
	void AttachWeaponToSocket(AWeaponBase* Weapon, FName SocketName);

	void FinishWeaponSwitch();
};
