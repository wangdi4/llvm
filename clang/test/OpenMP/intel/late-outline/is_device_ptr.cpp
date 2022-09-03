//INTEL_COLLAB
//RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fopenmp -fopenmp-version=50 \
//RUN:  -fopenmp-late-outline \
//RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

struct S {
  int a = 0;
  int *ptr = &a;
  int &ref = a;
  int *aaptr = &a;
  int arr[4];
  S() {}
  void foo(float *&refptr);
};

//CHECK-LABEL: S3foo
void S::foo(float *&refptr) {
  //CHECK: [[REFPTR:%refptr.*]] = alloca ptr, align 8

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr %ptr)
  //CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr %0)
  //CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr %aaptr)
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM"(ptr %this
  #pragma omp target is_device_ptr(ptr, refptr, aaptr, arr)
  ++a, ++*ptr, ++ref, ++arr[0];
  //CHECK: "DIR.OMP.END.TARGET"()
}


//CHECK-LABEL: foo
void foo() {
  int i;
  int &j = i;
  int *k = &j;
  //CHECK-DAG: [[K:%k.*]] = alloca ptr,
  //CHECK: [[L:%[0-9]+]] = load ptr, ptr %k

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr %k)
  //CHECK: "QUAL.OMP.MAP.TOFROM"(ptr [[L]]
  #pragma omp target map(tofrom: i) is_device_ptr(k)
  {
    i++; j++; k++;
  }
  //CHECK: "DIR.OMP.END.TARGET"()
}

struct SomeKernel {
  int targetDev;
  float* devPtr;
  SomeKernel();
  ~SomeKernel();

  template<unsigned int nRHS>
  void apply() {
    #pragma omp target teams is_device_ptr(devPtr) device(targetDev)
    {
      devPtr++;
    }
  }
};

//CHECK: define {{.*}}use_template
void use_template() {
  SomeKernel aKern;
  aKern.apply<32>();
}

//CHECK: define {{.*}}ZN10SomeKernel5applyILj32EEEvv
//CHECK: [[THIS:%this.*]] = load ptr, ptr %this.addr,
//CHECK: [[DEVPTR:%devPtr.*]] = getelementptr inbounds %struct.SomeKernel, ptr [[THIS]], i32 0, i32 1
//CHECK: "DIR.OMP.TARGET"()
//CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr %devPtr)
//CHECK: "QUAL.OMP.MAP.TO"(ptr %this

// CHECK-LABEL: omp_kernel
void omp_kernel(float * __restrict xxi) {
//CHECK: "DIR.OMP.TASK"()
//CHECK-SAME: "QUAL.OMP.FIRSTPRIVATE"(ptr %xxi.addr)
//CHECK: "DIR.OMP.TARGET"()
//CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr %xxi.addr)
 #pragma omp target teams distribute nowait is_device_ptr(xxi)
  for (int n = 0; n < 100; n += 10) {
    xxi[0] = 0;
  }
// CHECK: "DIR.OMP.END.TARGET"
// CHECK: "DIR.OMP.END.TASK"
}

//CHECK-LABEL: main
int main() {
  float a = 0;
  float *ptr = &a;
  float &ref = a;
  float arr[4];
  float vla[(int)a];

  //CHECK: [[A:%a.*]] = alloca float,
  //CHECK: [[PTR:%ptr.*]] = alloca ptr,
  //CHECK: [[REF:%ref.*]] = alloca ptr,
  //CHECK: [[ARR:%arr.*]] = alloca [4 x float],
  //CHECK: [[FPTR:%fptr.*]] = alloca ptr
  //CHECK: [[VLA:%vla.*]] = alloca float, i64
  //CHECK: [[L:%[0-9]+]] = load ptr, ptr [[PTR]]

  S s;
  s.foo(ptr);

  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr %ptr)
  //CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr %arr)
  //CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr %vla)
  //CHECK: "QUAL.OMP.MAP.TOFROM"(ptr [[L]]
  //CHECK: "QUAL.OMP.MAP.TO"(ptr [[ARR]]
  //CHECK: "QUAL.OMP.MAP.TO:VARLEN"(ptr [[VLA]]
  #pragma omp target \
               is_device_ptr(ptr, arr, vla)
  ++a, ++*ptr, ++ref, ++arr[0], ++vla[0];
  //CHECK: "DIR.OMP.END.TARGET"()

  void (*fptr)(void) = foo;
  //CHECK: store ptr @_Z3foov, ptr [[FPTR]]
  //CHECK: [[L14:%[0-9]+]] = load ptr, ptr [[FPTR]]
  //CHECK: "DIR.OMP.TARGET"()
  //CHECK-SAME: "QUAL.OMP.LIVEIN"(ptr %fptr)
  //CHECK-SAME: "QUAL.OMP.MAP.TOFROM:FPTR"(ptr [[L14]]
  #pragma omp target is_device_ptr(fptr)
    fptr();
  //CHECK: "DIR.OMP.END.TARGET"()

  return a;
}

// end INTEL_COLLAB
