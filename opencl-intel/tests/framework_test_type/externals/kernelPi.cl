__kernel void kernelPi(__global int* i){
int pi = 3141592653589;
*i+=pi;
}