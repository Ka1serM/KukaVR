#pragma once

#include "CoreMinimal.h"
#include "MaterialCompiler.h"
#include "Materials/MaterialExpressionCustom.h"


#include "WKColorKeyRemover.generated.h"

UCLASS(collapsecategories, hidecategories=Object)
class PUREVR_API UWKColorKeyRemover : public UMaterialExpression
{
    GENERATED_BODY()

public:

    UPROPERTY(Instanced)
    UMaterialExpressionCustom* CustomExpression = nullptr;

    UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    FExpressionInput InputColorA;

    UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    FExpressionInput InputColorB;

    UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    FExpressionInput epsilonDistance;

    virtual void PostInitProperties() override
    {
        Super::PostInitProperties();
        SetupInputsOnConstruct();
        SetupOutputsOnConstruct();
        SetupInternalExpressionsOnConstruct();
    }

    virtual void SetupInputsOnConstruct();
    virtual void SetupOutputsOnConstruct();
    virtual void SetupInternalExpressionsOnConstruct();

#if WITH_EDITOR

    



    //virtual FString NodeName() const override { return TEXT("UWKColorKeyRemover"); }
    virtual void GetCaption(TArray<FString>& OutCaptions) const override
    {
        OutCaptions.Empty();
        OutCaptions.Add(TEXT("WK Color Key Remover"));
    }

    virtual int32 Compile(FMaterialCompiler* Compiler, int32 OutputIndex) override;

    // Inputs
    //virtual const TArray<FExpressionInput*> GetInputs() override;
    //virtual FExpressionInput* GetInput(int32 Index) override;
    //virtual FName GetInputName(int32 Index) const override;

    // Outputs
    virtual uint32 GetOutputType(int32 OutputIndex) override;
#endif
};