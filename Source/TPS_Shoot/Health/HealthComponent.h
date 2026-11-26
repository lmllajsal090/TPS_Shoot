// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

USTRUCT(BlueprintType)
struct TPS_SHOOT_API FDamageTakenEvent
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Health|Damage")
	float Damage = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Health|Damage")
	FVector HitLocation = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "Health|Damage")
	FVector HitNormal = FVector::UpVector;

	UPROPERTY(BlueprintReadOnly, Category = "Health|Damage")
	FName HitBoneName = NAME_None;

	TWeakObjectPtr<AController> InstigatedBy;
	TWeakObjectPtr<AActor> DamageCauser;

	UPROPERTY(BlueprintReadOnly, Category = "Health|Damage")
	bool bHasHitInfo = false;

	UPROPERTY(BlueprintReadOnly, Category = "Health|Damage")
	bool bIsHeadshot = false;

	UPROPERTY(BlueprintReadOnly, Category = "Health|Damage")
	bool bIsFatal = false;
};
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPS_SHOOT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDamageTaken, FDamageTakenEvent, DamageEvent);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeath);
public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing=OnRep_HealthValues,EditAnywhere,BlueprintReadOnly,Category="Health")
	float MaxHealth=100;
	
	void InitializeHealth();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:	
	// 健康值变化事件
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	// 死亡事件
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;
	
	// 受伤事件
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDamageTaken OnDamageTaken;
	
	void ApplyDamage(float Damage);
	void ApplyDamageInternal(float Damage, FDamageTakenEvent Event);
	void Heal(float HealDamage);
	float GetCurrentHealth()const{return CurrentHealth;};
	bool GetIsDead()const{return bIsDead;};
	float GetMaxHealth()const{return MaxHealth;}
private:
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
		AController* InstigatedBy, AActor* DamageCauser);
	
	UFUNCTION()
	void HandleTakePointDamage(AActor* DamagedActor, float Damage, AController* InstigatedBy,
		FVector HitLocation, UPrimitiveComponent* FHitComponent, FName BoneName,
		FVector ShotFromDirection, const UDamageType* DamageType, AActor* DamageCauser);
	
	UPROPERTY(ReplicatedUsing = OnRep_HealthValues)
	float CurrentHealth=0;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	bool bIsDead=false;
	
	UFUNCTION()
	void OnRep_HealthValues();

	UFUNCTION()
	void OnRep_IsDead();
	
	/** 仅用于在同一次 TakeDamage 调用中，将 OnTakePointDamage 的命中信息传递给 OnTakeAnyDamage */
	struct FPendingPointHit
	{
		bool bValid = false;
		FVector HitLocation = FVector::ZeroVector;
		FVector HitNormal = FVector::UpVector;
		FName BoneName = NAME_None;
		void Reset()
		{
			bValid = false;
			HitLocation = FVector::ZeroVector;
			HitNormal = FVector::UpVector;
			BoneName = NAME_None;
		}
	};
	
	FPendingPointHit PendingPointHit;
	
};
