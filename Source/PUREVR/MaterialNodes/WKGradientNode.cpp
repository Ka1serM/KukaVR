#include "WKGradientNode.h"

void UWKGradientNode::SetupInputsOnConstruct()
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
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("zCoordIn") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("height") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("ColorIn") });
        
    }
}

void UWKGradientNode::SetupOutputsOnConstruct()
{
    if (CustomExpression)
    {
        CustomExpression->OutputType = CMOT_Float4; 
    }
}

void UWKGradientNode::SetupInternalExpressionsOnConstruct()
{
    if (CustomExpression)
    {
        // //epsilon = 0.01;
        CustomExpression->Code = TEXT(R"(
            if(height <= 0.0){
                height = 10; //einfach so mi height
            }

            float heightFrac = 1 / height;
            float zNormalized = zCoordIn * heightFrac;
    
            float4 BaseColor = float4(0, 0, 0, 0);
            if(zNormalized > 1.0){
                return BaseColor;
            }
            
            zNormalized = 1.0 - zNormalized;

            // Color output
            float4 color = lerp(BaseColor, ColorIn, zNormalized);
            return color;
        )");
    }
}

int32 UWKGradientNode::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{

    if (!CustomExpression)
        return INDEX_NONE;

    //eingänge bauen
    if(CustomExpression->Inputs.Num() >= 2){
        CustomExpression->Inputs[0].Input = zCoordIn;
        CustomExpression->Inputs[1].Input = height;
        CustomExpression->Inputs[2].Input = ColorIn;
    }
    

    return CustomExpression->Compile(Compiler, OutputIndex);

}

uint32 UWKGradientNode::GetOutputType(int32 OutputIndex)
{
    return MCT_Float4; // RGBA Color Output
}