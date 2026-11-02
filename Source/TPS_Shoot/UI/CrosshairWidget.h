#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CrosshairWidget.generated.h"

class UImage;

UCLASS()
class TPS_SHOOT_API UCrosshairWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void NotifyShot(float SpreadPerShot, float MaximumSpread, float RecoverySpeed);

	UFUNCTION(BlueprintCallable, Category = "Crosshair")
	void ResetSpread();

	UFUNCTION(BlueprintPure, Category = "Crosshair")
	float GetCurrentSpread() const { return CurrentSpread; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CrosshairTop;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CrosshairBottom;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CrosshairLeft;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> CrosshairRight;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CrosshairTopOutline;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CrosshairBottomOutline;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CrosshairLeftOutline;
	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UImage> CrosshairRightOutline;

	// Keep the kick visible briefly before easing back, in seconds.
	UPROPERTY(EditDefaultsOnly, Category = "Crosshair", meta = (ClampMin = "0"))
	float RecoveryDelay = 0.1f;

private:
	struct FMovingElement
	{
		TWeakObjectPtr<UWidget> Widget;
		FVector2D BaseTranslation;
		FVector2D Direction;
	};

	TArray<FMovingElement> MovingElements;
	float CurrentSpread = 0.f;
	float ActiveRecoverySpeed = 4.f;
	float RecoveryDelayRemaining = 0.f;

	void CacheElement(UWidget* Widget, const FVector2D& Direction);
	void ApplySpread();
};
