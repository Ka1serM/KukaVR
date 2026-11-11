#include "WKGridShader.h"

void UWKGridShader::SetupInputsOnConstruct()
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
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("sizeSquare") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("time") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("speed") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("UVin") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("ColorIn") });
        
    }
}

void UWKGridShader::SetupOutputsOnConstruct()
{
    if (CustomExpression)
    {
        CustomExpression->OutputType = CMOT_Float4; 
    }
}

void UWKGridShader::SetupInternalExpressionsOnConstruct()
{
    if (CustomExpression)
    {
        // //epsilon = 0.01;
        CustomExpression->Code = TEXT(R"(
            if(sizeSquare <= 0.0){
                sizeSquare = 10; //einfach so.
            }

            float sizeSquareFlip = 1 / sizeSquare;


            float2 uvMod = fmod(UVin * sizeSquareFlip, 1.0); 

            
            float lineThickness = 0.05;

            //step(a,b): //0 oder 1 wenn a < b
            float inLine = step(uvMod.x, lineThickness) + step(uvMod.y, lineThickness); 
            inLine = saturate(inLine); //clamp auf 0 bis 1


            //lol

            //Color output
            float4 BaseColor = float4(0, 0, 0, 0);
            float4 color = lerp(BaseColor, float4(ColorIn, 1.0), inLine);
            return color;
        )");
    }
}

int32 UWKGridShader::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{

    if (!CustomExpression)
        return INDEX_NONE;

    //eingänge bauen
    if(CustomExpression->Inputs.Num() >= 5){
        CustomExpression->Inputs[0].Input = sizeSquare;
        CustomExpression->Inputs[1].Input = time;
        CustomExpression->Inputs[2].Input = speed;
        CustomExpression->Inputs[3].Input = UVin;
        CustomExpression->Inputs[4].Input = ColorIn;
    }
    

    return CustomExpression->Compile(Compiler, OutputIndex);

}

uint32 UWKGridShader::GetOutputType(int32 OutputIndex)
{
    return MCT_Float4; // RGBA Color Output
}