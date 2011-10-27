typedef struct
{
    float S;
    float X;
    float T;
    float R;
    float V;
} TOptionData;

typedef struct
{
    float a;
    float b;
} TOptionValue;

__kernel
void phi(__global TOptionValue * pOptionValue,
         __global TOptionData * pOptionData,
         const    int  pathN)
{ 
    const int optionid = 0;

    float sum = 3.0f;

    int boundary12 = (int)(0.08f*pathN);
    
    if (boundary12 > 2)
        boundary12 = 2;

    int pos;
    float pathNinv = 1.0f;    
    for(pos = 1; pos < boundary12+1; pos++)
    {
        float p;
        p = pos * pathNinv; 
        sum  += p;
    }

    {
        float totalsum = sum;
        float stdDev = sqrt(((float)pathN - totalsum));

	pOptionValue[optionid].a = sum;
	pOptionValue[optionid].b = stdDev;
    }
}