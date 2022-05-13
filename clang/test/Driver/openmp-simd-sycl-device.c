/// Check that -fopenmp-simd is NOT passed to the device compilation when
/// used together with -fsycl.

// RUN: %clang -c -fsycl -fopenmp-simd -### %s 2>&1 | FileCheck %s
// RUN: %clang -c -fopenmp-simd -fopenmp-version=50 -### %s 2>&1 | FileCheck %s --check-prefix=UNCHANGED
// INTEL_CUSTOMIZATION
// RUN: %clang -c -fsycl -fiopenmp-simd -### %s 2>&1 | FileCheck %s --check-prefix=IOPENMP
// RUN: %clang -c -fiopenmp-simd -fopenmp-version=50 -### %s 2>&1 | FileCheck %s --check-prefix=UNCHANGED_IOPENMP
// end INTEL_CUSTOMIZATION

// CHECK-NOT: "-triple" "spir64"{{.*}} "-fsycl-is-device"{{.*}} "-target=spir64"{{.*}} "-fopenmp-simd"{{.*}} "-fopenmp-version=50"
// CHECK: "-triple"{{.*}} "-fsycl-is-host"{{.*}} "-fopenmp-simd"{{.*}}

// INTEL_CUSTOMIZATION
// IOPENMP-NOT: "-triple" "spir64"{{.*}} "-fsycl-is-device"{{.*}} "-target=spir64"{{.*}} "-fopenmp-simd"{{.*}}
// IOPENMP: "-triple"{{.*}} "-fsycl-is-host"{{.*}} "-fopenmp-simd"{{.*}}
// end INTEL_CUSTOMIZATION

// UNCHANGED: "-triple"{{.*}} "-fopenmp-simd"{{.*}} "-fopenmp-version=50"{{.*}}
// INTEL_CUSTOMIZATION
// UNCHANGED_IOPENMP: "-triple"{{.*}} "-fopenmp-simd"{{.*}} "-fopenmp-version=50"{{.*}}
// end INTEL_CUSTOMIZATION

void foo(long double) {}
