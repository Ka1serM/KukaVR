#include "WKColorKeyRemover.h"

void UWKColorKeyRemover::SetupInputsOnConstruct()
{
    UMaterial* OuterMaterial = GetTypedOuter<UMaterial>();
    if (OuterMaterial && !CustomExpression)
    {
        CustomExpression = NewObject<UMaterialExpressionCustom>(
            this,
            UMaterialExpressionCustom::StaticClass(),
            NAME_None,
            RF_Transactional);
    }

    if (CustomExpression)
    {
        CustomExpression->Inputs.Reset();
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("ColorA") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("ColorB") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("epsilonDistance") });
        
    }
}

void UWKColorKeyRemover::SetupOutputsOnConstruct()
{
    if (CustomExpression)
    {
        CustomExpression->OutputType = CMOT_Float1; 
    }
}

void UWKColorKeyRemover::SetupInternalExpressionsOnConstruct()
{
    if (CustomExpression)
    {
        // //epsilon = 0.01;
        CustomExpression->Code = TEXT(R"(
            float deltaR = abs(ColorA.r - ColorB.r);
            float deltaG = abs(ColorA.g - ColorB.g);
            float deltaB = abs(ColorA.b - ColorB.b);

            float epsilon = epsilonDistance;
            if(deltaR < epsilon && deltaG < epsilon && deltaB < epsilon)
                return 0.0;
            else
                return 1.0;
        )");
    }
}

int32 UWKColorKeyRemover::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{

    if (!CustomExpression)
        return INDEX_NONE;

    // Eingänge kompilieren
    CustomExpression->Inputs[0].Input = InputColorA;
    CustomExpression->Inputs[1].Input = InputColorB;

    if(CustomExpression->Inputs.Num() >= 3){
        CustomExpression->Inputs[2].Input = epsilonDistance;
    }

    return CustomExpression->Compile(Compiler, OutputIndex);

}

uint32 UWKColorKeyRemover::GetOutputType(int32 OutputIndex)
{
    return MCT_Float1; // CMOT_Float1; // OpacityMask
}