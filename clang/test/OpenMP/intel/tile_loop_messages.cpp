// RUN: %clang_cc1 -verify -fsyntax-only -std=c++14 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline %s -DMESSAGES
// RUN: %clang_cc1 -ast-dump -std=c++14 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline %s | FileCheck %s

void bar(int) noexcept;

#define LOOP                   \
  for (i = 16; i < 48; i++) {  \
    for (j = 8; j < 24; j++) { \
      bar(v_ptr[i][j]);        \
    }                          \
  }

const int M = 64;
const int N = 32;
void tile_test(int v_ptr[M][N])
{
  int i, j;
#ifdef MESSAGES
  //expected-warning@+1 {{'collapse' clause ignored when using 'tile' clause}}
  #pragma omp parallel for tile(8,16) collapse(1)
  LOOP

  //expected-warning@+1 {{'collapse' clause ignored when using 'tile' clause}}
  #pragma omp parallel for collapse(1) tile(8,16)
  LOOP

  int value = 8; // expected-note {{declared here}}
  //expected-error@+2 {{expression is not an integral constant}}
  //expected-note@+1 {{read of non-const variable 'value' is not allowed}}
  #pragma omp parallel for tile(value,16)
  LOOP

  //expected-error@+1 {{expected expression}}
  #pragma omp parallel for tile()
  LOOP

  //expected-error@+3 {{expected 3 for loops after}}
  //expected-note@+1 {{as specified in 'tile' clause}}
  #pragma omp parallel for tile(8,16,24)
  LOOP

  //expected-error@+1 {{argument to 'tile' clause must be a non-negative }}
  #pragma omp parallel for tile(-16,24)
  LOOP

  //expected-error@+1 {{argument to 'tile' clause must be a non-negative }}
  #pragma omp parallel for tile(24,-8)
  LOOP

  //expected-error@+1 {{cannot contain more than one 'tile' clause}}
  #pragma omp parallel for tile(24,8) tile(8,24)
  LOOP

  //expected-error@+1 {{unexpected OpenMP clause 'tile' in directive}}
  #pragma omp parallel tile(24,8)
  LOOP

  // Zero is okay
  #pragma omp parallel for tile(0,24)
  LOOP
#endif // MESSAGES

  //CHECK: OMPParallelForDirective
  //CHECK-NEXT: OMPTileClause
  //CHECK-NEXT: ConstantExpr{{.*}} 8{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}} 8{{$}}
  //CHECK-NEXT: ConstantExpr{{.*}} 16{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}} 16{{$}}
  //CHECK: OMPCollapseClause{{.*}}<implicit>
  //CHECK-NEXT: ConstantExpr{{.*}} 2{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}} 2{{$}}
  #pragma omp parallel for tile(8,16)
  LOOP

  //CHECK: OMPForSimdDirective
  #pragma omp for simd tile(8,16)
  LOOP

  //CHECK: OMPParallelForSimdDirective
  #pragma omp parallel for simd tile(8,16)
  LOOP

  //CHECK: OMPTargetParallelForDirective
  #pragma omp target parallel for tile(8,16)
  LOOP

  //CHECK: OMPDistributeParallelForDirective
  #pragma omp distribute parallel for tile(8,16)
  LOOP

  //CHECK: OMPDistributeParallelForSimdDirective
  #pragma omp distribute parallel for simd tile(8,16)
  LOOP

  //CHECK: OMPTargetParallelForSimdDirective
  #pragma omp target parallel for simd tile(8,16)
  LOOP

  //CHECK: OMPTeamsDistributeParallelForSimdDirective
  #pragma omp target
  #pragma omp teams distribute parallel for simd tile(8,16)
  LOOP

  //CHECK: OMPTeamsDistributeParallelForDirective
  #pragma omp target
  #pragma omp teams distribute parallel for tile(8,16)
  LOOP

  //CHECK: OMPTargetTeamsDistributeParallelForSimdDirective
  #pragma omp target teams distribute parallel for simd tile(8,16)
  LOOP

  //CHECK: OMPTargetTeamsDistributeParallelForDirective
  #pragma omp target teams distribute parallel for tile(8,16)
  LOOP
}
