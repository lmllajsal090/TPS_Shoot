#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlueprintLayoutAutomationLibrary.generated.h"

class UBlueprint;

/** Thin, editor-only automation adapter. Layout is computed by upstream Graph Formatter. */
UCLASS()
class GRAPHFORMATTER_API UBlueprintLayoutAutomationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="Graph Formatter|Automation")
    static FString PrepareGraph(UBlueprint* Blueprint, const FString& GraphName);

    UFUNCTION(BlueprintCallable, Category="Graph Formatter|Automation")
    static FString InspectGraph(UBlueprint* Blueprint, const FString& GraphName);

    /** Empty NodeGuids means the whole named graph. Does not move nodes or save assets. */
    UFUNCTION(BlueprintCallable, Category="Graph Formatter|Automation")
    static FString PlanLayout(UBlueprint* Blueprint, const FString& GraphName, const TArray<FString>& NodeGuids);

    /** Rejects stale plans. Changes only position and comment bounds, in one Undo transaction. */
    UFUNCTION(BlueprintCallable, Category="Graph Formatter|Automation")
    static FString ApplyLayout(UBlueprint* Blueprint, const FString& GraphName, const FString& PlanJson);

    UFUNCTION(BlueprintCallable, Category="Graph Formatter|Automation")
    static FString CaptureGraph(UBlueprint* Blueprint, const FString& GraphName, const FString& Filename, bool FitAll=true);
};
