//INTEL_COLLAB

//RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-version=51 \
//RUN:   -opaque-pointers -fopenmp-late-outline \
//RUN:  -triple x86_64-unknown-linux-gnu %s | FileCheck %s

// CHECK-LABEL: @_Z22target_has_device_addri(
// CHECK-NEXT:    entry:
// CHECK-NEXT:    [[ARGC_ADDR:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[X:%.*]] = alloca i32, align 4
// CHECK:         "DIR.OMP.TARGET"(),
// CHECk-SAME:    "QUAL.OMP.MAP.TO"(ptr [[X]]
// CHECK-NOT:     "QUAL.OMP.HAS_DEVICE_ADDR"(ptr [[X]])
// CHECK:         "DIR.OMP.END.TARGET"()
//
void target_has_device_addr(int argc) {
  int x = 5;
#pragma omp target has_device_addr(x)
   argc = x;
}

// CHECK-LABEL: @_Z24target_s_has_device_addri(
// CHECK-NEXT:     entry:
// CHECK-NEXT:    [[ARGC_ADDR:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[X:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[COND:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[FP:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[RD:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[LIN:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[STEP:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[MAP:%.*]] = alloca i32, align 4
// CHECK:         "DIR.OMP.TARGET"(),
// CHECK:         "QUAL.OMP.MAP.TO"(ptr [[MAP]],
// CHECK-NOT:     "QUAL.OMP.HAS_DEVICE_ADDR"(ptr [[MAP]]),
// CHECK:         "DIR.OMP.SIMD"(),
// CHECK:         omp.loop.exit:
// CHECK-NEXT:    "DIR.OMP.END.SIMD"()
// CHECK-NEXT:    "DIR.OMP.END.TARGET"()
//
void target_s_has_device_addr(int argc) {
  int x, cond, fp, rd, lin, step, map;
#pragma omp target simd if(cond) firstprivate(fp) reduction(+:rd) linear(lin: step) has_device_addr(map)
  for (int i = 0; i < 10; ++i)
    argc = x, x = map;
}

// CHECK-LABEL: @_Z26target_t_l_has_device_addri(
// CHECK-NEXT:    entry:
// CHECK-NEXT:    [[ARGC_ADDR:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[X:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[COND:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[FP:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[RD:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[MAP:%.*]] = alloca i32, align 4
// CHECK:         "DIR.OMP.TARGET"(),
// CHECK-SAME:    "QUAL.OMP.MAP.TO"(ptr [[MAP]]
// CHECK-NOT:     "QUAL.OMP.HAS_DEVICE_ADDR"(ptr [[MAP]])
// CHECK:         "DIR.OMP.TEAMS"(),
// CHECK:         "DIR.OMP.GENERICLOOP"(),
// CHECK:         omp.loop.exit:
// CHECK-NEXT:    "DIR.OMP.END.GENERICLOOP"()
// CHECK-NEXT:    "DIR.OMP.END.TEAMS"()
// CHECK-NEXT:    "DIR.OMP.END.TARGET"()
//
void target_t_l_has_device_addr(int argc) {
  int x, cond, fp, rd, map;
#pragma omp target teams loop if(cond) firstprivate(fp) reduction(+:rd) has_device_addr(map)
   for (int i = 0; i <10; ++i)
     argc = x, x = map;
}

// CHECK-LABEL: @_Z26target_p_l_has_device_addri(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[ARGC_ADDR:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[X:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[COND:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[FP:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[RD:%.*]] = alloca i32, align 4
// CHECK-NEXT:    [[MAP:%.*]] = alloca i32, align 4
// CHECK:         "DIR.OMP.TARGET"(),
// CHECK-SAME:    "QUAL.OMP.MAP.TO"(ptr [[MAP]]
// CHECK-NOT:     "QUAL.OMP.HAS_DEVICE_ADDR"(ptr [[MAP]])
// CHECK:         "DIR.OMP.PARALLEL"(),
// CHECK:         "DIR.OMP.GENERICLOOP"(),
// CHECK:       omp.loop.exit:
// CHECK-NEXT:    "DIR.OMP.END.GENERICLOOP"()
// CHECK-NEXT:    "DIR.OMP.END.PARALLEL"()
// CHECK-NEXT:    "DIR.OMP.END.TARGET"()
//
void target_p_l_has_device_addr(int argc) {
  int x, cond, fp, rd, map;
#pragma omp target parallel loop if(cond) firstprivate(fp) reduction(+:rd) has_device_addr(map)
  for (int i = 0; i < 10; ++i)
    argc = x, x = map;
}

struct SomeKernel {
  int targetDev;
  float devPtr;
  SomeKernel();
  ~SomeKernel();
// CHECK-LABEL: define {{.*}} @_ZN10SomeKernel5applyILj32EEEvv(
// CHECK-NEXT: entry:
// CHECK-NEXT: [[THIS:%this.*]] = alloca ptr,
// CHECK:      [[THIS1:%this1]] = load ptr, ptr [[THIS]],
// CHECK:      [[DEVPTR:%devPtr2]] = getelementptr inbounds %struct.SomeKernel, ptr [[THIS1]], i32 0, i32 1
// CHECK:      "DIR.OMP.TARGET"()
// CHECK-SAME  "QUAL.OMP.MAP.TOFROM"({{.*}} [[THIS1]], ptr* [[DEVPTR]]
// CHECK-NOT:  "QUAL.OMP.HAS_DEVICE_ADDR"(ptr [[DEVPTR]])
// CHECK:      "DIR.OMP.END.TARGET"()
  template<unsigned int nRHS>
  void apply() {
    #pragma omp target has_device_addr(devPtr) device(targetDev)
    {
      devPtr = 10;
    }
  }
};
void use_template() {
  SomeKernel Kern;
  Kern.apply<32>();
}

// end INTEL_COLLAB
