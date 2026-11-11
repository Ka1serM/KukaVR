#include "ProceduralMeshComponentCustom.h"
#include "DebugPlugin/DebugHelper.h"







void UProceduralMeshComponentCustom::ClearMesh(){

	TArray<FVector> anchors;
	anchors.Add(FVector(0, 0, 0));
	anchors.Add(FVector(0, 0, 0));
	UpdateMesh(anchors, 0.0f);
}

void UProceduralMeshComponentCustom::UpdateMesh(TArray<FVector> &anchors){
	UpdateMesh(anchors, 10.0f);
}

void UProceduralMeshComponentCustom::UpdateMesh(TArray<FVector> &anchors, float widthLine){
	MeshData newData;
	newData.MofidyEpsilon(0.0000000001f);

	//create meshdata from anchors
	if(anchors.Num() > 1){
		//FVector dir(1,0,0);
		//TArray<FVector> prevQuad = CreateQuad(anchors[0], dir, widthLine);
		for (int i = 0; i < anchors.Num() + 1; i++)
		{
			FVector prev = anchors[ModIndex(i-1, anchors)];
			FVector current = anchors[ModIndex(i, anchors)];
			FVector next = anchors[ModIndex(i + 1, anchors)];
			AppendCube(
				newData,
				prev,
				current,
				next,
				widthLine
			);
			/*
			dir = anchors[i] - anchors[i-1]; //AB = B - A
			TArray<FVector> currentQuad = CreateQuad(anchors[i], dir, widthLine);
			/ *
			1-->2
			|	|
			0<--3
			* /
			newData.appendCube(
				prevQuad,
				currentQuad
			);

			FVector dirNext = anchors[(i + 1) % anchors.Num()] - anchors[i];
			TArray<FVector> currentQuadToNextRotation = CreateQuad(anchors[i], dirNext, widthLine);
			newData.appendCube(
				currentQuad,
				currentQuadToNextRotation
			);



			prevQuad = currentQuadToNextRotation;*/
		}

		/*if(bLoop){
			FVector dirNext = anchors[anchors.Num() - 1] - anchors[i]; //AB = B - A
			TArray<FVector> currentQuadToNextRotation = CreateQuad(anchors[i], dirNext, widthLine);
			newData.appendCube(
				currentQuad,
				currentQuadToNextRotation
			);
			TArray<FVector> firstQuad = CreateQuad(anchors[0], dir, widthLine);
			newData.appendCube(
				prevQuad,
				firstQuad
			);
		}*/
	}
	newData.flipWindingOrder();
	newData.calculateNormals();
	OverrideMeshDataAndUpdateMesh(newData);
}

int UProceduralMeshComponentCustom::ModIndex(int i, TArray<FVector> &array){
	if(array.Num() == 0){
		return -1;
	}
	if(i >= 0){
		return i % array.Num();
	}
	if(i < 0){
		return (array.Num() + i) % array.Num();
	}
	return -1;
}

void UProceduralMeshComponentCustom::AppendCube(
	MeshData &newData,
	FVector &prevPosition,
	FVector &currentPosition,
	FVector &nextPosition,
	float widthLine
){
	FVector dir = currentPosition - prevPosition; //AB = B - A
	TArray<FVector> currentQuad = CreateQuad(currentPosition, dir, widthLine);
	TArray<FVector> prevQuad = CreateQuad(prevPosition, dir, widthLine);
	/*
	1-->2
	|	|
	0<--3
	*/
	newData.appendCube(
		prevQuad,
		currentQuad
	);

	FVector dirNext = nextPosition - currentPosition;
	TArray<FVector> currentQuadToNextRotation = CreateQuad(currentPosition, dirNext, widthLine);
	newData.appendCube(
		currentQuad,
		currentQuadToNextRotation
	);
}

TArray<FVector> UProceduralMeshComponentCustom::CreateQuad(
	FVector &pos, 
	FVector &dir, 
	float widthLine
){
	dir = dir.GetSafeNormal();
	/*
	1-->2
	|	|
	0<--3
	*/
	float width = widthLine / 2.0f;
	TArray<FVector> vertecies = {
		FVector(0.0f, -width, -width),
		FVector(0.0f, -width, width),
		FVector(0.0f, width, width),
		FVector(0.0f, width, -width)
	};

	FMatrix M = MakeTransform(pos, dir);
	TransformVertecies(vertecies, M);
	return vertecies;
}


FMatrix UProceduralMeshComponentCustom::MakeTransform(FVector &pos, FVector &dir){
	FMatrix M = FRotationMatrix::MakeFromX(dir);
	M.SetOrigin(pos);   //Darf so, weil M = T * R.<--lese richtung-- R in SO3 berührt T nicht.
	return M;
}

void UProceduralMeshComponentCustom::TransformVertecies(
	TArray<FVector> &vertecies,
	FMatrix &M
){
	//FRotationMatrix R = FRotationMatrix::MakeFromX(dir);
	for (int i = 0; i < vertecies.Num(); i++){
		vertecies[i] = M.TransformPosition(vertecies[i]);
		//vertecies[i] += pos;
	}
}


	




void UProceduralMeshComponentCustom::OverrideMeshDataAndUpdateMesh(MeshData &newData){
	data = newData;
	updateMesh();
}


void UProceduralMeshComponentCustom::SetEndEffectorPosition(FVector &Pos){
	if(MaterialSaved){
		MaterialSaved->SetVectorParameterValue(
			FName("EndEffector"),
			FLinearColor(Pos.X, Pos.Y, Pos.Z, 0.f)
		);
	}
}

void UProceduralMeshComponentCustom::SetEndEffectorPosition(FVector &pos, float distanceTarget){
	if(MaterialSaved){
		SetEndEffectorPosition(pos);

		//DOES NOT WORK RN!
		MaterialSaved->SetScalarParameterValue(
			FName("EndEffectorDistance"),
			distanceTarget
		);
	}
	
}

void UProceduralMeshComponentCustom::ApplyMaterial(UMaterialInstanceDynamic *material){
	if(material){
		ApplyMaterial(material, 0);
	}
}

/// @brief applys a material to the whole component at passed index slot
/// @param ProceduralMeshComponent mesh to apply for
/// @param Material material to set
/// @param layer layer to apply for / index
void UProceduralMeshComponentCustom::ApplyMaterial(
    UMaterialInstanceDynamic* Material,
    int layer
) {
	if (Material != nullptr) {
		// Apply the material to the first material slot (index 0) of the procedural mesh
		SetMaterial(layer, Material);
		MaterialSaved = Material;
	}
}




void UProceduralMeshComponentCustom::updateMesh(){
	updateMesh(
		data,
		0,
		false
	);
}

/// @brief replaces the mesh layer for an mesh component
/// caution: mesh section is recreated because modifying the triangle buffer is not allowed
/// when will to update an mesh!
/// @param meshcomponent 
/// @param otherMesh 
/// @param layer 
/// @param enableCollision 
void UProceduralMeshComponentCustom::updateMesh(
    MeshData &otherMesh, //MUST BE SAVED IN A VALUE CLASS SCOPE SOMEWHERE!
    int layer,
    bool bIsRaycastMesh
){
    if(otherMesh.getVerteciesRef().Num() == 0){
        return;
    }


    /**
     * example: 
     * 
    Mesh->CreateMeshSection(
        layer, 
        newvertecies, 
        this->triangles, 
        normals, 
        UV0, 
        VertexColors, 
        Tangents, 
        true
    );*/

	//meshcomponent.ClearMeshSection(layer);
    CreateMeshSection(
        layer, 
        otherMesh.getVerteciesRef(),//newvertecies, 
        otherMesh.getTrianglesRef(),//this->triangles, 
        otherMesh.getNormalsRef(),//normals, 
        otherMesh.getUV0Ref(),//UV0, 
        otherMesh.getVertexColorsRef(),//VertexColors, 
        otherMesh.getTangentsRef(),//Tangents, 
        bIsRaycastMesh
    );

}
