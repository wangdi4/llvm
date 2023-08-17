// Test OpenMP AOT (Ahead of Time) compilation support
// REQUIRES: x86-registered-target

/// Check -Xopenmp-target triggers error when multiple triples are used.
// RUN: not %clang -### -fiopenmp -fopenmp-targets=spir64,spir64_gen -Xopenmp-target-frontend -DFOO %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-FOPENMP-AMBIGUOUS-ERROR %s
// RUN: not %clang_cl -### -Qiopenmp -Qopenmp-targets=spir64,spir64_gen -Xopenmp-target-frontend -DFOO %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-FOPENMP-AMBIGUOUS-ERROR %s
// CHK-FOPENMP-AMBIGUOUS-ERROR: clang{{.*}} error: cannot deduce implicit triple value for -Xopenmp-target, specify triple using -Xopenmp-target=<triple>

/// ###########################################################################

/// Check -Xopenmp-target-frontend triggers error when an option requiring
/// arguments is passed to it.
// RUN: not %clang -### -fiopenmp -fopenmp-targets=spir64 -Xopenmp-target-frontend -Xopenmp-target-frontend -mcpu=none %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-FOPENMP-NESTED-ERROR %s
// RUN: not %clang_cl -### -Qiopenmp -Qopenmp-targets=spir64 -Xopenmp-target-frontend -Xopenmp-target-frontend -mcpu=none %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-FOPENMP-NESTED-ERROR %s
// CHK-FOPENMP-NESTED-ERROR: clang{{.*}} error: invalid -Xopenmp-target argument: '-Xopenmp-target-frontend -Xopenmp-target-frontend', options requiring arguments are unsupported

/// ###########################################################################

/// Ahead of Time compilation for fpga, gen, cpu - tool invocation
// RUN: %clang -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_gen %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=CHK-TOOLS-AOT,CHK-TOOLS-GEN
// RUN: %clang -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_x86_64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=CHK-TOOLS-AOT,CHK-TOOLS-CPU
// CHK-TOOLS-AOT: clang{{.*}} "-triple" "x86_64-unknown-linux-gnu" {{.*}} "-o" "[[OUTPUT7:.+\.o]]"
// CHK-TOOLS-GEN: clang{{.*}} "-triple" "spir64_gen"
// CHK-TOOLS-CPU: clang{{.*}} "-triple" "spir64_x86_64"
// CHK-TOOLS-AOT: "-aux-triple" "x86_64-unknown-linux-gnu" {{.*}} "-o" "[[OUTPUT1:.+\.bc]]"
// CHK-TOOLS-AOT: llvm-link{{.*}} "[[OUTPUT1]]"{{.*}} "-o" "[[OUTPUT2:.+\.bc]]"
// CHK-TOOLS-AOT: llvm-link{{.*}} "--only-needed" "[[OUTPUT2]]" {{.*}} "-o" "[[OUTPUT2T:.+\.bc]]"
// CHK-TOOLS-AOT: sycl-post-link{{.*}} "-o" "[[OUTPUT2_1:.+\.bc]]" "[[OUTPUT2T]]"
// CHK-TOOLS-AOT: llvm-spirv{{.*}} "-o" "[[OUTPUT3:.+\.spv]]" {{.*}} "[[OUTPUT2_1]]"
// CHK-TOOLS-GEN: ocloc{{.*}} "-output" "[[OUTPUT4:.+\.out]]" "-file" "[[OUTPUT3]]"
// CHK-TOOLS-CPU: opencl-aot{{.*}} "-o=[[OUTPUT4:.+\.out]]" {{.*}} "[[OUTPUT3]]"
// CHK-TOOLS-GEN: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[OUTPUT5:.+\.bc]]" "-kind=openmp" "-target=spir64_gen" "[[OUTPUT4]]"
// CHK-TOOLS-CPU: clang-offload-wrapper{{.*}} "-host" "x86_64-unknown-linux-gnu" "-o" "[[OUTPUT5:.+\.bc]]" "-kind=openmp" "-target=spir64_x86_64" "[[OUTPUT4]]"
// CHK-TOOLS-AOT: clang{{.*}} "-emit-obj" {{.*}} "-o" "[[OUTPUT6:.+\.o]]" {{.*}} "[[OUTPUT5]]"
// CHK-TOOLS-AOT: ld{{.*}} "[[OUTPUT7]]"{{.*}} "[[OUTPUT6]]"

/// -fopenmp-target-simd settings.
// RUN: %clang -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_gen -fopenmp-target-simd %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=CHK-TOOLS-GEN-SIMD
// CHK-TOOLS-GEN-SIMD: ocloc{{.*}} "-output" "{{.*}}.out"{{.*}} "-options" "{{.*}}-vc-codegen"

/// ###########################################################################

// Check to be sure that for windows, the 'exe' tools are called
// RUN: %clang_cl -Qiopenmp -Qopenmp-targets=spir64_x86_64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=CHK-TOOLS-CPU-WIN
// RUN: %clang -target x86_64-pc-windows-msvc -fiopenmp -fopenmp-targets=spir64_x86_64 %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=CHK-TOOLS-CPU-WIN
// RUN: %clang_cl -Qiopenmp -Qopenmp-targets=spir64_gen %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=CHK-TOOLS-GEN-WIN
// RUN: %clang -target x86_64-pc-windows-msvc -fiopenmp -fopenmp-targets=spir64_gen %s -### 2>&1 \
// RUN:  | FileCheck %s -check-prefixes=CHK-TOOLS-GEN-WIN
// CHK-TOOLS-GEN-WIN: ocloc.exe{{.*}}
// CHK-TOOLS-CPU-WIN: opencl-aot.exe{{.*}}

/// ###########################################################################

/// Check -Xopenmp-target-backend option passing
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_gen,spir64_x86_64 -Xopenmp-target-backend=spir64_gen "-DFOO1 -DFOO2" -Xopenmp-target-backend=spir64_x86_64 "-DFOO3 -DFOO4" %s 2>&1 \
// RUN:   | FileCheck -check-prefixes=CHK-TOOLS-GEN-OPTS,CHK-TOOLS-CPU-OPTS %s

// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_gen -Xopenmp-target-backend "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-GEN-OPTS %s
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_gen -Xopenmp-target-backend=spir64_gen "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-GEN-OPTS %s
// CHK-TOOLS-GEN-OPTS: ocloc{{.*}} "-output" {{.*}} "-output_no_suffix" {{.*}} "-DFOO1" "-DFOO2"

// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_x86_64 -Xopenmp-target-backend "-DFOO3 -DFOO4" %s 2>&1 \
  // RUN:   | FileCheck -check-prefix=CHK-TOOLS-CPU-OPTS %s
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_x86_64 -Xopenmp-target-backend=spir64_x86_64 "-DFOO3 -DFOO4" %s 2>&1 \
  // RUN:   | FileCheck -check-prefix=CHK-TOOLS-CPU-OPTS %s
// CHK-TOOLS-CPU-OPTS: opencl-aot{{.*}} "-DFOO3" "-DFOO4"

// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_x86_64 -Xopenmp-target-backend "--bo='\"-DFOO1 -DFOO2\"'" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-CPU-OPTS3 %s
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_x86_64 -Xopenmp-target-backend=spir64_x86_64 "--bo='\"-DFOO1 -DFOO2\"'" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-CPU-OPTS3 %s
// CHK-TOOLS-CPU-OPTS3: opencl-aot{{.*}} "--bo=\"-DFOO1 -DFOO2\""

/// Check for implied options (-g -O0)
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_x86_64 -g -O0 -Xopenmp-target-backend "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-IMPLIED-OPTS-CPU %s
// RUN: %clang_cl -### -Qiopenmp -Qopenmp-targets=spir64_x86_64 -Zi -Od -Xopenmp-target-backend "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-IMPLIED-OPTS-CPU %s
// CHK-TOOLS-IMPLIED-OPTS-CPU: opencl-aot{{.*}} "--bo=-g -cl-opt-disable" "-DFOO1" "-DFOO2"

// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_gen -g -O0 -Xopenmp-target-backend "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-IMPLIED-OPTS-GEN %s
// RUN: %clang_cl -### -Qiopenmp -Qopenmp-targets=spir64_gen -Zi -Od -Xopenmp-target-backend "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-IMPLIED-OPTS-GEN %s
// CHK-TOOLS-IMPLIED-OPTS-GEN: ocloc{{.*}} "-options" "-g -cl-take-global-address -cl-match-sincospi -cl-opt-disable" "-DFOO1" "-DFOO2"

// passing -cl-no-match-sincospi should not set the default, and should also
// be stripped from the ocloc call, as it isn't really a valid option.
// RUN: %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_gen -Xopenmp-target-backend -cl-no-match-sincospi %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-GEN-NO-SINCOSPI %s
// RUN: %clang_cl -### -Qiopenmp -Qopenmp-targets=spir64_gen -Zi -Od -Xs -cl-no-match-sincospi %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-GEN-NO-SINCOSPI %s
// CHK-TOOLS-GEN-NO-SINCOSPI-NOT: "-cl-match-sincospi"
// CHK-TOOLS-GEN-NO-SINCOSPI-NOT: "-cl-no-match-sincospi"

/// Check -Xopenmp-target-linker option passing
// RUN:   %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_gen -Xopenmp-target-linker "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-GEN-OPTS2 %s
// CHK-TOOLS-GEN-OPTS2: ocloc{{.*}} "-output" {{.*}} "-output_no_suffix" {{.*}} "-DFOO1" "-DFOO2"
// CHK-TOOLS-GEN-OPTS2-NOT: clang-offload-wrapper{{.*}} "-link-opts={{.*}}

// RUN:   %clang -### -target x86_64-unknown-linux-gnu -fiopenmp -fopenmp-targets=spir64_x86_64 -Xopenmp-target-linker "-DFOO1 -DFOO2" %s 2>&1 \
// RUN:   | FileCheck -check-prefix=CHK-TOOLS-CPU-OPTS2 %s
// CHK-TOOLS-CPU-OPTS2: opencl-aot{{.*}} "-DFOO1" "-DFOO2"
// CHK-TOOLS-CPU-OPTS2-NOT: clang-offload-wrapper{{.*}} "-link-opts=-DFOO1 -DFOO2"

