#include "UI/CrosshairWidget.h"

#include "Components/Image.h"

void UCrosshairWidget::NativeConstruct()
{
	Super::NativeConstruct();
	MovingElements.Reset();
	CacheElement(CrosshairTop, FVector2D(0.f, -1.f));
	CacheElement(CrosshairTopOutline, FVector2D(0.f, -1.f));
	CacheElement(CrosshairBottom, FVector2D(0.f, 1.f));
	CacheElement(CrosshairBottomOutline, FVector2D(0.f, 1.f));
	CacheElement(CrosshairLeft, FVector2D(-1.f, 0.f));
	CacheElement(CrosshairLeftOutline, FVector2D(-1.f, 0.f));
	CacheElement(CrosshairRight, FVector2D(1.f, 0.f));
	CacheElement(CrosshairRightOutline, FVector2D(1.f, 0.f));
	ResetSpread();
}

void UCrosshairWidget::NativeDestruct()
{
	ResetSpread();
	MovingElements.Reset();
	Super::NativeDestruct();
}

void UCrosshairWidget::CacheElement(UWidget* Widget, const FVector2D& Direction)
{
	if (IsValid(Widget))
	{
		MovingElements.Add({Widget, Widget->GetRenderTransform().Translation, Direction});
	}
}

void UCrosshairWidget::NotifyShot(float SpreadPerShot, float MaximumSpread, float RecoverySpeed)
{
	if (!FMath::IsFinite(SpreadPerShot) || !FMath::IsFinite(MaximumSpread) ||
		!FMath::IsFinite(RecoverySpeed) || SpreadPerShot < 0.f ||
		MaximumSpread < 0.f || RecoverySpeed <= 0.f)
	{
		return;
	}

	CurrentSpread = FMath::Clamp(CurrentSpread + SpreadPerShot, 0.f, MaximumSpread);
	ActiveRecoverySpeed = RecoverySpeed;
	RecoveryDelayRemaining = FMath::Max(0.f, RecoveryDelay);
	ApplySpread();
}

void UCrosshairWidget::ResetSpread()
{
	CurrentSpread = 0.f;
	RecoveryDelayRemaining = 0.f;
	ApplySpread();
}

void UCrosshairWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (CurrentSpread <= 0.f)
	{
		return;
	}

	const float RecoveryDelta = FMath::Max(0.f, InDeltaTime - RecoveryDelayRemaining);
	RecoveryDelayRemaining = FMath::Max(0.f, RecoveryDelayRemaining - InDeltaTime);
	CurrentSpread *= FMath::Exp(-ActiveRecoverySpeed * RecoveryDelta);
	if (CurrentSpread < 0.01f)
	{
		CurrentSpread = 0.f;
	}
	ApplySpread();
}

void UCrosshairWidget::ApplySpread()
{
	for (const FMovingElement& Element : MovingElements)
	{
		if (UWidget* Widget = Element.Widget.Get())
		{
			Widget->SetRenderTranslation(Element.BaseTranslation + Element.Direction * CurrentSpread);
		}
	}
}
