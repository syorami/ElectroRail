#include "common.h"

typedef struct
{
    float P;
    float I;
    float D;
    float Desired;
    float Error;
    float PreError;
    float PrePreError;
    float Increment;
    float Integ;
    float iLimit;
    float Deriv;
    float Output;
 
}PID_Typedef;

float Turn = 0;