#include "WKColorMixerByScalar.h"

void UWKColorMixerByScalar::SetupInputsOnConstruct()
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
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("distance") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("maxDistance") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("colorMin") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("colorMax") });
    }
}

void UWKColorMixerByScalar::SetupOutputsOnConstruct()
{
    if (CustomExpression)
    {
        CustomExpression->OutputType = CMOT_Float4; 
    }
}

void UWKColorMixerByScalar::SetupInternalExpressionsOnConstruct()
{
    if (CustomExpression)
    {
        // //epsilon = 0.01;
        CustomExpression->Code = TEXT(R"(
            if(maxDistance <= 0.0){
                maxDistance = 1; //prevent div by zero
            }

            float scalar = distance / maxDistance;
            float3 scaledMin = colorMax * scalar; //closer means LESS MAX COLOR 
            float3 scaledMax = colorMin * scalar; //closer means LESS MAX COLOR 
            float3 scaled = scaledMin + scaledMax;
            return float4(scaled, 1.0);
        )");
    }
}

int32 UWKColorMixerByScalar::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{

    if (!CustomExpression)
        return INDEX_NONE;

    //eingänge bauen
    if(CustomExpression->Inputs.Num() >= 3){
        CustomExpression->Inputs[0].Input = distance;
        CustomExpression->Inputs[1].Input = maxDistance;
        CustomExpression->Inputs[2].Input = colorMin;
        CustomExpression->Inputs[3].Input = colorMax;
    }
    

    return CustomExpression->Compile(Compiler, OutputIndex);

}

uint32 UWKColorMixerByScalar::GetOutputType(int32 OutputIndex)
{
    return MCT_Float4; // RGB raw Color Output
}