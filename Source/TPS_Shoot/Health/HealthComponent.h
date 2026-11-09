// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPS_SHOOT_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, NewHealth, float, MaxHealth);
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
	
	void ApplyDamage(float Damage);
	void Heal(float HealDamage);
	float GetCurrentHealth()const{return CurrentHealth;};
	bool GetIsDead()const{return bIsDead;};
	float GetMaxHealth()const{return MaxHealth;}
private:
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType,
	AController* InstigatedBy, AActor* DamageCauser);
	
	UPROPERTY(ReplicatedUsing = OnRep_HealthValues)
	float CurrentHealth=0;
	
	UPROPERTY(ReplicatedUsing = OnRep_IsDead)
	bool bIsDead=false;
	
	UFUNCTION()
	void OnRep_HealthValues();

	UFUNCTION()
	void OnRep_IsDead();
};
