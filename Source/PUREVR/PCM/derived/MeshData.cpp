// Fill out your copyright notice in the Description page of Project Settings.


#include "MeshData.h"

#include "DebugPlugin/DebugHelper.h"
#include "KismetProceduralMeshLibrary.h"


#include <algorithm>
#include <set>

MeshData::MeshData()
{
}


/// @brief creates the mesh from triangles and vertecies AND calculates the normals!
/// @param verteciesIn 
/// @param trianglesIn 
MeshData::MeshData(TArray<FVector> &&verteciesIn, TArray<int> &&trianglesIn){
    setVertecies(MoveTemp(verteciesIn));
    setTriangles(MoveTemp(trianglesIn));
    calculateNormals();
    
}

void MeshData::rebuild(TArray<FVector> &&verteciesIn, TArray<int> &&trianglesIn){
    clearMesh();
    setVertecies(MoveTemp(verteciesIn));
    setTriangles(MoveTemp(trianglesIn));
    calculateNormals();
    
}



MeshData::~MeshData()
{
    clearMesh();
}


/// @brief copy constructor gets called if passing by value
/// @param other 
MeshData::MeshData(const MeshData &other){
    *this = other;
}

/// @brief abusing the assign operator to create a deepcopy of the data
/// @param other 
/// @return 
MeshData& MeshData::operator=(const MeshData &other){
    if(&other != this){
        clearMesh(); //clear mesh data before adding any data!

        
        //broken?
        
        vertecies = other.vertecies;
        triangles = other.triangles;
        normals = other.normals;
        UV0 = other.UV0;
        Tangents = other.Tangents;
        VertexColors = other.VertexColors;


     

        /*
        for (int i = 0; i < other.vertecies.Num(); i++){
            vertecies.Add(other.vertecies[i]);
        }
        for (int i = 0; i < other.triangles.Num(); i++){
            triangles.Add(other.triangles[i]);
        }
        for (int i = 0; i < other.normals.Num(); i++){
            normals.Add(other.normals[i]);
        }
        for (int i = 0; i < other.UV0.Num(); i++){
            UV0.Add(other.UV0[i]);
        }
        for (int i = 0; i < other.Tangents.Num(); i++){
            Tangents.Add(other.Tangents[i]);
        }
        for (int i = 0; i < other.VertexColors.Num(); i++){
            VertexColors.Add(other.VertexColors[i]);
        }
        */
        

        
    }
    return *this;
}


/// @brief clean up data completely
void MeshData::clearMesh(){
    vertecies.Empty();
    triangles.Empty();
    normals.Empty();

    UV0.Empty();
    Tangents.Empty();
    VertexColors.Empty();
}

/// @brief clears the normals 
void MeshData::clearNormals(){
    normals.Empty();
}

/**
 * --- apply data ---
 */

/// @brief calculates the normals and applies it to the vertecies
void MeshData::calculateNormals(){
    
    
    clearNormals();
    normals.SetNum(vertecies.Num());
    for (int i = 2; i < triangles.Num(); i += 3) {
        int32 Index0 = triangles[i - 2];
        int32 Index1 = triangles[i - 1];
        int32 Index2 = triangles[i];

        if(isValidVertexIndex(Index0, Index1, Index2)){
            FVector Edge1 = vertecies[Index1] - vertecies[Index0];
            FVector Edge2 = vertecies[Index2] - vertecies[Index0];
            FVector Normal = FVector::CrossProduct(Edge1, Edge2).GetSafeNormal();

            normals[Index0] = Normal;
            normals[Index1] = Normal;
            normals[Index2] = Normal;
        }

        
    }

    UKismetProceduralMeshLibrary::CalculateTangentsForMesh(vertecies, triangles, UV0, normals, Tangents);
    
    
}

void MeshData::flipNormals(){
    for (int32 i = 0; i < normals.Num(); i++){
        normals[i] *= -1.0f;
    }
}

void MeshData::flipWindingOrder(){
    for (int32 t = 2; t < triangles.Num(); t += 3){
        /*
        1->2
        |
        0

        to 
        1<-2
        |
        0
        */
        int32 copy = triangles[t - 2];
        triangles[t - 2] = triangles[t - 1];
        triangles[t - 1] = copy;
    }
}



/// @brief sets the data for all vertecies, pass by r value reference
/// @param vertecies veretecies to set, be carefull with overriding
void MeshData::setVertecies(TArray<FVector> &&verteciesIn){
    vertecies = MoveTemp(verteciesIn);  // Move the data instead of copying, creating an r value

}
/// @brief sets the data for all triangles, pass by r value reference
/// @param trianglesIn triangles to set for the mesh
void MeshData::setTriangles(TArray<int32> &&trianglesIn){
    triangles = MoveTemp(trianglesIn);
}

/// @brief join another mesh, vertecies add, triangles added with offset added to index
/// all data will be COPIED, regardless of duplicated vertecies, normals and triangles
/// use appendEfficent(Meshdata) to get more data efficent results
/// but with computational overhead!
/// @param other 
void MeshData::append(MeshData &other)
{
    join(
        other.getVerteciesRef(),
        other.getTrianglesRef(), 
        other.getNormalsRef(), 
        other.getUV0Ref()
    );
}

void MeshData::join(
    TArray<FVector> &verteciesRef, 
    TArray<int32> &trianglesRef, 
    TArray<FVector> &normalsin,
    TArray<FVector2D> &uvrefin
){
    int triangleOffset = vertecies.Num(); //beim vertex count offset starten!

    //copy triangles, apply offset
    for (int i = 0; i < trianglesRef.Num(); i++){
        int32 copy = trianglesRef[i];
        copy += triangleOffset;
        //--> offset index weil buffer sich verschiebt insgesamt um size des aktuellen
        //    vertex buffers
        triangles.Add(copy);
    }

    // --- will introduce resize overhead. Dont use this method. ---

    //copy vertecies
    /*
    for(int i = 0; i < verteciesRef.Num(); i++){
        FVector &ref = verteciesRef[i];
        vertecies.Add(ref);
    }

    //copy normals
    for(int i = 0; i < normalsin.Num(); i++){
        FVector &ref = normalsin[i];
        normals.Add(ref);
    }

    //copy uvs
    for(int i = 0; i < uvrefin.Num(); i++){
        UV0.Add(uvrefin[i]);
    }*/

    if(verteciesRef.Num() > 0){
        int32 sizebuffer = vertecies.Num();
        vertecies.SetNum(sizebuffer + verteciesRef.Num());
        for (int i = 0; i < verteciesRef.Num(); i++)
        {
            FVector &ref = verteciesRef[i];
            vertecies[sizebuffer + i] = ref;
        }
    }

    if(normalsin.Num() > 0){
        int32 sizebuffer = normals.Num();
        normals.SetNum(sizebuffer + normalsin.Num());
        for (int i = 0; i < normalsin.Num(); i++)
        {
            FVector &ref = normalsin[i];
            normals[sizebuffer + i] = ref;
        }
    }

    if(uvrefin.Num() > 0){
        int32 sizebuffer = UV0.Num();
        UV0.SetNum(sizebuffer + uvrefin.Num());
        for (int i = 0; i < uvrefin.Num(); i++)
        {
            FVector2D &ref = uvrefin[i];
            UV0[sizebuffer + i] = ref;
        }
    }

    
}



/*

---- manual adding section ----

*/
void MeshData::append(
    FVector &a, 
    FVector &b, 
    FVector &c
){
    TArray<FVector> _vertecies;
    TArray<int32> _triangles;
    buildTriangle(a, b, c, _vertecies, _triangles);
    MeshData appendMesh(
        MoveTemp(_vertecies),
        MoveTemp(_triangles)
    );
    append(appendMesh);
}


void MeshData::append(
    FVector &a, 
    FVector &b, 
    FVector &c,
    FVector &d
){
    append(a, b, c);
    append(a, c, d);
}

void MeshData::appendDoublesided(
    FVector &a, 
    FVector &b, 
    FVector &c
){
    /*
    1 2
    0 
    */
   //kann nicht weiter vereinfacht werden damit jene duplikat vertecies auch andere normalen haben
    append(a, b, c); 
    append(a, c, b); 
}

void MeshData::appendDoublesided(
    FVector &a,
    FVector &b,
    FVector &c,
    FVector &d
){
    /*
    b c
    a d
    */
    appendDoublesided(a, b, c);
    appendDoublesided(a, c, d);
}


void MeshData::appendDoubleSidedTriangleBuffer(
    std::vector<FVector> &buffer
){  
    if(buffer.size() < 3){
        return;
    }

    for (int i = 0; i < buffer.size() - 3; i += 3)
    {
        appendDoublesided(
            buffer[i],
            buffer[i + 1],
            buffer[i + 2]
        );
    }
}




void MeshData::buildTriangle(
    FVector &a, 
    FVector &b, 
    FVector &c,
    TArray<FVector> &output,
    TArray<int32> &trianglesOutput
){
    //add triangles (new more readable)
    int32 offset = output.Num();
    trianglesOutput.Add(0 + offset); // 0th vertex in the first triangle
    trianglesOutput.Add(1 + offset); // 1st vertex in the first triangle
    trianglesOutput.Add(2 + offset); // 2nd vertex in the first triangle

    output.Add(a);
    output.Add(b);
    output.Add(c);
}

void MeshData::appendUvs(
    FVector2D &a,
    FVector2D &b,
    FVector2D &c
){
    UV0.Add(a);
    UV0.Add(b);
    UV0.Add(c);
}

void MeshData::appendUvs(
    FVector2D &a,
    FVector2D &b,
    FVector2D &c,
    FVector2D &d
){
    appendUvs(a, b, c);
    appendUvs(a, c, d);
}

void MeshData::appendUvsDoubleSided(
    FVector2D &a,
    FVector2D &b,
    FVector2D &c
){
    appendUvs(a, b, c);
    appendUvs(a, c, b);
}
void MeshData::appendUvsDoubleSided(
    FVector2D &a,
    FVector2D &b,
    FVector2D &c,
    FVector2D &d
){
    appendUvsDoubleSided(a, b, c);
    appendUvsDoubleSided(a, c, d);
}

/// @brief offsets all vertecies in a given direction
/// @param offset 
void MeshData::offsetAllvertecies(FVector &offset){
    for (int i = 0; i < vertecies.Num(); i++){
        vertecies[i] += offset;
    }
}





/// @brief appends the vertecies to the meshbuffer and creates triangles with n latest vertecies
/// to connect each vertex of the passed vector (allows for smooth 
/// shading and not duplicating vertecies)
/// caution: vec.size() should not be greater than the current vertex count, unless
/// this mesh data vertexcount is 0
/// will connect to the latest vec.size() vertecies, gaps cannot be detected
/// @param vec vector to connect, expected in some correct order 

void MeshData::appendVertecies(std::vector<FVector> &vec){
    if(vec.size() <= 0){
        return;
    }
    if(vertecies.Num() == 0){
        for (int i = 0; i < vec.size(); i++)
        {
            vertecies.Add(vec[i]);
        }
        return;
    }
    

    //soll sich an die letzten vertecies anheften
    //aber ohne vertecies zu duplizieren

    int32 startingIndex = vertecies.Num() - vec.size(); // 0
    if(vertecies.Num() < vec.size()){
        fillUpMissingVertecies(startingIndex); //difference abs
        startingIndex = 0;
    }

    int32 offset = vertecies.Num(); //stimmt so, getestet
    for (int i = 0; i < vec.size(); i++)
    {
        vertecies.Add(vec[i]);
    }

    //für jeden vertex den man hinzugefügt hat, aus 3 (+2) dreiecke basteln
    for (int i = 0; i < vec.size(); i++){

        
        // so.
        /*
        1 2
        0 3
        */
        // 1 triangle
        if(true){
            //das hier sieht eher nicht aus wie die 012 023 winding order, aber sie ist korrekt
            //nach aussen gedreht
            int next = (i + 1) % vec.size();
            triangles.Add(startingIndex + i); //lower(0)
            triangles.Add(startingIndex + next); //lower(3)
            triangles.Add(offset + i); //upper(1)
            
            //2 triangle
            triangles.Add(startingIndex + next); //lower (3)
            triangles.Add(offset + next); //upper(2)
            triangles.Add(offset + i); //upper (1)
        }
        
    }
}

/**
 * caution: this function is not tested
 */
/// @brief will fill up vertecies between the last two ones
/// @param count 
void MeshData::fillUpMissingVertecies(int count){
    if(count <= 0){
        return;
    }
    int sizeOfVertexbuffer = vertecies.Num();
    count = std::abs(count);
    if(sizeOfVertexbuffer == 1){
        FVector copy = vertecies[0];
        for (int i = 0; i < count; i++)
        {
            vertecies.Add(copy);
        }
        return;
    }

    //DebugHelper::logMessage("meshData_debug inserted vertecies");

    FVector last = vertecies[sizeOfVertexbuffer - 1];
    FVector prev = vertecies[sizeOfVertexbuffer - 2];
    FVector connect = last - prev; //AB = B - A
    connect /= count;
    vertecies.RemoveAt(vertecies.Num() - 1);
    for (int i = 0; i < count; i++)
    {
        FVector inner = prev + connect * i;
        vertecies.Add(inner);
    }
    vertecies.Add(last);

    //fix triangle buffer offset because of insert
    int vertexIndexTargeted = sizeOfVertexbuffer - 1;
    for (int i = 0; i < triangles.Num(); i++)
    {
        if(triangles[i] == vertexIndexTargeted){
            triangles[i] = vertecies.Num() - 1;
        }
    }
}



void MeshData::closeMeshAtCenter(FVector &center, std::vector<FVector> &vec, bool clockWise){
    //vertecies.Add(center); //jetzt offset index valid
    for (int i = 0; i < vec.size(); i++){
        vertecies.Add(vec[i]);
    }
    closeMeshAtCenter(center, vec.size(), clockWise);
    return;

}

void MeshData::closeMeshAtCenter(FVector &center, int bufferSizeToConnect, bool clockWise){
    int limit = bufferSizeToConnect; //vertex count um an mitte zu binden
    if(limit > vertecies.Num()){
        limit = vertecies.Num();
    }
    int offset = vertecies.Num() - bufferSizeToConnect;
    if(offset < 0){
        offset = 0;
    }
    vertecies.Add(center);

    for (int i = 0; i < limit; i++)
    {
        int next = (i + 1) % limit;
        if(clockWise){
            /*
            1 2
            0
            */
            triangles.Add(offset); //0
            triangles.Add(offset + i); //1
            triangles.Add(offset + next); //2
        }else{
            /*
            2 1
            0
            */
           triangles.Add(offset); //0
           triangles.Add(offset + next); //2
           triangles.Add(offset + i); //1
        }
    }
}












/**
 * --- helper function ---
 */

///@brief searches for the closest index to a vertex and explicitly looks for duplicate vertecies
std::vector<int> MeshData::findClosestIndexWithVertexDuplicatesTo(
    FVector &vertex
){
    std::vector<int> output;

    int indexFound = findClosestIndexTo(vertex);
    if(isValidVertexIndex(indexFound)){
        output.push_back(indexFound);

        FVector comparePos = vertecies[indexFound];

        //search vertex buffer for similar vertecies
        for (int v = 0; v < vertecies.Num(); v++){
            if(
                v != indexFound && 
                FVector::Dist(comparePos, vertecies[v]) <= 1.0f
            ){
                output.push_back(v);
            }
        }
    }

    return output;
}

/// @brief finds the closest index to a vertex, -1 if the vertex buffer is clear
/// @param vertex position to find
/// @return index in vertecies array
int MeshData::findClosestIndexTo(FVector &vertex){
    if(vertecies.Num() == 0){
        return -1;
    }
    int closestIndex = 0;
    float dist = FVector::Dist(vertex, vertecies[0]);

    for (int i = 1; i < vertecies.Num(); i++){
        float newDist = FVector::Dist(vertex, vertecies[i]);
        if(newDist < dist){ //debug keep distance
            dist = newDist;
            closestIndex = i;
        }
    }
    return closestIndex;
}

bool MeshData::isCloseSame(FVector &a, int index){
    if(index < 0 || index >= vertecies.Num()){
        return false;
    }
    return isCloseSame(a, vertecies[index]);
}

bool MeshData::isCloseSame(FVector &a, FVector &b){
    return std::abs(a.X - b.X) <= EPSILON &&
           std::abs(a.Y - b.Y) <= EPSILON &&
           std::abs(a.Z - b.Z) <= EPSILON;
}



int MeshData::findClosestIndexToAndAvoid(FVector &vertex, int indexAvoid){
    std::vector<int> vec = {indexAvoid};
    return findClosestIndexToAndAvoid(vertex, vec);
}

int MeshData::findClosestIndexToAndAvoid(FVector &vertex, std::vector<int> &avoid){
    if(vertecies.Num() == 0){
        return -1;
    }
    int closestIndex = 0;
    float dist = FVector::Dist(vertex, vertecies[0]);

    for (int i = 1; i < vertecies.Num(); i++){

        if(!contains(avoid, i)){
            float newDist = FVector::Dist(vertex, vertecies[i]);
            if(newDist < dist){ //debug keep distance
                dist = newDist;
                closestIndex = i;
            }
        }
    }
    return closestIndex;
}






/// @brief creates a vertex buffer sample of 0123 clockwise dir on XY Pane
/// @param xMax xMax size
/// @param yMax yMax size
/// @return starting from (0,0,0) 0123 clockwise buffer quad
std::vector<FVector> MeshData::create2DQuadVertecies(int xMax, int yMax){
    std::vector<FVector> shape;
    shape.push_back(FVector(0,0,0));
    shape.push_back(FVector(0,yMax,0));
    shape.push_back(FVector(xMax,yMax,0));
    shape.push_back(FVector(xMax,0,0));
    return shape;
}


/**
 * 
 * is tested!
 * 
 */
bool MeshData::IsDegenerateTriangle(int a, int b, int c){
    return a == b || b == c || c == a;
}

void MeshData::appendEfficent(
    FVector &a, 
    FVector &b, 
    FVector &c
){
    int indexA = findClosestIndexTo(a);
    int indexB = findClosestIndexTo(b);
    int indexC = findClosestIndexTo(c);

    if(IsDegenerateTriangle(indexA, indexB, indexC)){
        //DebugHelper::logMessage("Detected Degenerate Triangle! ");
        //return;
    }

    int debugEfficentAdded = 3;

    //add if not found correctly
    if(!isCloseSame(a, indexA)){
        vertecies.Add(a);
        indexA = vertecies.Num() - 1; //0
        debugEfficentAdded--;
    }
    if(!isCloseSame(b, indexB)){
        vertecies.Add(b);
        indexB = vertecies.Num() - 1; //1
        debugEfficentAdded--;
    }
    if(!isCloseSame(c, indexC)){
        vertecies.Add(c);
        indexC = vertecies.Num() - 1; //2
        debugEfficentAdded--;
    }
    //add to triangle buffer
    triangles.Add(indexA);
    triangles.Add(indexB);
    triangles.Add(indexC);

    if(debugEfficentAdded > 0 && false){
        FString message = FString::Printf(TEXT("meshdata efficentAdded %d"), debugEfficentAdded);
        DebugHelper::logMessage(message);
    }
}


void MeshData::appendEfficent(
    FVector &a, 
    FVector &b, 
    FVector &c,
    FVector &d
){
    appendEfficent(a, b, c);
    appendEfficent(a, c, d);
}



/// @brief appends all mesh data, eliminates duplicated vertecies,
/// and recalculates all normals
/// @param other some mesh data
void MeshData::appendEfficent(MeshData &other){

    TArray<int32> &trianglesRef = other.getTrianglesRef();
    for (int i = 0; i < trianglesRef.Num() - 3; i += 3){ //-2 ? (no, -3 is correct)
        int32 v0 = trianglesRef[i];
        int32 v1 = trianglesRef[i + 1];
        int32 v2 = trianglesRef[i + 2];

        if(
            other.isValidVertexIndex(v0) &&
            other.isValidVertexIndex(v1) &&
            other.isValidVertexIndex(v2)
        ){
            FVector &v0vertex = other.vertecies[v0];
            FVector &v1vertex = other.vertecies[v1];
            FVector &v2vertex = other.vertecies[v2];

            appendEfficent(v0vertex, v1vertex, v2vertex);
        }
    }

    calculateNormals();
}

void MeshData::appendEfficentTriangleShapedBuffer(TArray<FVector> &verteciesIn){
    for (int i = 2; i < verteciesIn.Num(); i+=3){
        appendEfficent(
            verteciesIn[i-2],
            verteciesIn[i-1],
            verteciesIn[i]
        );
    }
}



FVector MeshData::createNormal(int v0, int v1, int v2){
    if(isValidVertexIndex(v0) && isValidVertexIndex(v1) && isValidVertexIndex(v2)){
        FVector &a = vertecies[v0];
        FVector &b = vertecies[v1];
        FVector &c = vertecies[v2];

        FVector ab = b - a;
        FVector ac = c - a;
        return FVector::CrossProduct(ab, ac);
    }
    return FVector(0,0,0);
}





bool MeshData::isValidVertexIndex(int i){
    return i >= 0 && i < vertecies.Num();
}

bool MeshData::isValidVertexIndex(int i, int j, int n){
    return isValidVertexIndex(i) && isValidVertexIndex(j) && isValidVertexIndex(n);
}

bool MeshData::isValidTriangleIndex(int i){
    return i >= 0 && i < triangles.Num();
}

bool MeshData::isValidNormalIndex(int i){
    return i >= 0 && i < normals.Num();
}

bool MeshData::isValidNormalIndex(int i, int j, int k){
    return isValidNormalIndex(i) && isValidNormalIndex(j) && isValidNormalIndex(k);
}

/**
 * --- Data references ---
 */

/// @brief returns the reference to the mesh data vertecies, be carefull with modifying
/// @return mesh data vertecies by reference
TArray<FVector> &MeshData::getVerteciesRef(){
    return vertecies;
}

/// @brief returns the reference to the mesh data triangles, be carefull with modifying
/// @return mesh data triangles by reference
TArray<int32> &MeshData::getTrianglesRef(){
    return triangles;
}

TArray<FVector> &MeshData::getNormalsRef(){
    return normals;
}

TArray<FVector2D> &MeshData::getUV0Ref(){
    return UV0;
}
TArray<FProcMeshTangent> &MeshData::getTangentsRef(){
    return Tangents;
}
TArray<FColor> &MeshData::getVertexColorsRef(){
    return VertexColors;
}

const TArray<FVector> &MeshData::getVerteciesRefConst() const {
    return vertecies;
}
const TArray<int32> &MeshData::getTrianglesRefConst() const{
    return triangles;
}
const TArray<FVector> &MeshData::getNormalsRefConst() const{
    return normals;
}
const TArray<FVector2D> &MeshData::getUV0RefConst() const{
    return UV0;
}

const TArray<FProcMeshTangent> &MeshData::getTangentsRefConst() const {
    return Tangents;
}








/**
 * 
 * --- special split mesh function ---
 * 
 */
void MeshData::seperateMeshIntoAllTrianglesDoubleSided(std::vector<MeshData> &meshDataVectorOutput){
    for (int i = 0; i < triangles.Num() - 3; i += 3) {
        int32 Index0 = triangles[i];
        int32 Index1 = triangles[i + 1];
        int32 Index2 = triangles[i + 2];

        if(
            isValidVertexIndex(Index0, Index1, Index2)
        ){
            FVector &v0 = vertecies[Index0];
            FVector &v1 = vertecies[Index1];
            FVector &v2 = vertecies[Index2];
            MeshData newTriangleMesh;
            newTriangleMesh.appendDoublesided(v0, v1, v2); //normals will be calculated in the constructor
            meshDataVectorOutput.push_back(newTriangleMesh);
        }
    }
}



/// @brief will create all triangles possible to split and append it into the vector
/// traingles being lower than the min distance will not be added!
/// @param meshDataVectorOutput 
void MeshData::splitAllTrianglesInHalfAndSeperateMeshIntoAllTrianglesDoubleSided(
    std::vector<MeshData> &meshDataVectorOutput
){
    for (int i = 0; i < triangles.Num() - 3; i += 3) {
        int32 Index0 = triangles[i];
        int32 Index1 = triangles[i + 1];
        int32 Index2 = triangles[i + 2];

        FVector &v0 = vertecies[Index0];
        FVector &v1 = vertecies[Index1];
        FVector &v2 = vertecies[Index2];

        if(canSplit(v0,v1,v2)){
            FVector middle = v1 + 0.5f * (v2 - v1); // gx = A + r(B-A)

            MeshData newTriangleMeshA;
            newTriangleMeshA.appendDoublesided(v0, v1, middle);
            newTriangleMeshA.calculateNormals();
            meshDataVectorOutput.push_back(newTriangleMeshA);

            MeshData newTriangleMeshB;
            newTriangleMeshB.appendDoublesided(v0, middle, v2);
            newTriangleMeshB.calculateNormals();
            meshDataVectorOutput.push_back(newTriangleMeshB);
        }else{
            //DebugHelper::logMessage("DEBUGSPLIT could not split");
        }
    }

}

bool MeshData::canSplit(FVector &a, FVector &b, FVector &c, float mindistanceKept){
    if(FVector::Dist(a,b) < mindistanceKept){
        return false;
    }
    if(FVector::Dist(a,c) < mindistanceKept){
        return false;
    }
    if(FVector::Dist(b,c) < mindistanceKept){
        return false;
    }
    return true;
}


bool MeshData::canSplit(FVector &a, FVector &b, FVector &c){
    return canSplit(a, b, c, MIN_SPLITDISTANCE);
}

bool MeshData::canSplit(int v0, int v1, int v2){
    return canSplit(v0, v1, v2, MIN_SPLITDISTANCE);
}

bool MeshData::canSplit(int v0, int v1, int v2, float mindistanceKept){
    if(isValidVertexIndex(v0,v1,v2)){
        return canSplit(
            vertecies[v0],
            vertecies[v1],
            vertecies[v2],
            mindistanceKept
        );
    }
    return false;
}



FVector MeshData::splitEdge(FVector &v0, FVector &v1, FVector &v2, int egdeIndex){
    egdeIndex = std::abs(egdeIndex) % 3; //safety
    int endEdgeIndex = (egdeIndex + 1) % 3;
    TArray<FVector> asArray = {v0, v1, v2};

    FVector start = asArray[egdeIndex];
    FVector dir = asArray[endEdgeIndex] - start; //AB = B - A

    FVector newVertex = start + 0.5f * dir;
    return newVertex;
}

void MeshData::Split(FVector &v0, FVector &v1, FVector &v2, TArray<FVector> &outBuffer){

    int indexEdge = findLongestSideIndex(v0, v1, v2);//v0v1 v1v2 v2v0
    FVector newVertex = splitEdge(v0, v1, v2, indexEdge);
    if (indexEdge == 0)
    {
        /*
         1-->2
        x|  -
         0-
        */
        //new triangles at 
        //0,1,new
        //new,1,2
        outBuffer.Add(v0);
        outBuffer.Add(newVertex);
        outBuffer.Add(v2);

        outBuffer.Add(newVertex);
        outBuffer.Add(v1);
        outBuffer.Add(v2);
    }

    if (indexEdge == 1)
    {
        /*
        1-x->2
        |  -
        0-
        */
        //new triangles at 
        //0,1,new
        //0,new,2
        outBuffer.Add(v0);
        outBuffer.Add(v1);
        outBuffer.Add(newVertex);

        outBuffer.Add(v0);
        outBuffer.Add(newVertex);
        outBuffer.Add(v2);
    }

    if (indexEdge == 2)
    {
        /*
        1-->2
        |  -x
        0-
        */
        //new triangles at 
        //0,1,new
        //new,1,2
        outBuffer.Add(v0);
        outBuffer.Add(v1);
        outBuffer.Add(newVertex);

        outBuffer.Add(v1);
        outBuffer.Add(v2);
        outBuffer.Add(newVertex);
    }









    /*FVector newVertex = start + 0.5f * dir;

    FVector start;
    FVector dir;
    findLongestSide(v0, v1, v2, start, dir);
    FVector newVertex = start + 0.5f * dir;
    //FVector newVertex = v1 + 0.5f * (v2 - v1); // gx = A + r(B-A)*/
    
    
}

void MeshData::SplitTriangleShapedBufferOverride(
    TArray<FVector> &inBuffer
){
    TArray<FVector> outBuffer;
    SplitTriangleShapedBuffer(inBuffer, outBuffer);
    inBuffer = outBuffer;
}

void MeshData::SplitTriangleShapedBuffer(
    TArray<FVector> &inBuffer, //must be triangle shaped
    TArray<FVector> &outBuffer
){
    for(int i = 2; i < inBuffer.Num(); i += 3){
        Split(
            inBuffer[i-2],
            inBuffer[i-1],
            inBuffer[i], 
            outBuffer
        );
    }
}

void MeshData::addTriangle(int v0, int v1, int v2){
    if(isValidVertexIndex(v0,v1,v2)){
        triangles.Add(v0);
        triangles.Add(v1);
        triangles.Add(v2);
    }
}

void MeshData::addTriangleDoublesided(int v0, int v1, int v2){
    addTriangle(v0, v1, v2);
    addTriangle(v0, v2, v1);
}

///@brief removes the triangle targeted and splits it if possible
void MeshData::splitTriangleInHalf(int v0, int v1, int v2){
    if(isValidVertexIndex(v0,v1,v2)){
        //remove triangle
        int removedTriangles = removeTriangleSimilarTo(v0, v1, v2);
        //DebugHelper::logMessage("debugTriangle -> triangles removed: ", removedTriangles);

        //create new triangles
        float maxDistance = 200.0f;
        if (canSplit(v0, v1, v2, maxDistance))
        {
        
            FVector &A = vertecies[v0];
            FVector &B = vertecies[v1];
            FVector &C = vertecies[v2];

            //FVector middle = A + 0.5f * (B - A); // gx = A + r(B-A)

            FVector start;
            FVector dir;
            findLongestSide(A, B, C, start, dir);
            FVector middle = start + 0.5f * dir;

            //append new vertex and normal
            int newIndex = vertecies.Num();
            if(normals.Num() == vertecies.Num()){
                FVector cross = FVector::CrossProduct((middle - A), (C - A));
                cross = cross.GetSafeNormal();
                normals.Add(cross);
            }
            vertecies.Add(middle);


            if(removedTriangles == 1){
                //append sub triangles
                addTriangle(v0, newIndex, v2);
                addTriangle(newIndex, v1, v2);
            }else{

                //not needed basically. But if a triangle was double sided the splittet
                //will be too.
                addTriangleDoublesided(v0, newIndex, v2);
                addTriangleDoublesided(newIndex, v1, v2);
            }

            //DebugHelper::logMessage("debugTriangle added");

            //calculateNormals();
        }
    }
}

int MeshData::findLongestSideIndex(
    FVector &a, 
    FVector &b, 
    FVector &c
){
    FVector dirAB = b - a;
    FVector dirBC = c - b;
    FVector dirCA = a - c;

    std::vector<FVector> sides = {
        dirAB,
        dirBC,
        dirCA
    };

    int foundindex = 0;
    float largestSide = -1.0f; // so the start is set anyway
    for (int i = 0; i < sides.size(); i++){
        FVector &current = sides[i];

        float updateSize = current.Size();
        if (updateSize > largestSide)
        {
            foundindex = i;
            largestSide = updateSize;
        }
    }
    return foundindex;
}

void MeshData::findLongestSide(
    FVector &a, 
    FVector &b, 
    FVector &c,
    FVector &startOut,
    FVector &dirOut
){
    FVector dirAB = b - a;
    FVector dirBC = c - b;
    FVector dirCA = a - c;

    std::vector<FVector> sides = {
        dirAB,
        dirBC,
        dirCA
    };
    std::vector<FVector> startPoints = {a, b, c};

    int longestSideIndex = findLongestSideIndex(a, b, c);
    if(longestSideIndex >= 0 && longestSideIndex < 3){
        startOut = startPoints[longestSideIndex];
        dirOut = sides[longestSideIndex];
    }
}


///@brief finds all triangles where an index is contained
void MeshData::findTrianglesInvolvedWith(
    int index,
    std::vector<int> &trianglesFound
){
    if(isValidVertexIndex(index)){
        for (int i = 2; i < triangles.Num(); i+=3){
            int v0 = triangles[i - 2];
            int v1 = triangles[i - 1];
            int v2 = triangles[i];
    
            if(isPartOfTraingle(index, v0, v1, v2)){
                trianglesFound.push_back(v0);
                trianglesFound.push_back(v1);
                trianglesFound.push_back(v2);
            }
        }
    }
}

bool MeshData::isPartOfTraingle(int target, int v0, int v1, int v2){
    return target == v0 || target == v1 || target == v2;
}


///@brief removed triangle can be in incorrect order
int MeshData::removeTriangleSimilarTo(int v0, int v1, int v2){
    int removed = 0;
    if (isValidVertexIndex(v0, v1, v2))
    {
        int i = 2;
        int limit = triangles.Num();
        while(i < limit){
            int &v0Current = triangles[i - 2];
            int &v1Current = triangles[i - 1];
            int &v2Current = triangles[i];

            if(trianglesAreSame(v0, v1, v2, v0Current, v1Current, v2Current)){
                int lastIndex = triangles.Num() - 1;
                if(lastIndex - 2 >= 0){
                    v2Current = triangles[lastIndex]; //copied into reference.
                    v1Current = triangles[lastIndex - 1];
                    v0Current = triangles[lastIndex - 2];

                    triangles.Pop();
                    triangles.Pop();
                    triangles.Pop();
                    limit = triangles.Num();

                    //DebugHelper::logMessage("debugTriangle removed, new size: ", triangles.Num());

                    removed++;
                }
                //return;
            }
            i+=3;
        }
    }
    return removed;
}

bool MeshData::trianglesAreSame(int v0, int v1, int v2, int v00, int v01, int v02){
    std::set<int> set;
    set.insert(v0);
    set.insert(v1);
    set.insert(v2);
    set.insert(v00);
    set.insert(v01);
    set.insert(v02);

    return set.size() == 3;
}

/// @brief calculates the center of the vertecies
/// @return center
FVector MeshData::center(){
    FVector zeroVec(0, 0, 0);
    if(vertecies.Num() <= 0){
        return zeroVec;
    }

    for (int i = 0; i < vertecies.Num(); i++){
        zeroVec += vertecies[i];
    }
    zeroVec /= vertecies.Num();
    return zeroVec;
}

/// @brief centers the mesh
void MeshData::centerMesh(){
    FVector thiscenter = center();
    for (int i = 0; i < vertecies.Num(); i++){
        vertecies[i] -= thiscenter;
    }
}

/// @brief flips all triangle surfaces but doesnt refresh the normals!
void MeshData::flipAllTriangles(){
    for (int i = 2; i < triangles.Num(); i += 3){
        //int32 v0 = triangles[i - 2];
        int32 v1 = triangles[i - 1];
        int32 v2 = triangles[i];

        //spaw
        triangles[i] = v1;
        triangles[i - 1] = v2;
    }
}



/**
 * 
 * -- mesh cutting helpers --
 * 
 */


///@brief removes out a vertex from the mesh
void MeshData::removeVertex(int index){
    if(isValidVertexIndex(index)){
        std::vector<int> connected;
        findConnectedVerteciesTo(index, connected);
        removeVertex(index, connected);
    }
    
    
    
}

///@brief removes out a vertex from the mesh data by index and:
///@param connectedvertecies saves the connected vertecies defined by the triangle buffer
void MeshData::removeVertex(int index, std::vector<int>& connectedvertecies){

    if(isValidVertexIndex(index) && vertecies.Num() > 0){
        removeTrianglesInvolvedWith(index, connectedvertecies);

        //remove vertex by swapping with end and change the triangle buffer as needed
        int oldEnd = vertecies.Num() - 1;
        vertecies[index] = vertecies[oldEnd]; //index der removed wird nach hinten tauschen
        vertecies.Pop(); // pop back end which is in new pos now
        if(isValidNormalIndex(index) && isValidNormalIndex(oldEnd)){
            normals[index] = normals[oldEnd];
            normals.Pop();
        }

        //(update the indices in the triangle buffer because the vertex has changed)
        for (int i = 0; i < triangles.Num(); i++){
            if(triangles[i] == oldEnd){
                triangles[i] = index; //update weil swap
            }
        }

        //find old end in connected vertecies and replace the index
        for (int i = 0; i < connectedvertecies.size(); i++){
            int vertexIndexNow = connectedvertecies[i];
            if(oldEnd == vertexIndexNow){
                connectedvertecies[i] = index; //update weil swap
            }
        }

    }
}

///@brief removes all triangles from the triangle buffer by vertex index and
///@param connectedvertecies saves the connected vertecies for processing
void MeshData::removeTrianglesInvolvedWith(int vertexIndex, std::vector<int> &connectedvertecies){
    if(!isValidVertexIndex(vertexIndex)){
        return;
    }
    TArray<int32> triangleBufferCopy;
    for (int i = 0; i < triangles.Num() - 3; i += 3)
    {
        int32 v0 = triangles[i];
        int32 v1 = triangles[i+1];
        int32 v2 = triangles[i+2];

        bool v0ok = (v0 != vertexIndex);
        bool v1ok = (v1 != vertexIndex);
        bool v2ok = (v2 != vertexIndex);

        if(v0ok && v1ok && v2ok){
            //dont share edges with the vertexIndex seached
            triangleBufferCopy.Add(v0);
            triangleBufferCopy.Add(v1);
            triangleBufferCopy.Add(v2);
        }else{
            //connected vertecies are added to the buffer
            //dont copy the vertex which is removed
            if(v0ok && !contains(connectedvertecies, v0)){
                connectedvertecies.push_back(v0);
            }
            if(v1ok && !contains(connectedvertecies, v1)){
                connectedvertecies.push_back(v1);
            }
            if(v2ok && !contains(connectedvertecies, v2)){
                connectedvertecies.push_back(v2);
            }
            

            
        }
    }
    triangles = triangleBufferCopy;
}


bool MeshData::contains(std::vector<int> &ref, int index){
    for (int i = 0; i < ref.size(); i++){
        if(ref[i] == index){
            return true;
        }
    }
    return false;
}



void MeshData::findConnectedVerteciesTo(int index, std::vector<int> &output){
    if(isValidVertexIndex(index)){
        for (int i = 0; i < triangles.Num() - 3; i += 3)
        {
            int v0 = triangles[i];
            int v1 = triangles[i+1];
            int v2 = triangles[i+2];
            if(v0 == index || v1 == index || v2 == index){
                if (!contains(output, v0)){
                    output.push_back(v0);
                }
                if (!contains(output, v1)){
                    output.push_back(v1);
                }
                if (!contains(output, v2)){
                    output.push_back(v2);
                }
            }
        }
    }   
}







/**
 * 
 * room mesh data helper 
 * 
 */

///@brief appends a cube using the efficent method
void MeshData::appendCube(
    FVector &a, 
    FVector &b,
    FVector &c,
    FVector &d,
    FVector &a1, 
    FVector &b1,
    FVector &c1,
    FVector &d1
){
    appendEfficent(a, d, c, b);
    appendEfficent(a1, b1, c1, d1);

    appendEfficent(b, b1, a1, a);
    appendEfficent(c, c1, b1, b);
    appendEfficent(d, d1, c1, c);
    appendEfficent(a, a1, d1, d);

    //calculateNormals();
}

void MeshData::appendCube(
    FVector &a,
    FVector &b,
    FVector &c,
    FVector &d,
    FVector orthogonalDir
){
    FVector a1 = a + orthogonalDir;
    FVector b1 = b + orthogonalDir;
    FVector c1 = c + orthogonalDir;
    FVector d1 = d + orthogonalDir;
    appendCube(a, b, c, d, a1, b1, c1, d1);
}


void MeshData::appendCubeAt(
    FVector &pivot,
    int scaleX,
    int scaleY,
    int scaleZ
){
    /*
    1->2
    |  |
    0<-3
    */
    FVector a = pivot;
    FVector b = pivot + FVector(0, scaleY, 0);
    FVector c = pivot + FVector(scaleX, scaleY, 0);
    FVector d = pivot + FVector(scaleX, 0, 0);

    FVector updir(0, 0, scaleZ);

    appendCube(a,b,c,d,updir);
}

int MeshData::verteciesNum(){
    return vertecies.Num();
}

bool MeshData::hasAnyVertecies(){
    return verteciesNum() > 0;
}








/** 

--- Duplicate Mesh Recursive ---

*/
MeshData MeshData::CreateCopyRecuriveDetail(int recursion){
    MeshData outData;
    CreateCopyRecuriveDetailTo(outData, recursion);
    return outData;
}


void MeshData::CreateCopyRecuriveDetailTo(MeshData &outData, int recursion){
    recursion = std::abs(recursion);
    if(recursion == 0){
        return;
    }

    TArray<FVector> bufferMade;
    for (int i = 2; i < triangles.Num(); i += 3){
        int32 t0 = triangles[i - 2];
        int32 t1 = triangles[i - 1];
        int32 t2 = triangles[i];

        FVector &v0 = vertecies[t0];
        FVector &v1 = vertecies[t1];
        FVector &v2 = vertecies[t2];

        TArray<FVector> tempBuffer = {v0, v1, v2};
        for(int j = 0; j < recursion; j++){
            SplitTriangleShapedBufferOverride(tempBuffer);
            if(j == recursion - 1){ //append to main buffer
                outData.appendEfficentTriangleShapedBuffer(tempBuffer);
            }
        }
    }
    outData.calculateNormals();
}


void MeshData::CreateCopyRecuriveDetailToDistance(MeshData &outData, float distance){

    distance = std::abs(distance);
    if(distance <= 1.0f){
        distance = 1.0f;
    }
    float dist2 = distance * distance;

    TArray<FVector> bufferMade;
    for (int i = 2; i < triangles.Num(); i += 3){
        int32 t0 = triangles[i - 2];
        int32 t1 = triangles[i - 1];
        int32 t2 = triangles[i];

        FVector &v0 = vertecies[t0];
        FVector &v1 = vertecies[t1];
        FVector &v2 = vertecies[t2];

        TArray<FVector> tempBuffer = {v0, v1, v2};
        bool finished = false;
        int splitCountMax = 5; //10, 12, 40

        //200 / 2^5

        while (!finished)
        {
            SplitTriangleShapedBufferOverride(tempBuffer);
            splitCountMax--;
            if(splitCountMax <= 0){
                finished = true;
            }
            if (tempBuffer.Num() > 2)
            {

                if(AverageDist2TriangleShapedBuffer(tempBuffer) <= dist2){
                    outData.appendEfficentTriangleShapedBuffer(tempBuffer);
                    finished = true;
                }
            }
            else
            {
                finished = true;
            }
        }
    }
    outData.calculateNormals();
}

float MeshData::AverageDist2TriangleShapedBuffer(TArray<FVector> &buffer){
    float distAccumulated = 0.0f;
    for (int i = 2; i < buffer.Num(); i += 3){
        FVector &v0 = buffer[i - 2];
        FVector &v1 = buffer[i - 1];
        FVector &v2 = buffer[i];
        float tmpDist = FVector::DistSquared(v0, v1);
        tmpDist += FVector::DistSquared(v1, v2);
        tmpDist += FVector::DistSquared(v2, v0);
        distAccumulated += (tmpDist / 3.0f);
    }
    float trianglCount = buffer.Num() / 3.0f;
    return distAccumulated / trianglCount;
}









void MeshData::appendCube(
    TArray<FVector> &sideA,
    TArray<FVector> &sideB
){
    if(sideA.Num() == 4 && sideA.Num() == sideB.Num()){

        //flipped faces
        append(sideB[0], sideB[1], sideA[1], sideA[0]); //side view left
        append(sideA[3], sideA[2], sideB[2], sideB[3]); //side view right

        append(sideA[2], sideA[1], sideB[1], sideB[2]);
        append(sideA[0], sideA[3], sideB[3], sideB[0]);
        


        //top and bottom
        /*append(sideA[2], sideA[1], sideB[1], sideB[2]); //top view
        append(sideA[0], sideA[3], sideB[3], sideB[0]); //bottom view

        //sides
        append(sideB[0], sideB[1], sideA[1], sideA[0]);
        append(sideA[3], sideB[2], sideB[2], sideB[3]);*/

        /*
        appendCube(
            sideA[0], 
            sideA[1],
            sideA[2],
            sideA[3],
            sideB[0], 
            sideB[1],
            sideB[2],
            sideB[3]
        );*/
    }
}