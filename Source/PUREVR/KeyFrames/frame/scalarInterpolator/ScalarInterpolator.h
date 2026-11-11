#pragma once 

#include "CoreMinimal.h"

class PUREVR_API ScalarInterpolator {

public:
    ScalarInterpolator();
    ~ScalarInterpolator();

    

    void Setup(float aIn, float bIn);

    //returns a scalar param value between 0 and 1
    float Tick(float deltatime, float velocity);

    void ProgressTo(float scalar);

    void OverrideTarget(float bIn);

    bool isFinished();

private:
    float a = 0.0f;
    float b = 1.0f;

    float integratedDT = 0.0f;
    float timeToFrame = 1.0f;

    float Scalar();
    
    float Evaluated();
};