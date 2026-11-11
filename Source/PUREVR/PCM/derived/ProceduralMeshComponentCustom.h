#pragma once

#include "CoreMinimal.h"
#include "PUREVR/PCM/derived/MeshData.h"

#include "KismetProceduralMeshLibrary.h"
#include "ProceduralMeshComponent.h"
#include "ProceduralMeshComponentCustom.generated.h"



UCLASS()
class PUREVR_API UProceduralMeshComponentCustom : public UProceduralMeshComponent {

    GENERATED_BODY()

public:
    
    //will generate the meshdata.
    void UpdateMesh(TArray<FVector> &anchors);
    void UpdateMesh(TArray<FVector> &anchors, float widthLine);
    void ClearMesh();
    void OverrideMeshDataAndUpdateMesh(MeshData &newData);

    void ApplyMaterial(UMaterialInstanceDynamic *material);
    void ApplyMaterial(
        UMaterialInstanceDynamic *Material,
        int layer
    );

    //sets location of mesh colored marking but not color
    void SetEndEffectorPosition(FVector &pos);

    //sets location of mesh colored marking and color based on distance
    void SetEndEffectorPosition(FVector &pos, float distanceTarget);



private:
    void updateMesh();
    void updateMesh(
        MeshData &otherMesh, // MUST BE SAVED IN A VALUE CLASS SCOPE SOMEWHERE!
        int layer,
        bool bIsRaycastMesh);

    MeshData data;
    UMaterialInstanceDynamic *MaterialSaved = nullptr;
    
    TArray<FVector> CreateQuad(
        FVector &pos,
        FVector &dir,
        float widthLine
    );

    FMatrix MakeTransform(FVector &pos, FVector &dir);
    void TransformVertecies(
        TArray<FVector> &vertecies,
        FMatrix &M
    );

    void AppendCube(
        MeshData &data,
        FVector &prevPosition,
        FVector &currentPosition,
        FVector &nextPosition,
        float widthLine
    );

    int ModIndex(int i, TArray<FVector> &array);
};
