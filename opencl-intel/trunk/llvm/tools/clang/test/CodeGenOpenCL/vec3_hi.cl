// RUN: %clang_cc1 %s -emit-llvm -o -

typedef short short2 __attribute__((ext_vector_type(2)));
typedef short short3 __attribute__((ext_vector_type(3)));

void foo(short2 src, short3 dst) {
	dst.hi = src;
}
