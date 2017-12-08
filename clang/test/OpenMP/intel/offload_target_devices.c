// RUN: %clang_cc1 -emit-llvm -o - %s -fopenmp -fintel-compatibility -fintel-openmp-region -fopenmp-threadprivate-legacy -fopenmp-targets=x86_64-mic,i386-pc-linux-gnu -triple x86_64-unknown-linux-gnu | FileCheck %s
//
// The target device information is represented as module level attribute in the form of
// target device_triples = x86_64-mic,i386-pc-linux-gnu.
int y;
int main()
{
int x=1;
    #pragma omp target map(tofrom:y,x)
    y = x + 1; // The copy of y on the device has a value of 2.
}
//CHECK: target device_triples
