#include "ScalarInterpolator.h"

ScalarInterpolator::ScalarInterpolator(){

}

ScalarInterpolator::~ScalarInterpolator(){

}

void ScalarInterpolator::Setup(float aIn, float bIn){
    a = aIn;
    b = bIn;
    integratedDT = 0.0f;
    timeToFrame = 1.0f; //dummy value
}

void ScalarInterpolator::OverrideTarget(float bIn){
    float aNew = Evaluated();
    Setup(aNew, bIn);
}

float ScalarInterpolator::Tick(float deltatime, float velocity){
    if(isFinished()){
        return b;
    }
    
    //v = m / s
    //s = m / v
    integratedDT += deltatime;
    timeToFrame = std::abs(b - a) / velocity;

    if(isFinished()){
        return b;
    }

    return Evaluated();
}

bool ScalarInterpolator::isFinished(){
    return Scalar() >= 0.99f || std::abs(b-a) <= 0.01f;
}

float ScalarInterpolator::Scalar(){
    if (timeToFrame > 0.0f){
        return integratedDT / timeToFrame;
    }
    return 1.0f;
}

float ScalarInterpolator::Evaluated(){
    float t = Scalar();
    //gT = A + t(B-A)
    return a + t * (b - a);
}

void ScalarInterpolator::ProgressTo(float scalar){
    //s = i / target
    //i = s * target
    a = scalar;
}