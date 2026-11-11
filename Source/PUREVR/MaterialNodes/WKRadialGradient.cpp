/*
UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    FExpressionInput PositionIn;

    UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    FExpressionInput ColorIn;
    
    UPROPERTY(EditAnywhere, meta = (RequiredInput = "true"))
    FExpressionInput radiusIn;

*/

#include "WKRadialGradient.h"

void UWKRadialGradient::SetupInputsOnConstruct()
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
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("PositionIn") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("ColorIn") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("radiusIn") });
         CustomExpression->Inputs.Add(FCustomInput{ TEXT("VertexIn") });
        
    }

}

void UWKRadialGradient::SetupOutputsOnConstruct()
{
    if (CustomExpression)
    {
        CustomExpression->OutputType = CMOT_Float4; 
    }
}

void UWKRadialGradient::SetupInternalExpressionsOnConstruct()
{
    if (CustomExpression)
    {
        // //epsilon = 0.01;
        CustomExpression->Code = TEXT(R"(
            float radius2 = radiusIn * radiusIn;
            if(radius2 <= 0.0001){
                radius2 = 0.0001;
            }

            float dist2 = dot(VertexIn - PositionIn, VertexIn - PositionIn); //quadrat distanz ist das selbe wie dot.
            if(dist2 > radius2){
                return float4(0, 0, 0, 0);
            }

            float fraction = dist2 / (radius2);
            fraction = saturate(fraction);
            fraction = 1.0 - fraction;

            float4 BaseColor = float4(0, 0, 0, 0);
            //float4 color = lerp(BaseColor, float4(ColorIn,1.0), fraction);
            float4 color = lerp(BaseColor, ColorIn, fraction); //float 4 in colorIn
            return color;

            //-der kündigt an was so in der klausur kommt. Weiss net ob heute oder nächste mal, aber der
            //sagt was so wichtig ist.
            //aber eigentlich halt alles so.
            //war immer viel wiederholung, wird net so schwer
            
        )");
    }
}

int32 UWKRadialGradient::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{

    if (!CustomExpression)
        return INDEX_NONE;

    //eingänge bauen
    if(CustomExpression->Inputs.Num() >= 3){
        CustomExpression->Inputs[0].Input = PositionIn;
        CustomExpression->Inputs[1].Input = ColorIn;
        CustomExpression->Inputs[2].Input = radiusIn;
        CustomExpression->Inputs[3].Input = VertexIn;
        
    }
    

    return CustomExpression->Compile(Compiler, OutputIndex);

}

uint32 UWKRadialGradient::GetOutputType(int32 OutputIndex)
{
    return MCT_Float4; // RGBA Color Output
}