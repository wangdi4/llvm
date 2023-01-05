//INTEL_COLLAB
//RUN: %clang_cc1 -emit-llvm -o - -fopenmp -fopenmp-version=51 \
//RUN:  -no-opaque-pointers -fopenmp-late-outline \
//RUN:  -triple x86_64-unknown-linux-gnu %s \
//RUN:  | FileCheck --check-prefixes=NOPCHECK %s

// NOPCHECK-LABEL: @_Z22target_has_device_addri(
// NOPCHECK-NEXT: entry:
// NOPCHECK-NEXT: [[ARGC_ADDR:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[X:%.*]] = alloca i32, align 4
// NOPCHECK:      "DIR.OMP.TARGET"(),
// NOPCHECK-SAME: "QUAL.OMP.MAP.TO"(i32* [[X]]
// NOPCHECK-NOT: "QUAL.OMP.HAS_DEVICE_ADDR"(i32* [[X]])
// NOPCHECK:      "DIR.OMP.END.TARGET"()
//
void target_has_device_addr(int argc) {
  int x = 5;
#pragma omp target has_device_addr(x)
   argc = x;
}

// NOPCHECK-LABEL: @_Z24target_s_has_device_addri(
// NOPCHECK-NEXT:  entry:
// NOPCHECK-NEXT: [[ARGC_ADDR:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[X:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[COND:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[FP:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[RD:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[LIN:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[STEP:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[MAP:%.*]] = alloca i32, align 4
// NOPCHECK:      "DIR.OMP.TARGET"(),
// NOPCHECK-SAME: "QUAL.OMP.MAP.TO"(i32* [[MAP]],
// NOPCHECK:      "DIR.OMP.SIMD"(),
// NOPCHECK:      omp.loop.exit:
// NOPCHECK-NEXT: "DIR.OMP.END.SIMD"()
// NOPCHECK-NEXT: "DIR.OMP.END.TARGET"()
//
void target_s_has_device_addr(int argc) {
  int x, cond, fp, rd, lin, step, map;
#pragma omp target simd if(cond) firstprivate(fp) reduction(+:rd) linear(lin: step) has_device_addr(map)
  for (int i = 0; i < 10; ++i)
    argc = x, x = map;
}

// NOPCHECK-LABEL: @_Z26target_t_l_has_device_addri(
// NOPCHECK-NEXT: entry:
// NOPCHECK-NEXT: [[ARGC_ADDR:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[X:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[COND:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[FP:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[RD:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT: [[MAP:%.*]] = alloca i32, align 4
// NOPCHECK:      "DIR.OMP.TARGET"(),
// NOPCHECK-SAME "QUAL.OMP.MAP.TO"(i32* [[MAP]],
// NOPCHECK-NOT "QUAL.OMP.HAS_DEVICE_ADDR"(i32* [[MAP]])
// NOPCHECK:      "DIR.OMP.TEAMS"(),
// NOPCHECK:      "DIR.OMP.GENERICLOOP"(),
// NOPCHECK:      omp.loop.exit:
// NOPCHECK-NEXT: "DIR.OMP.END.GENERICLOOP"()
// NOPCHECK-NEXT: "DIR.OMP.END.TEAMS"()
// NOPCHECK-NEXT: "DIR.OMP.END.TARGET"()
//
void target_t_l_has_device_addr(int argc) {
  int x, cond, fp, rd, map;
#pragma omp target teams loop if(cond) firstprivate(fp) reduction(+:rd) has_device_addr(map)
   for (int i = 0; i <10; ++i)
     argc = x, x = map;
}

// NOPCHECK-LABEL: @_Z26target_p_l_has_device_addri(
// NOPCHECK-NEXT:  entry:
// NOPCHECK-NEXT:    [[ARGC_ADDR:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT:    [[X:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT:    [[COND:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT:    [[FP:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT:    [[RD:%.*]] = alloca i32, align 4
// NOPCHECK-NEXT:    [[MAP:%.*]] = alloca i32, align 4
// NOPCHECK:         "DIR.OMP.TARGET"(),
// NOPCHECK-SAME:    "QUAL.OMP.MAP.TO"(i32* [[MAP]],
// NOPCHECK-NOT:     "QUAL.OMP.HAS_DEVICE_ADDR"(i32* [[MAP]])
// NOPCHECK:         "DIR.OMP.PARALLEL"(),
// NOPCHECK:         "DIR.OMP.GENERICLOOP"(),
// NOPCHECK:       omp.loop.exit:
// NOPCHECK-NEXT:    "DIR.OMP.END.GENERICLOOP"()
// NOPCHECK-NEXT:    "DIR.OMP.END.PARALLEL"()
// NOPCHECK-NEXT:    "DIR.OMP.END.TARGET"()
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
// NOPCHECK-LABEL: define {{.*}} @_ZN10SomeKernel5applyILj32EEEvv(
// NOPCHECK-NEXT: entry:
// NOPCHECK-NEXT: [[THIS:%this.*]] = alloca %struct.SomeKernel*,
// NOPCHECK:      [[THIS1:%this1]] = load %struct.SomeKernel*, %struct.SomeKernel** [[THIS]],
// NOPCHECK:      [[DEVPTR:%devPtr2]] = getelementptr inbounds %struct.SomeKernel, %struct.SomeKernel* [[THIS1]], i32 0, i32 1
// NOPCHECK:      "DIR.OMP.TARGET"()
// NOPCHECK:      "QUAL.OMP.MAP.TOFROM"({{.*}} [[THIS1]], float* [[DEVPTR]],
// NOPCHECK-NOT: "QUAL.OMP.HAS_DEVICE_ADDR"(float* [[DEVPTR]])
// NOPCHECK:      "DIR.OMP.END.TARGET"()
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
