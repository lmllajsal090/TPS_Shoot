// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "TPS_ShootCharacter.generated.h"

class UAmmoWidget;
class USpringArmComponent;
class UCameraComponent;
class UInputAction;
class AWeaponBase;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class ATPS_ShootCharacter : public ACharacter
{
	GENERATED_BODY()

	
public:
	/** Constructor */
	ATPS_ShootCharacter();	

	FVector GetAimPoint() const;

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles look inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoLook(float Yaw, float Pitch);

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump pressed inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	virtual void BeginPlay() override;
	
	// 蓝图里选择开局生成哪一种武器
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<AWeaponBase> DefaultWeaponClass;

	// 游戏运行时实际生成的那一把武器
	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon,VisibleInstanceOnly,BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<AWeaponBase> EquippedWeapon;

	UFUNCTION()
	void OnRep_EquippedWeapon();
	
	// 填你已经创建好的角色持枪 Socket 名称
	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	FName WeaponAttachSocketName;
	
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual void GetLifetimeReplicatedProps(
	TArray<FLifetimeProperty>& OutLifetimeProps) const override;
protected:

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> FollowCamera;
	
	/*Fire Input Action*/
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> FireAction;
	
	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category="Input")
	TObjectPtr<UInputAction> MouseLookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> ReloadAction;

	// 在角色蓝图中选择 WBP_Ammo
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UAmmoWidget> AmmoWidgetClass;

	// 运行时创建的界面实例
	UPROPERTY(Transient)
	TObjectPtr<UAmmoWidget> AmmoWidget;
	
	void Reload(const FInputActionValue& Value);
	
	void InitializeDefaultWeapon();
	void InitializeLocalUI();
	
	void TryBindAmmoWidget();
	
	/** Initialize input action bindings */
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	UFUNCTION(Server, Reliable)
	void ServerFire(FVector_NetQuantize AimPoint);
	
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	/*Callled for Fire*/
	void Fire(const FInputActionValue& Value);
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Camera|Aim")
	bool bIsAiming = false;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
	float AimArmLength = 280.f;

	UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
	float AimSpeed = 12.f;
	
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim")
	float AimFOV = 65.f;

	// 缓存你实际配置的普通视角
	float NormalArmLength = 0.f;
	float NormalFOV = 0.f;

	bool bCameraTransitioning = false;
	
	void StartAiming();
	void StopAiming();

	virtual void Tick(float DeltaSeconds) override;
	
	UPROPERTY(EditDefaultsOnly, Category = "Camera|Aim",
	meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float AimLookMultiplier = 0.65f;

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Aim",
		meta = (ClampMin = "0"))
	float AimWalkSpeed = 270.f;

	float NormalWalkSpeed = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> CrosshairWidget;
	
	UFUNCTION(Server, Reliable)
	void ServerReload();
};

