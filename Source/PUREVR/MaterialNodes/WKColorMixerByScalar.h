#pragma once

#include "CoreMinimal.h"
#include "MaterialCompiler.h"
#include "Materials/MaterialExpressionCustom.h"


#include "WKColorMixerByScalar.generated.h"

UCLASS(collapsecategories, hidecategories=Object)
class PUREVR_API UWKColorMixerByScalar : public UMaterialExpression
{
    GENERATED_BODY()

public:

    UPROPERTY(Instanced)
    UMaterialExpressionCustom* CustomExpression = nullptr;

    
    // --- to make scalar ---
    UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    FExpressionInput distance;
    
    UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    FExpressionInput maxDistance;

    // --- max and min color ---
    UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    FExpressionInput colorMin;

    UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    FExpressionInput colorMax;







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
        OutCaptions.Add(TEXT("WK Color Mix by scalar Shader"));
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