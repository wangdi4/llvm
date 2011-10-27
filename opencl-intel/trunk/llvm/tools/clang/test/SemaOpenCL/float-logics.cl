// RUN: %clang_cc1 %s -emit-llvm -o -

typedef int   int2   __attribute__((ext_vector_type(2)));
typedef float float2 __attribute__((ext_vector_type(2)));

void foo() {
    float2 a = (float2)( 0.0, 0.0 );
    float  b = 0.0;
    int2   i = a && b;
    int2   j = b && a;
}
