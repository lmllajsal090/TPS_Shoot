// Copyright Epic Games, Inc. All Rights Reserved.

#include "TPS_ShootCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "TPS_Shoot.h"
#include "TPSWeapon/WeaponBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "UI/AMmoWidget.h"
#include "GameFramework/PlayerController.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"



ATPS_ShootCharacter::ATPS_ShootCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	bUseControllerRotationPitch = false;//上抬下俯
	bUseControllerRotationRoll = false;//左右倾斜4
	
	bUseControllerRotationYaw = true;//左右转向
	// 身体朝向由控制器决定
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bUsePawnControlRotation = true;

	// 配置肩部视角
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->SocketOffset = FVector(0.f, 20.f, 46.f);
	CameraBoom->bEnableCameraLag = false;
	
	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;
	
	FollowCamera->bUsePawnControlRotation = false;
	FollowCamera->FieldOfView = 85.f;
	
	PrimaryActorTick.bCanEverTick = true;
}

void ATPS_ShootCharacter::BeginPlay()
{
	Super::BeginPlay();
	NormalArmLength = CameraBoom->TargetArmLength;
	NormalFOV = FollowCamera->FieldOfView;
	NormalWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
	bAimSettingsInitialized = true;
	ApplyAimingState();
	
	if (HasAuthority())
	{
		// Server-side muzzle transforms must stay current even when off screen.
		GetMesh()->VisibilityBasedAnimTickOption =
			EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
		InitializeDefaultWeapon();
	}

	if (IsLocallyControlled())
	{
		InitializeLocalUI();
	}
	
	if (WeaponAttachSocketName.IsNone() ||
     		!GetMesh()->DoesSocketExist(WeaponAttachSocketName))
     	{
     		UE_LOG(LogTPS_Shoot, Warning, TEXT("持枪 Socket 不存在"));
     		return;
     	}
}

void ATPS_ShootCharacter::OnRep_EquippedWeapon()
{
	
	TryBindAmmoWidget();
}


void ATPS_ShootCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATPS_ShootCharacter, EquippedWeapon);
	DOREPLIFETIME(ATPS_ShootCharacter, bIsAiming);
}

void ATPS_ShootCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATPS_ShootCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ATPS_ShootCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATPS_ShootCharacter::Look);
		
		// Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ATPS_ShootCharacter::Fire);
		
		// Reload
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ATPS_ShootCharacter::Reload);
		
		if (AimAction)
		{
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &ATPS_ShootCharacter::StartAiming);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &ATPS_ShootCharacter::StopAiming);
			EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Canceled, this, &ATPS_ShootCharacter::StopAiming);
		}
	}
	else
	{
		UE_LOG(LogTPS_Shoot, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ATPS_ShootCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void ATPS_ShootCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void ATPS_ShootCharacter::Fire(const FInputActionValue& Value)
{
	if (!IsLocallyControlled() || !IsValid(EquippedWeapon))
	{
		return;
	}
	
	ServerFire(GetAimPoint());
	
}

void ATPS_ShootCharacter::ServerFire_Implementation(
	FVector_NetQuantize AimPoint)
{

	if (!IsValid(EquippedWeapon) || AimPoint.ContainsNaN())
	{
		return;
	}

	EquippedWeapon->Fire(AimPoint);
}

void ATPS_ShootCharacter::StartAiming()
{
	SetAiming(true);
}

void ATPS_ShootCharacter::StopAiming()
{
	SetAiming(false);
}

void ATPS_ShootCharacter::SetAiming(bool bNewAiming)
{
	if (!IsLocallyControlled())
	{
		return;
	}

	// Apply locally for responsive input; the server owns the replicated state.
	bIsAiming = bNewAiming;
	ApplyAimingState();
	if (!HasAuthority())
	{
		ServerSetAiming(bNewAiming);
	}
}

void ATPS_ShootCharacter::ServerSetAiming_Implementation(bool bNewAiming)
{
	bIsAiming = bNewAiming;
	ApplyAimingState();
}

void ATPS_ShootCharacter::OnRep_IsAiming()
{
	ApplyAimingState();
}

void ATPS_ShootCharacter::ApplyAimingState()
{
	if (!bAimSettingsInitialized)
	{
		return;
	}

	GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : NormalWalkSpeed;
	if (IsLocallyControlled())
	{
		bCameraTransitioning = true;
	}
}

void ATPS_ShootCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	if (!IsLocallyControlled() || !bCameraTransitioning)
	{
		return;
	}
	
	const float TargetArm =bIsAiming ? AimArmLength : NormalArmLength;

	const float TargetFOV =bIsAiming ? AimFOV : NormalFOV;
	
	const bool bArmFinished = FMath::IsNearlyEqual(
	CameraBoom->TargetArmLength, TargetArm, 0.1f);

	const bool bFOVFinished = FMath::IsNearlyEqual(
		FollowCamera->FieldOfView, TargetFOV, 0.01f);

	if (bArmFinished && bFOVFinished)
	{
		CameraBoom->TargetArmLength = TargetArm;
		FollowCamera->SetFieldOfView(TargetFOV);

		bCameraTransitioning = false;
	}else
	{
		CameraBoom->TargetArmLength = FMath::FInterpTo(
			CameraBoom->TargetArmLength,
			TargetArm,
			DeltaSeconds,
			AimSpeed
		);

		FollowCamera->SetFieldOfView(FMath::FInterpTo(
			FollowCamera->FieldOfView,
			TargetFOV,
			DeltaSeconds,
			AimSpeed
		));
	}
	
}

void ATPS_ShootCharacter::ServerReload_Implementation()
{
	if (IsValid(EquippedWeapon))
	{
		EquippedWeapon->StartReload();
	}
}

FVector ATPS_ShootCharacter::GetAimPoint() const
{
	if (!IsValid(EquippedWeapon))
	{
		return GetPawnViewLocation();
	}

	const FVector Start = FollowCamera->GetComponentLocation();
	FVector End=Start+FollowCamera->GetForwardVector()*EquippedWeapon->TraceDistance;
	
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);
	Params.AddIgnoredActor(EquippedWeapon);
	
	FHitResult Hit;
	bool bHit=GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECollisionChannel::ECC_Visibility,
		Params
		);
	
	return bHit ? Hit.ImpactPoint : End;
}

void ATPS_ShootCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void ATPS_ShootCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController())
	{
		const float Multiplier =
			bIsAiming ? AimLookMultiplier : 1.f;

		AddControllerYawInput(Yaw * Multiplier);
		AddControllerPitchInput(Pitch * Multiplier);
	}
}

void ATPS_ShootCharacter::Reload(const FInputActionValue& Value)
{
	if (IsLocallyControlled() && IsValid(EquippedWeapon))
	{
		ServerReload();
	}
}

void ATPS_ShootCharacter::InitializeDefaultWeapon()
{
	if (!DefaultWeaponClass)
	{
		UE_LOG(LogTPS_Shoot, Warning, TEXT("未设置默认武器类"));
		return;
	}
	
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	
	EquippedWeapon=GetWorld()->SpawnActor<AWeaponBase>(
		DefaultWeaponClass,
		GetActorTransform(), 
		SpawnParams
		);
	
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachToComponent(
			GetMesh(),
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			WeaponAttachSocketName);
	}
	
}

void ATPS_ShootCharacter::InitializeLocalUI()
{
	APlayerController* PC =Cast<APlayerController>(GetController());

	if (!PC || !PC->IsLocalController())
	{
		return;
	}
	
	if (!IsValid(AmmoWidget) && AmmoWidgetClass)
	{
		AmmoWidget = CreateWidget<UAmmoWidget>(PC, AmmoWidgetClass);
	}
	if (IsValid(AmmoWidget))
	{
		if (!AmmoWidget->IsInViewport())
		{
			AmmoWidget->AddToViewport();
		}
		TryBindAmmoWidget();
	}
	
	if (!IsValid(CrosshairWidget) && CrosshairWidgetClass)
	{
		CrosshairWidget =
			CreateWidget<UUserWidget>(PC, CrosshairWidgetClass);
    
		if (CrosshairWidget)
		{
			CrosshairWidget->SetVisibility(
				ESlateVisibility::HitTestInvisible);
    
			CrosshairWidget->AddToViewport(10);
		}
	}
}

void ATPS_ShootCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void ATPS_ShootCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}

void ATPS_ShootCharacter::TryBindAmmoWidget()
{
	if (!IsLocallyControlled())
	{
		return;
	}

	if (!IsValid(AmmoWidget))
	{
		return;
	}

	AmmoWidget->SetWeapon(EquippedWeapon.Get());
}

void ATPS_ShootCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();
	if (HasActorBegunPlay())
	{
		if (IsLocallyControlled())
		{
			InitializeLocalUI();
		}
		else
		{
			RemoveLocalUI();
		}
	}
}

void ATPS_ShootCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	if (HasActorBegunPlay() && IsLocallyControlled())
	{
		InitializeLocalUI();
	}
}

void ATPS_ShootCharacter::RemoveLocalUI()
{
	if (IsValid(AmmoWidget))
	{
		AmmoWidget->SetWeapon(nullptr);
		AmmoWidget->RemoveFromParent();
		AmmoWidget = nullptr;
	}
	if (IsValid(CrosshairWidget))
	{
		CrosshairWidget->RemoveFromParent();
		CrosshairWidget = nullptr;
	}
}

void ATPS_ShootCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	RemoveLocalUI();
	if (HasAuthority() && IsValid(EquippedWeapon))
	{
		EquippedWeapon->Destroy();
	}
	Super::EndPlay(EndPlayReason);
}
