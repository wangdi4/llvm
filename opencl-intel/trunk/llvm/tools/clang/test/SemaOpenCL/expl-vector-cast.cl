// RUN: %clang_cc1 %s -verify -pedantic -fsyntax-only

typedef unsigned int uint;

typedef short short2 __attribute__((ext_vector_type(2)));
typedef short short3 __attribute__((ext_vector_type(3)));
typedef short short4 __attribute__((ext_vector_type(4)));
typedef uint  uint4  __attribute__((ext_vector_type(4)));
typedef float float4 __attribute__((ext_vector_type(4)));

float4 mad(float4 a, float4 b, float4 c) {
    return (a*b)+c;
}

void __attribute__((overloadable)) min(short3 x) { 
	uint4   temp = (uint4)(1,2,3,4);
	float4  ftemp = mad( (float4)temp, 1.0f, 1.0f ); // expected-error {{casting vector type 'uint4' to vector type 'float4' is not allowed in OpenCL}}
	short4  s4 = (short4)temp; // expected-error {{casting vector type 'uint4' to vector type 'short4' is not allowed in OpenCL}}
	short3  s3 = (short3)s4; // expected-error {{casting vector type 'short4' to vector type 'short3' is not allowed in OpenCL}}
	short2  s2 = (short2)ftemp; // expected-error {{casting vector type 'float4' to vector type 'short2' is not allowed in OpenCL}}
}
