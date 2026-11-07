// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/NetSerialization.h"
#include "TimerManager.h"
#include "CombatComponent.generated.h"


class AWeaponBase;
class UAnimMontage;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPS_SHOOT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();
	
	// 对外接口：请求切换武器
	void SwitchWeapon(int32 SlotIndex);
	void CancelWeaponSwitch();

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsSwitchingWeapon() const{return bIsSwitchingWeapon;}
	
	// 对外接口：获取当前武器
	AWeaponBase* GetCurrentWeapon() const;
	
	void RegisterInitialWeapon(AWeaponBase* Weapon);
	
	void Fire(const FVector& AimPoint);
	void Reload();
	void StartFire();
	void StopFire();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
protected:
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayWeaponSwitch(UAnimMontage* SwitchMontage);
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Weapon|Montage")
	TObjectPtr<UAnimMontage> AM_SwitchToPistol;
	
	UPROPERTY(EditDefaultsOnly,BlueprintReadOnly,Category="Weapon|Montage")
	TObjectPtr<UAnimMontage> AM_SwitchToRifle;
	
	UPROPERTY()
	TObjectPtr<AWeaponBase> PendingWeapon;
	
	bool bFireButtonPressed=false;
	FTimerHandle FireTimerHandle;
	TWeakObjectPtr<AWeaponBase>FiringWeapon;
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerFire(FVector_NetQuantize AimPoint);
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
private:	
	UPROPERTY()
	TArray<AWeaponBase*> WeaponSlots;
	UPROPERTY(Replicated)
	bool bIsSwitchingWeapon = false;
	
	void AttachWeaponToSocket(AWeaponBase* Weapon, FName SocketName);
	void FinishWeaponSwitch();
	void TryFire();
	void CommitWeaponSwitch();
	void HideCurrentWeapon();

	FTimerHandle WeaponSwitchHideTimerHandle;
	FTimerHandle WeaponSwitchCommitTimerHandle;
	FTimerHandle WeaponSwitchFinishTimerHandle;
};
