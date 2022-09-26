// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51     \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                           \
//RUN:   -fsyntax-only -verify -o - %s

//RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fopenmp -fopenmp-version=51  \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                           \
//RUN:   -fsyntax-only -verify -o - %s

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51     \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                           \
//RUN:   -emit-pch -o %t %s

// expected-no-diagnostics

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51     \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                           \
//RUN:   -ast-print %s | FileCheck %s --check-prefix=PRINT

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51     \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                           \
//RUN:   -ast-dump %s | FileCheck %s --check-prefix=DUMP

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51     \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                           \
//RUN:   -include-pch %t -ast-print %s | FileCheck %s --check-prefix=PRINT

//RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51     \
//RUN:   -fopenmp-late-outline -x c++ -std=c++14 -fexceptions -fcxx-exceptions \
//RUN:   -Wno-source-uses-openmp -Wno-openmp-clauses                           \
//RUN:   -include-pch %t -ast-dump-all %s | FileCheck %s --check-prefix=DUMP

#ifndef HEADER
#define HEADER

typedef void *omp_interop_t;

enum { OPENCL=3, SYCL=4, LEVEL_ZERO=6};

void bar_v1(float* F1, float *F2, omp_interop_t);
void bar_v2(float* F1, float *F2, omp_interop_t, omp_interop_t);

//PRINT: #pragma omp declare variant(bar_v1) match(construct={dispatch}) append_args(interop(target,targetsync,prefer_type(1,3)))
//DUMP: FunctionDecl{{.*}}bar1 'void (float *, float *)'
//DUMP: OMPDeclareVariantAttr{{.*}}construct={dispatch} Target_TargetSync
//DUMP: DeclRefExpr{{.*}}bar_v1
#pragma omp declare variant(bar_v1) match(construct={dispatch}) \
                                    append_args(interop(target,prefer_type(1,3),targetsync))
void bar1(float *FF1, float *FF2) { return; }

//PRINT: #pragma omp declare variant(bar_v1) match(construct={dispatch}) append_args(interop(targetsync,prefer_type("cuda")))
//DUMP: FunctionDecl{{.*}}bar2 'void (float *, float *)'
//DUMP: OMPDeclareVariantAttr{{.*}}construct={dispatch} TargetSync
//DUMP: DeclRefExpr{{.*}}bar_v1
#pragma omp declare variant(bar_v1) match(construct={dispatch}) \
                                    append_args(interop(prefer_type("cuda"),targetsync))
void bar2(float *FF1, float *FF2) { return; }

//PRINT: #pragma omp declare variant(bar_v1) match(construct={dispatch}) append_args(interop(target,prefer_type(2,"sycl")))
//DUMP: FunctionDecl{{.*}}bar3 'void (float *, float *)'
//DUMP: OMPDeclareVariantAttr{{.*}}construct={dispatch} Target
//DUMP: DeclRefExpr{{.*}}bar_v1
#pragma omp declare variant(bar_v1) match(construct={dispatch}) \
                                    append_args(interop(target,prefer_type(2,"sycl")))
void bar3(float *FF1, float *FF2) { return; }

//PRINT: #pragma omp declare variant(bar_v2) match(construct={dispatch}) append_args(interop(target,prefer_type("level_zero")), interop(targetsync,prefer_type(4,1)))
//DUMP: FunctionDecl{{.*}}bar4 'void (float *, float *)'
//DUMP: OMPDeclareVariantAttr{{.*}}construct={dispatch} Target TargetSync
//DUMP: DeclRefExpr{{.*}}bar_v2
#pragma omp declare variant(bar_v2) match(construct={dispatch}) \
                       append_args(interop(target,prefer_type("level_zero")), interop(prefer_type(4,1),targetsync))
void bar4(float *FF1, float *FF2) { return; }

//PRINT: #pragma omp declare variant(bar_v1) match(construct={dispatch}) append_args(interop(target,prefer_type(LEVEL_ZERO,(int)+4.F)))
//DUMP: FunctionDecl{{.*}}bar5 'void (float *, float *)'
//DUMP: OMPDeclareVariantAttr{{.*}}construct={dispatch} Target
//DUMP: DeclRefExpr{{.*}}bar_v1
//DUMP: EnumConstant {{.*}}LEVEL_ZERO
//DUMP: CastExpr {{.*}}'int'
//DUMP: FloatingLiteral {{.*}}4.0
#pragma omp declare variant(bar_v1) match(construct={dispatch}) \
                                    append_args(interop(target,prefer_type(LEVEL_ZERO,(int)+4.0f)))
void bar5(float *FF1, float *FF2) { return; }

//PRINT: class A {
//DUMP: CXXRecordDecl{{.*}}class A definition
class A {
public:
  void memberfoo_v1(float *A, float *B, int *I, omp_interop_t IOp);
  //PRINT: #pragma omp declare variant(memberfoo_v1) match(construct={dispatch}) append_args(interop(target,prefer_type(1,2,3,4,5,6)))
  //DUMP: CXXMethodDecl{{.*}}memberbar 'void (float *, float *, int *)'
  //DUMP: OMPDeclareVariantAttr{{.*}}Implicit construct={dispatch} Target
  //DUMP: DeclRefExpr{{.*}}'memberfoo_v1' 'void (float *, float *, int *, omp_interop_t)'
  #pragma omp declare variant(memberfoo_v1) match(construct={dispatch}) \
    append_args(interop(target,prefer_type(1,2,3,4,5,6)))
  void memberbar(float *A, float *B, int *I) { return; }

  static void smemberfoo_v1(float *A, float *B, int *I, omp_interop_t IOp);
  //PRINT: #pragma omp declare variant(smemberfoo_v1) match(construct={dispatch}) append_args(interop(target,prefer_type("cuda","cuda_driver","opencl","sycl","hip","level_zero")))
  //DUMP: CXXMethodDecl{{.*}}smemberbar 'void (float *, float *, int *)' static
  //DUMP: OMPDeclareVariantAttr{{.*}}Implicit construct={dispatch} Target
  //DUMP: DeclRefExpr{{.*}}'smemberfoo_v1' 'void (float *, float *, int *, omp_interop_t)'
  #pragma omp declare variant(smemberfoo_v1) match(construct={dispatch}) \
    append_args(interop(prefer_type("cuda","cuda_driver","opencl","sycl","hip","level_zero"),target))
  static void smemberbar(float *A, float *B, int *I) { return; }
};

template <typename T> void templatefoo_v1(const T& t, omp_interop_t I);
template <typename T> void templatebar(const T& t) {}

//PRINT: #pragma omp declare variant(templatefoo_v1<int>) match(construct={dispatch}) append_args(interop(target,prefer_type(5)))
//DUMP: FunctionDecl{{.*}}templatebar 'void (const int &)'
//DUMP: OMPDeclareVariantAttr{{.*}}Implicit construct={dispatch} Target
//DUMP: DeclRefExpr{{.*}}'templatefoo_v1' 'void (const int &, omp_interop_t)'
#pragma omp declare variant(templatefoo_v1<int>) match(construct={dispatch}) \
  append_args(interop(target,prefer_type(5)))
void templatebar(const int &t) {}

template <signed d>
class X {
public:
  void tfoo_v1(float *A, float *B, int *I, omp_interop_t IOp);

  //PRINT: #pragma omp declare variant(tfoo_v1) match(construct={dispatch}) append_args(interop(target,prefer_type(1,d)))
  #pragma omp declare variant(tfoo_v1) match(construct={dispatch}) \
    append_args(interop(target, prefer_type(1,d)))
  void tbar(float *A, float *B, int *I) { return; }
};

  //PRINT: #pragma omp declare variant(tfoo_v1) match(construct={dispatch}) append_args(interop(target,prefer_type(1,5)))
int main()
{
  float f1 = 0.0;
  int I = 0;
  X<5> x;
  x.tbar(&f1, &f1, &I);
  return 0;
}
#endif // HEADER
// end INTEL_COLLAB
