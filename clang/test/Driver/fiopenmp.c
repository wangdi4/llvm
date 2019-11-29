// INTEL_COLLAB
// RUN: %clang -target x86_64-linux-gnu -fiopenmp -c %s -### 2>&1 | FileCheck %s --check-prefix=CHECK-CC1-OPENMP --check-prefix=CHECK-PAR-DEFAULT
// RUN: %clang -target x86_64-linux-gnu -fiopenmp -fopenmp=libiomp5 -c %s -### 2>&1 | FileCheck %s --check-prefix=CHECK-CC1-OPENMP --check-prefix=CHECK-PAR-DEFAULT
// RUN: %clang -target x86_64-linux-gnu -fiopenmp -fno-openmp -c %s -### 2>&1 | FileCheck %s --check-prefix=CHECK-CC1-OPENMP
// RUN: %clang -target x86_64-linux-gnu -fiopenmp -mllvm -paropt=4 -c %s -### 2>&1 | FileCheck %s --check-prefix=CHECK-CC1-OPENMP --check-prefix=CHECK-PAR4
//
// CHECK-CC1-OPENMP: "-cc1"
// CHECK-CC1-OPENMP: "-fopenmp-late-outline"
// CHECK-CC1-OPENMP: "-fopenmp-threadprivate-legacy"
// CHECK-CC1-OPENMP: "-fopenmp"
//
// CHECK-PAR-DEFAULT: "-mllvm" "-paropt=31"
//
// CHECK-PAR4: "-mllvm" "-paropt=4"
//
// RUN: %clang -target x86_64-linux-gnu -fiopenmp -fopenmp=libomp %s -o %t -### 2>&1 | FileCheck %s --check-prefix=CHECK-LD-OMP
// RUN: %clang -target x86_64-linux-gnu -fiopenmp -fopenmp=libgomp %s -o %t -### 2>&1 | FileCheck %s --check-prefix=CHECK-LD-GOMP --check-prefix=CHECK-LD-GOMP-RT
// RUN: %clang -target x86_64-linux-gnu -fiopenmp -fopenmp=libiomp5 %s -o %t -### 2>&1 | FileCheck %s --check-prefix=CHECK-LD-IOMP5
//
// CHECK-LD-OMP: "{{.*}}ld{{(.exe)?}}"
// CHECK-LD-OMP: "-lomp"
//
// CHECK-LD-GOMP: "{{.*}}ld{{(.exe)?}}"
// CHECK-LD-GOMP: "-lgomp"
// CHECK-LD-GOMP-RT: "-lrt"
// CHECK-LD-GOMP-NO-RT-NOT: "-lrt"
//
// CHECK-LD-IOMP5: "{{.*}}ld{{(.exe)?}}"
// CHECK-LD-IOMP5: "-liomp5"
//
// RUN: %clang -target x86_64-linux-gnu -fiopenmp %s -o %t -### 2>&1 | FileCheck %s --check-prefix=CHECK-LD-ANY
//
// CHECK-LD-ANY: "{{.*}}ld{{(.exe)?}}"
// CHECK-LD-ANY: "-l{{(omp|gomp|iomp5)}}"
//
// Test for the change of -paropt value when doing offload
//
// RUN: %clang -target x86_64-linux-gnu -fiopenmp -fopenmp-targets=x86_64-unknown-linux-gnu -c %s -### 2>&1 | FileCheck %s --check-prefix=CHECK2-CC1-OPENMP --check-prefix=CHECK2-PAR-DEFAULT
// RUN: %clang -target x86_64-linux-gnu -fiopenmp -fopenmp=libiomp5 -fopenmp-targets=x86_64-unknown-linux-gnu -c %s -### 2>&1 | FileCheck %s --check-prefix=CHECK2-CC1-OPENMP --check-prefix=CHECK2-PAR-DEFAULT
// RUN: %clang -target x86_64-linux-gnu -fiopenmp -fopenmp-targets=x86_64-unknown-linux-gnu -mllvm -paropt=4 -c %s -### 2>&1 | FileCheck %s --check-prefix=CHECK2-CC1-OPENMP --check-prefix=CHECK2-PAR4
//
// CHECK2-CC1-OPENMP: "-cc1"
// CHECK2-CC1-OPENMP: "-fopenmp-late-outline"
// CHECK2-CC1-OPENMP: "-fopenmp-threadprivate-legacy"
// CHECK2-CC1-OPENMP: "-fopenmp"
//
// CHECK2-PAR-DEFAULT: "-mllvm" "-paropt=31"
// CHECK2-PAR-DEFAULT: "-mllvm" "-paropt=63"
//
// CHECK2-PAR4: "-mllvm" "-paropt=4"
// CHECK2-PAR4: "-mllvm" "-paropt=36"
//
// end INTEL_COLLAB
