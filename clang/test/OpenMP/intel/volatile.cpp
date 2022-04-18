// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fintel-compatibility -fopenmp \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s \
// RUN:  | FileCheck %s --check-prefix=ENABLED --check-prefix=ALL

// RUN: %clang_cc1 -opaque-pointers -emit-llvm -o - -fintel-compatibility -fopenmp \
// RUN:  -fopenmp-late-outline -triple x86_64-unknown-linux-gnu %s \
// RUN:  -fintel-compatibility-disable=VolatileInOMPRegions \
// RUN:  | FileCheck %s --check-prefix=DISABLED --check-prefix=ALL

int get_thread_num();
// ALL-LABEL: foo
void foo() {
  // ALL: [[LCK:%lck[0-9]*]] = alloca i32,
  // ALL: store volatile i32 0, ptr [[LCK]],
  int thread;
  int volatile lck;
  lck = 0;

  // ALL: directive.region.entry{{.*}}DIR.OMP.PARALLEL
  #pragma omp parallel private(thread )
  {

    thread = get_thread_num();
    // ENABLED: load volatile i32, ptr [[LCK]],
    // DISABLED-NOT: load volatile i32, ptr [[LCK]],
    while (thread != 0 && lck == 0) {
    }

    if (thread == 0) {
      // ENABLED: store volatile i32 1, ptr [[LCK]],
      // DISABLED-NOT: store volatile i32 1, ptr [[LCK]],
      lck = 1;
    }
  }
  // ALL: directive.region.exit{{.*}}DIR.OMP.END.PARALLEL
}
