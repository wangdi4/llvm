// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -DONE -fopenmp-simd -Werror \
// RUN:  -Wsource-uses-openmp -fintel-compatibility -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s -check-prefix=CHECK-ONE

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -DTWO -fopenmp-simd -Wsource-uses-openmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -verify %s

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -DTHREE -fopenmp -fno-openmp-simd \
// RUN:  -Wsource-uses-openmp -fintel-compatibility -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -verify %s \
// RUN:  | FileCheck %s -check-prefix=CHECK-THREE

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -DFOUR -fopenmp-simd -Werror \
// RUN:  -Wsource-uses-openmp -fintel-compatibility -fopenmp-late-outline \
// RUN:  -triple x86_64-unknown-linux-gnu -x c++ %s \
// RUN:  | FileCheck %s -check-prefix=CHECK-FOUR

#ifdef ONE
// Full OpenMP not enabled, only simd
float AAA[1000];
// CHECK-ONE-LABEL: foo1
void foo1()
{
  int arr[10];
  int i;

  //CHECK-ONE: [[RED:%red.*]] = alloca i32, align 4

// CHECK-ONE: DIR.OMP.SIMD
  #pragma omp simd
  for (i=0;i<10;++i) {
    arr[i] = i;
  }
  int iii;
  //CHECK-ONE: DIR.OMP.SIMD
  #pragma omp parallel for simd shared(AAA)
  for(iii=0; iii<1000; iii++) {
    AAA[iii] = 123.456;
  }

  int red = 0;
  int out[1024];
  int in[1024];
  //CHECK-ONE: DIR.OMP.SIMD
  //CHECK-ONE-SAME: "QUAL.OMP.REDUCTION.ADD:INSCAN"(ptr [[RED]], i64 1)
  #pragma omp simd reduction(inscan, +: red)
  for (int i = 0; i < 1024; ++i) {
    red = red + in[i];
    //CHECK-ONE: DIR.OMP.SCAN
    //CHECK-ONE-SAME: "QUAL.OMP.INCLUSIVE"(ptr [[RED]], i64 1)
    #pragma omp scan inclusive(red)
    out[i] = red;
  }
}
#endif

#ifdef TWO
// Full OpenMP not enabled, only simd
// CHECK-TWO-LABEL: foo2
void foo2()
{
  int arr[10];
  int i;
  #pragma omp parallel for // expected-warning {{unexpected '#pragma omp ...' in program}}
  for (i=0;i<10;++i) {
    arr[i] = i;
  }
}
#endif

#ifdef THREE
// Full OpenMP enabled, but simd disabled
// CHECK-THREE-LABEL: foo3
void foo3()
{
  int arr[10];
  int i;
// CHECK-THREE: DIR.OMP.PARALLEL.LOOP
  #pragma omp parallel for
  for (i=0;i<10;++i) {
    arr[i] = i;
  }
  #pragma omp simd // expected-warning {{unexpected '#pragma omp ...' in program}}
  for (i=0;i<10;++i) {
    arr[i] = i;
  }
}
#endif

#ifdef FOUR
struct ComplexType {
  int min_val;
  int *min_it;
  bool operator<(const ComplexType& obj) {
    if(obj.min_val < min_val) { return false; }
    else if(min_val < obj.min_val) { return true; }
    else { return !(obj.min_it < min_it); }
  }
};

void my_comb(ComplexType& out, ComplexType& in) {
  out = ((in < out) ? in : out);
}

void my_init(ComplexType& out, ComplexType& in) {
  out = in;
}

// CHECK-FOUR-LABEL: simd_min
int* simd_min(int *first, int n) noexcept {

  ComplexType init{*first, first};

  #pragma omp declare reduction(min_func : ComplexType : \
    my_comb(omp_out, omp_in))               \
    initializer(my_init(omp_priv, omp_orig))

  // CHECK-FOUR: "DIR.OMP.SIMD"
  // CHECK-FOUR-SAME: "QUAL.OMP.REDUCTION.UDR"
  // CHECK-FOUR: DIR.OMP.END.SIMD
  #pragma omp simd reduction(min_func:init)
  for (int i = 1; i < n; ++i) {
    if (first[i] < init.min_val){
      init.min_val = first[i];
      init.min_it  = first+i;
    }
  }
  return init.min_it;
}
#endif
