#include "WKRadialOpacity.h"

void UWKRadialOpacity::SetupInputsOnConstruct()
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
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("radius") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("WorldPositionVertex") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("WorldPositionOrigin") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("WorldPositionSelf") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("WorldPositionObject") });
        CustomExpression->Inputs.Add(FCustomInput{ TEXT("NormalPlane") });

    }
}

void UWKRadialOpacity::SetupOutputsOnConstruct()
{
    if (CustomExpression)
    {
        CustomExpression->OutputType = CMOT_Float1; 
    }
}

void UWKRadialOpacity::SetupInternalExpressionsOnConstruct()
{
    if (CustomExpression)
    {
        //hier todo:
        //normale aus material node, bleibt eh gleich.



        // //epsilon = 0.01;
        CustomExpression->Code = TEXT(R"(
            
            //aus hessischer normal form (Q auf Ebene)
            //Ex = n(P-Q) = 0 //auf ebene wenn skalarprodukt = 0
            //n((A + t(B-A))-Q) = 0
            //nA + n t(B-A) -nQ = 0
            //n t(B-A) = nQ - nA
            //t * n(B-A) = nQ - nA
            //t = (nQ - nA) / (n(B-A))

            float3 AB = WorldPositionObject - WorldPositionOrigin; //AB = B - A
            float nAB = NormalPlane.x * AB.x + NormalPlane.y * AB.y + NormalPlane.z * AB.z; //DOT
            
            //DotProduct(NormalPlane, AB);
            if(abs(nAB) < 0.00001){
                return 0.0;
            }
            float nQ = NormalPlane.x * WorldPositionSelf.x + NormalPlane.y * WorldPositionSelf.y + NormalPlane.z * WorldPositionSelf.z; //DOT//DotProduct(NormalPlane, WorldPositionSelf);
            float nA = NormalPlane.x * WorldPositionOrigin.x + NormalPlane.y * WorldPositionOrigin.y + NormalPlane.z * WorldPositionOrigin.z;//DotProduct(NormalPlane, WorldPositionOrigin);
            float nominator = nQ - nA;
            float t = nominator / nAB;
            

            float3 hp = WorldPositionOrigin + AB * t;
            float3 connectHPtoCenter = WorldPositionVertex - hp;


            float distSquared = connectHPtoCenter.x * connectHPtoCenter.x + 
                                connectHPtoCenter.y * connectHPtoCenter.y + 
                                connectHPtoCenter.z * connectHPtoCenter.z;

            float distMaxSquared = radius * radius;
            if(distSquared < distMaxSquared){
                float frac = distSquared / distMaxSquared; //scalar = distPart / distAll

                //flip (idk why, dont care :-) )
                frac = 1.0 - frac;

                if(frac < 0.0){
                    return 0.0; 
                }
                if(frac > 1.0){
                    return 1.0;
                }
                
                return frac;
            }
            return 0.0;
        )");
    }
}

int32 UWKRadialOpacity::Compile(FMaterialCompiler* Compiler, int32 OutputIndex)
{

    if (!CustomExpression)
        return INDEX_NONE;

    //eingänge bauen
    if(CustomExpression->Inputs.Num() >= 6){
        CustomExpression->Inputs[0].Input = radius;
        CustomExpression->Inputs[1].Input = WorldPositionVertex;
        CustomExpression->Inputs[2].Input = WorldPositionOrigin;
        CustomExpression->Inputs[3].Input = WorldPositionSelf;
        CustomExpression->Inputs[4].Input = WorldPositionObject;
        CustomExpression->Inputs[5].Input = NormalPlane;
    }
    

    return CustomExpression->Compile(Compiler, OutputIndex);

}

uint32 UWKRadialOpacity::GetOutputType(int32 OutputIndex)
{
    return MCT_Float1; // CMOT_Float1; // OpacityMask
}