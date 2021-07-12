// RUN: %clang_cc1 %s -verify -pedantic -fsyntax-only -cl-std=CL1.2
// if INTEL_CUSTOMIZATION
// RUN: %clang_cc1 %s -verify -pedantic -fsyntax-only -cl-std=CL3.0 -cl-ext=-__opencl_c_program_scope_global_variables -DNO_GENERIC_AS
// RUN: %clang_cc1 %s -verify -pedantic -fsyntax-only -cl-std=CL3.0 -cl-ext=+__opencl_c_program_scope_global_variables -DNO_GENERIC_AS
// endif INTEL_CUSTOMIZATION
static constant int G1 = 0;
constant int G2 = 0;

// if INTEL_CUSTOMIZATION
int G3 = 0;
#ifndef __opencl_c_program_scope_global_variables
// expected-error@-2 {{program scope variable must reside in constant address space}}
#endif

global int G4 = 0;
#ifndef __opencl_c_program_scope_global_variables
// expected-error@-2 {{program scope variable must reside in constant address space}}
#endif

static float g_implicit_static_var = 0;
#ifndef __opencl_c_program_scope_global_variables
// expected-error@-2 {{program scope variable must reside in constant address space}}
#endif

static constant float g_constant_static_var = 0;

static global float g_global_static_var = 0;
#ifndef __opencl_c_program_scope_global_variables
// expected-error@-2 {{program scope variable must reside in constant address space}}
#endif

static local float g_local_static_var = 0;
#ifndef __opencl_c_program_scope_global_variables
// expected-error@-2 {{program scope variable must reside in constant address space}}
#else
// expected-error@-4 {{program scope variable must reside in global or constant address space}}
#endif

static private float g_private_static_var = 0;
#ifndef __opencl_c_program_scope_global_variables
// expected-error@-2 {{program scope variable must reside in constant address space}}
#else
// expected-error@-4 {{program scope variable must reside in global or constant address space}}
#endif

static generic float g_generic_static_var = 0; // expected-error-re {{OpenCL C version {{1.2|3.0}} does not support the 'generic' type qualifier}}
#ifndef __opencl_c_program_scope_global_variables
// expected-error@-2 {{program scope variable must reside in constant address space}}
#endif

extern float g_implicit_extern_var;
#ifndef __opencl_c_program_scope_global_variables
// expected-error@-2 {{extern variable must reside in constant address space}}
#endif

extern constant float g_constant_extern_var;

extern global float g_global_extern_var;
#ifndef __opencl_c_program_scope_global_variables
// expected-error@-2 {{extern variable must reside in constant address space}}
#endif

extern local float g_local_extern_var;
#ifndef __opencl_c_program_scope_global_variables
// expected-error@-2 {{extern variable must reside in constant address space}}
#else
// expected-error@-4 {{extern variable must reside in global or constant address space}}
#endif

extern private float g_private_extern_var;
#ifndef __opencl_c_program_scope_global_variables
// expected-error@-2 {{extern variable must reside in constant address space}}
#else
// expected-error@-4 {{extern variable must reside in global or constant address space}}
#endif

extern generic float g_generic_extern_var; // expected-error-re {{OpenCL C version {{1.2|3.0}} does not support the 'generic' type qualifier}}
#ifndef __opencl_c_program_scope_global_variables
// expected-error@-2 {{extern variable must reside in constant address space}}
#endif

void kernel foo(int x) {
  // static is not allowed at local scope before CL2.0
  static int S1 = 5;
  static constant int S2 = 5;
#if __OPENCL_C_VERSION__ < 300
// expected-error@-3 {{variables in function scope cannot be declared static}}
// expected-error@-3 {{variables in function scope cannot be declared static}}
#elif !defined(__opencl_c_program_scope_global_variables)
// expected-error@-6 {{static local variable must reside in constant address space}}
#endif
// endif INTEL_CUSTOMIZATION

  constant int L1 = 0;
  local int L2;

  if (true) {
    local int L1; // expected-error {{variables in the local address space can only be declared in the outermost scope of a kernel function}}
    constant int L1 = 42; // expected-error {{variables in the constant address space can only be declared in the outermost scope of a kernel function}}
  }

  auto int L3 = 7;                            // INTEL: expected-error-re{{OpenCL C version {{1.2|3.0}} does not support the 'auto' storage class specifier}}
  global int L4;                              // expected-error{{function scope variable cannot be declared in global address space}}
  __attribute__((address_space(100))) int L5; // expected-error{{automatic variable qualified with an invalid address space}}

  constant int L6 = x;                        // expected-error {{initializer element is not a compile-time constant}}
  global int *constant L7 = &G4;
  private int *constant L8 = &x;              // expected-error {{initializer element is not a compile-time constant}}
  constant int *constant L9 = &L1;
  local int *constant L10 = &L2;              // expected-error {{initializer element is not a compile-time constant}}
}

static void kernel bar() { // expected-error{{kernel functions cannot be declared static}}
}

void f() {
  constant int L1 = 0;                        // expected-error{{non-kernel function variable cannot be declared in constant address space}}
  local int L2;                               // expected-error{{non-kernel function variable cannot be declared in local address space}}
  global int L3;                              // expected-error{{function scope variable cannot be declared in global address space}}
  __attribute__((address_space(100))) int L4; // expected-error{{automatic variable qualified with an invalid address space}}

  {
    constant int L1 = 0;                        // expected-error{{non-kernel function variable cannot be declared in constant address space}}
    local int L2;                               // expected-error{{non-kernel function variable cannot be declared in local address space}}
    global int L3;                              // expected-error{{function scope variable cannot be declared in global address space}}
    __attribute__((address_space(100))) int L4; // expected-error{{automatic variable qualified with an invalid address space}}
  }

// if INTEL_CUSTOMIZATION
  static float l_implicit_static_var = 0;
#if __OPENCL_C_VERSION__ < 300
// expected-error@-2 {{variables in function scope cannot be declared static}}
#elif !defined(__opencl_c_program_scope_global_variables)
// expected-error@-4 {{static local variable must reside in constant address space}}
#endif

  static constant float l_constant_static_var = 0;
#if __OPENCL_C_VERSION__ < 300
// expected-error@-2 {{variables in function scope cannot be declared static}}
#endif

  static global float l_global_static_var = 0;
#if __OPENCL_C_VERSION__ < 300
// expected-error@-2 {{variables in function scope cannot be declared static}}
#elif !defined(__opencl_c_program_scope_global_variables)
// expected-error@-4 {{static local variable must reside in constant address space}}
#endif

  static local float l_local_static_var = 0;
#if __OPENCL_C_VERSION__ < 300
// expected-error@-2 {{variables in function scope cannot be declared static}}
#elif !defined(__opencl_c_program_scope_global_variables)
// expected-error@-4 {{static local variable must reside in constant address space}}
#elif defined(__opencl_c_program_scope_global_variables)
// expected-error@-6 {{static local variable must reside in global or constant address space}}
#endif

  static private float l_private_static_var = 0;
#if __OPENCL_C_VERSION__ < 300
// expected-error@-2 {{variables in function scope cannot be declared static}}
#elif !defined(__opencl_c_program_scope_global_variables)
// expected-error@-4 {{static local variable must reside in constant address space}}
#elif defined(__opencl_c_program_scope_global_variables)
// expected-error@-6 {{static local variable must reside in global or constant address space}}
#endif

  static generic float l_generic_static_var = 0; // expected-error-re{{OpenCL C version {{1.2|3.0}} does not support the 'generic' type qualifier}}
#if __OPENCL_C_VERSION__ < 300
// expected-error@-2 {{variables in function scope cannot be declared static}}
#elif !defined(__opencl_c_program_scope_global_variables)
// expected-error@-4 {{static local variable must reside in constant address space}}
#endif

  extern float l_implicit_extern_var;
#if __OPENCL_C_VERSION__ < 300
// expected-error@-2 {{extern variable must reside in constant address space}}
#elif !defined(__opencl_c_program_scope_global_variables)
// expected-error@-4 {{extern variable must reside in constant address space}}
#endif

  extern constant float l_constant_extern_var;

  extern global float l_global_extern_var;
#if __OPENCL_C_VERSION__ < 300
// expected-error@-2 {{extern variable must reside in constant address space}}
#elif !defined(__opencl_c_program_scope_global_variables)
// expected-error@-4 {{extern variable must reside in constant address space}}
#endif

  extern local float l_local_extern_var;
#if __OPENCL_C_VERSION__ < 300
// expected-error@-2 {{extern variable must reside in constant address space}}
#elif !defined(__opencl_c_program_scope_global_variables)
// expected-error@-4 {{extern variable must reside in constant address space}}
#elif defined(__opencl_c_program_scope_global_variables)
// expected-error@-6 {{extern variable must reside in global or constant address space}}
#endif

  extern private float l_private_extern_var;
#if __OPENCL_C_VERSION__ < 300
// expected-error@-2 {{extern variable must reside in constant address space}}
#elif !defined(__opencl_c_program_scope_global_variables)
// expected-error@-4 {{extern variable must reside in constant address space}}
#elif defined(__opencl_c_program_scope_global_variables)
// expected-error@-6 {{extern variable must reside in global or constant address space}}
#endif

  extern generic float l_generic_extern_var; // expected-error-re{{OpenCL C version {{1.2|3.0}} does not support the 'generic' type qualifier}}
#if __OPENCL_C_VERSION__ < 300
// expected-error@-2 {{extern variable must reside in constant address space}}
#elif !defined(__opencl_c_program_scope_global_variables)
// expected-error@-4 {{extern variable must reside in constant address space}}
#endif
}
// endif INTEL_CUSTOMIZATION
