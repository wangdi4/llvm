/// Intel specific optimization options
// REQUIRES: x86-registered-target

// RUN: %clang -qopt-mem-layout-trans -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT %s
// RUN: %clang_cl -Qopt-mem-layout-trans -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT %s
// RUN: %clang -qopt-mem-layout-trans=2 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT %s
// RUN: %clang_cl -Qopt-mem-layout-trans:2 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=LAYOUT_TRANS_DEFAULT %s
// LAYOUT_TRANS_DEFAULT: "-mllvm" "-irmover-type-merging=false"
// LAYOUT_TRANS_DEFAULT: "-mllvm" "-spill-freq-boost=true"

// RUN: touch %t.o
// RUN: %clang -qopt-mem-layout-trans -target x86_64-unknown-linux-gnu -flto -### %t.o 2>&1 \
// RUN:  | FileCheck -DOPTION=-plugin-opt= -check-prefix=LAYOUT_TRANS_LTO %s
// RUN: %clang_cl /Qopt-mem-layout-trans -flto -fuse-ld=lld -### %t.o 2>&1 \
// RUN:  | FileCheck -DOPTION=-mllvm: -check-prefix=LAYOUT_TRANS_LTO %s
// LAYOUT_TRANS_LTO: "[[OPTION]]-irmover-type-merging=false"
// LAYOUT_TRANS_LTO: "[[OPTION]]-spill-freq-boost=true"

// Behavior with -#x option
// RUN: %clang -#x %s -c 2>&1 | FileCheck %s --check-prefix=CHECK-HASH-X
// RUN: %clang_cl -#x %s -c 2>&1 | FileCheck %s --check-prefix=CHECK-HASH-X
// CHECK-HASH-X: -emit-obj \
// CHECK-HASH-X: -mrelax-all \

/// -unroll support
// RUN: %clang -unroll3 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=UNROLL,UNROLL3 %s
// RUN: %clang_cl -Qunroll3 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=UNROLL,UNROLL3 %s
// RUN: %clang -unroll -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=UNROLL %s
// RUN: %clang_cl -Qunroll -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=UNROLL %s
// RUN: %clang -unroll0 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=NO-UNROLL %s
// RUN: %clang_cl -Qunroll0 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefixes=NO-UNROLL %s
// UNROLL: "-funroll-loops"
// UNROLL3: "-mllvm" "-hir-general-unroll-max-factor=3"
// UNROLL3: "-mllvm" "-hir-complete-unroll-loop-trip-threshold=3"
// UNROLL3: "-mllvm" "-hir-unroll-and-jam-max-factor=3"
// UNROLL3: "-mllvm" "-unroll-max-count=3"
// UNROLL3: "-mllvm" "-unroll-full-max-count=3"
// NO-UNROLL: "-fno-unroll-loops"

// Behavior with -qopt-matmul and /Qopt-matmul option
// RUN: %clang -### %s -target x86_64-unknown-linux-gnu --intel -qopt-matmul 2>&1 | FileCheck %s --check-prefixes=CHECK-QOPT-MATMUL,CHECK-QOPT-MATMUL-LIN
// RUN: %clang_cl -### %s --intel /Qopt-matmul 2>&1 | FileCheck %s --check-prefixes=CHECK-QOPT-MATMUL,CHECK-QOPT-MATMUL-WIN
// CHECK-QOPT-MATMUL-NOT: "-mllvm" "-disable-hir-generate-mkl-call"
// CHECK-QOPT-MATMUL-WIN: "--dependent-lib=libmatmul"
// CHECK-QOPT-MATMUL-LIN: ld{{.*}} "-lmatmul"

// RUN: %clang -### %s -c -qno-opt-matmul 2>&1 | FileCheck %s --check-prefix=CHECK-QNO-OPT-MATMUL
// RUN: %clang_cl -### %s -c /Qopt-matmul- 2>&1 | FileCheck %s --check-prefix=CHECK-QNO-OPT-MATMUL
// CHECK-QNO-OPT-MATMUL: "-mllvm" "-disable-hir-generate-mkl-call"

//Behavior with -Qoption,tool,arg
// RUN: %clang -### %s -c -Qoption,asm,--compress-debug-sections 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-ASM
// RUN: %clang_cl -### %s -c /Qoption,asm,--compress-debug-sections 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-ASM
// CHECK-QOPTION-ASM: "--compress-debug-sections"
// RUN: %clang -### %s -c -Qoption,assembler,-Ifoo_dir 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-ASSEMBLER
// RUN: %clang_cl -### %s -c /Qoption,assembler,-Ifoo_dir 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-ASSEMBLER
// CHECK-QOPTION-ASSEMBLER: "-Ifoo_dir"
// RUN: %clang -### %s -c -Qoption,asm,--compress-debug-sections,-fdebug-compilation-dir=.,--noexecstack 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-ASM-ARGS
// RUN: %clang_cl -### %s -c /Qoption,asm,--compress-debug-sections,-fdebug-compilation-dir=.,--noexecstack 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-ASM-ARGS
// CHECK-QOPTION-ASM-ARGS: "--compress-debug-sections" "-fdebug-compilation-dir" "."
// CHECK-QOPTION-ASM-ARGS: "-mnoexecstack"

// RUN: %clang -### %s -c -Qoption,preprocessor,-MMD 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-PREPROCESSOR
// RUN: %clang_cl -### %s -c /Qoption,preprocessor,-MMD 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-PREPROCESSOR
// RUN: %clang -### %s -c -Qoption,cpp,-MMD 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-PREPROCESSOR
// RUN: %clang_cl -### %s -c /Qoption,cpp,-MMD 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-PREPROCESSOR
// CHECK-QOPTION-PREPROCESSOR: "-MMD"

// RUN: %clang -target x86_64-unknown-linux-gnu -### %s -Qoption,ld,--no-demangle 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-LD-ARG
// RUN: %clang -target x86_64-unknown-linux-gnu -### %s -Qoption,link,--no-demangle 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-LD-ARG
// RUN: %clang_cl -### %s /Qoption,ld,--no-demangle 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTIONCL-LD-ARG
// RUN: %clang_cl -### %s /Qoption,link,--no-demangle 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTIONCL-LD-ARG
// CHECK-QOPTION-LD-ARG: ld{{.*}} "--no-demangle"
// CHECK-QOPTIONCL-LD-ARG: link{{.*}} "--no-demangle"

// RUN: %clang -c -### %s -Qoption,compiler,-MP 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-COMPILER-ARG
// RUN: %clang_cl -c -### %s /Qoption,compiler,-MP 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-COMPILER-ARG
// RUN: %clang -c -### %s -Qoption,clang,-MP 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-COMPILER-ARG
// RUN: %clang_cl -c -### %s /Qoption,clang,-MP 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-COMPILER-ARG
// CHECK-QOPTION-COMPILER-ARG: "-MP"
// RUN: %clang -c -### %s -Qoption,compiler,-MP,-mintrinsic-promote,-extended_float_types 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-COMPILER-ARGS
// RUN: %clang_cl -c -### %s /Qoption,compiler,-MP,-mintrinsic-promote,-extended_float_types 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-COMPILER-ARGS
// CHECK-QOPTION-COMPILER-ARGS: "-MP" "-mintrinsic-promote" "-extended_float_types"

// RUN: %clang -target x86_64-unknown-linux-gnu -### %s -Qoption,l,--no-demangle -Qoption,compiler,-MP,-mintrinsic-promote,--extended_float_types -Qoption,cpp,-MMD -Qoption,a,--compress-debug-sections 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTION-TOOLS-ARGS
// CHECK-QOPTION-TOOLS-ARGS: "--compress-debug-sections"
// CHECK-QOPTION-TOOLS-ARGS: "-MMD"
// CHECK-QOPTION-TOOLS-ARGS: "-MP" "-mintrinsic-promote" "--extended_float_types"
// CHECK-QOPTION-TOOLS-ARGS: ld{{.*}} "--no-demangle"
// RUN: %clang_cl -### %s /Qoption,l,--no-demangle /Qoption,compiler,-MP,-mintrinsic-promote,--extended_float_types /Qoption,cpp,-MMD /Qoption,a,--compress-debug-sections 2>&1 | FileCheck %s --check-prefix=CHECK-QOPTIONCL-TOOLS-ARGS
// CHECK-QOPTIONCL-TOOLS-ARGS: "--compress-debug-sections"
// CHECK-QOPTIONCL-TOOLS-ARGS: "-MMD"
// CHECK-QOPTIONCL-TOOLS-ARGS: "-MP" "-mintrinsic-promote" "--extended_float_types"
// CHECK-QOPTIONCL-TOOLS-ARGS: link{{.*}} "--no-demangle"


// Behavior with /Qopt/-qopt-multiple-gather-scatter-by-shuffles option
// RUN: %clang -### %s -c -qopt-multiple-gather-scatter-by-shuffles 2>&1 | FileCheck %s --check-prefix=CHECK-GATHER-SCATTER-SHUFFLES
// RUN: %clang_cl -### %s -c /Qopt-multiple-gather-scatter-by-shuffles 2>&1 | FileCheck %s --check-prefix=CHECK-GATHER-SCATTER-SHUFFLES
// CHECK-GATHER-SCATTER-SHUFFLES: "-mllvm" "-vplan-vls-level=always"
// RUN: %clang -### %s -c -qno-opt-multiple-gather-scatter-by-shuffles 2>&1 | FileCheck %s --check-prefix=CHECK-NO-GATHER-SCATTER-SHUFFLES
// RUN: %clang_cl -### %s -c /Qopt-multiple-gather-scatter-by-shuffles- 2>&1 | FileCheck %s --check-prefix=CHECK-NO-GATHER-SCATTER-SHUFFLES
// CHECK-NO-GATHER-SCATTER-SHUFFLES: "-mllvm" "-vplan-vls-level=never"

// Behavior with -qopt-dynamic-align option
// RUN: %clang -### %s -c -qopt-dynamic-align 2>&1 | FileCheck %s --check-prefix=CHECK-DYNAMIC-ALIGN
// RUN: %clang_cl -### %s -c /Qopt-dynamic-align 2>&1 | FileCheck %s --check-prefix=CHECK-DYNAMIC-ALIGN
// CHECK-DYNAMIC-ALIGN: "-mllvm" "-vplan-enable-peeling=true"

// Behavior with -qno-opt-dynamic-align option
// RUN: %clang -### %s -c -qno-opt-dynamic-align 2>&1 | FileCheck %s --check-prefix=CHECK-NO-DYNAMIC-ALIGN
// RUN: %clang_cl -### %s -c /Qopt-dynamic-align- 2>&1 | FileCheck %s --check-prefix=CHECK-NO-DYNAMIC-ALIGN
// CHECK-NO-DYNAMIC-ALIGN: "-mllvm" "-vplan-enable-peeling=false"

// Behavior with -fvec-with-mask option
// RUN: %clang -### %s -c -fvec-with-mask 2>&1 | FileCheck %s --check-prefix=CHECK-VECTORIZE-MASKED-MODE
// RUN: %clang_cl -### %s -c /Qvec-with-mask 2>&1 | FileCheck %s --check-prefix=CHECK-VECTORIZE-MASKED-MODE
// CHECK-VECTORIZE-MASKED-MODE: "-mllvm" "-vplan-enable-masked-variant=true"

// Behavior with -fvec-peel-loops option
// RUN: %clang -### %s -c -fvec-peel-loops 2>&1 | FileCheck %s --check-prefix=CHECK-VECTORIZE-PEEL-LOOPS
// RUN: %clang_cl -### %s -c /Qvec-peel-loops 2>&1 | FileCheck %s --check-prefix=CHECK-VECTORIZE-PEEL-LOOPS
// CHECK-VECTORIZE-PEEL-LOOPS: "-mllvm" "-vplan-enable-vectorized-peel=true"

// Behavior with -fvec-remainder-loops option
// RUN: %clang -### %s -c -fvec-remainder-loops 2>&1 | FileCheck %s --check-prefix=CHECK-VECTORIZE-REMAINDER-LOOPS
// RUN: %clang_cl -### %s -c /Qvec-remainder-loops 2>&1 | FileCheck %s --check-prefix=CHECK-VECTORIZE-REMAINDER-LOOPS
// CHECK-VECTORIZE-REMAINDER-LOOPS: "-mllvm" "-vplan-enable-masked-vectorized-remainder=true"
// CHECK-VECTORIZE-REMAINDER-LOOPS: "-mllvm" "-vplan-enable-non-masked-vectorized-remainder=true"

// Behavior with -fno-vec-with-mask option
// RUN: %clang -### %s -c -fno-vec-with-mask 2>&1 | FileCheck %s --check-prefix=CHECK-NO-VECTORIZE-MASKED-MODE
// RUN: %clang_cl -### %s -c /Qvec-with-mask- 2>&1 | FileCheck %s --check-prefix=CHECK-NO-VECTORIZE-MASKED-MODE
// CHECK-NO-VECTORIZE-MASKED-MODE: "-mllvm" "-vplan-enable-masked-variant=false"

// Behavior with -fno-vec-peel-loops option
// RUN: %clang -### %s -c -fno-vec-peel-loops 2>&1 | FileCheck %s --check-prefix=CHECK-NO-VECTORIZE-PEEL-LOOPS
// RUN: %clang_cl -### %s -c /Qvec-peel-loops- 2>&1 | FileCheck %s --check-prefix=CHECK-NO-VECTORIZE-PEEL-LOOPS
// CHECK-NO-VECTORIZE-PEEL-LOOPS: "-mllvm" "-vplan-enable-vectorized-peel=false"

// Behavior with -fno-vec-remainder-loops option
// RUN: %clang -### %s -c -fno-vec-remainder-loops 2>&1 | FileCheck %s --check-prefix=CHECK-NO-VECTORIZE-REMAINDER-LOOPS
// RUN: %clang_cl -### %s -c /Qvec-remainder-loops- 2>&1 | FileCheck %s --check-prefix=CHECK-NO-VECTORIZE-REMAINDER-LOOPS
// CHECK-NO-VECTORIZE-REMAINDER-LOOPS: "-mllvm" "-vplan-enable-masked-vectorized-remainder=false"
// CHECK-NO-VECTORIZE-REMAINDER-LOOPS: "-mllvm" "-vplan-enable-non-masked-vectorized-remainder=false"

// Behavior with /fstack-limit-register=<arg> option
// RUN: %clang -### %s -c -fstack-limit-register=et 2>&1 | FileCheck %s --check-prefix=CHECK-FSTACK-LIMIT-REGISTER
// CHECK-FSTACK-LIMIT-REGISTER: "-fstack-limit-register=et"

//Behavior with -fiopenmp-offload and -fno-iopenmp-offload option
// RUN: %clang -### %s -c -fiopenmp -fopenmp-targets=spir64 -fno-iopenmp-offload 2>&1 | FileCheck %s --check-prefix=CHECK-FNO-IOPENMP-OFFLOAD
// RUN: %clang_cl -### %s -c /Qiopenmp /Qiopenmp-offload- 2>&1 | FileCheck %s --check-prefix=CHECK-FNO-IOPENMP-OFFLOAD
// CHECK-FNO-IOPENMP-OFFLOAD: "-fno-intel-openmp-offload"
// CHECK-FNO-IOPENMP-OFFLOAD: "-mllvm" "-vpo-paropt-use-offload-metadata=false"

// Check for -target-feature with -xHOST
// RUN: %clang -### %s -c -xHOST 2>&1 | FileCheck %s --check-prefix=CHECK-XHOST
// RUN: %clang_cl -### %s -c /QxHOST 2>&1 | FileCheck %s --check-prefix=CHECK-XHOST
// CHECK-XHOST: "-cc1"{{.*}}"-target-cpu"{{.*}}"-target-feature"

// RUN: %clang -qopt-for-throughput=single-job -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=THROUGHPUT-SINGLE %s
// RUN: %clang_cl -Qopt-for-throughput:single-job -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=THROUGHPUT-SINGLE %s
// THROUGHPUT-SINGLE: "-mllvm" "-throughput-opt=1"

// RUN: %clang -qopt-for-throughput=multi-job -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=THROUGHPUT-MULTI %s
// RUN: %clang_cl -Qopt-for-throughput:multi-job -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=THROUGHPUT-MULTI %s
// THROUGHPUT-MULTI: "-mllvm" "-throughput-opt=2"

// RUN: %clang -qopt-for-throughput=badarg -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=THROUGHPUT-BADARG %s
// RUN: %clang_cl -Qopt-for-throughput=badarg -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=THROUGHPUT-BADARG %s
// THROUGHPUT-BADARG: error: invalid argument 'badarg'

// Make sure -mllvm -loopopt is passed when -x<arg> is used
// RUN: %clangxx -mllvm -loopopt -xAVX2 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK_LOOPOPT %s
// CHECK_LOOPOPT: "-mllvm" "-loopopt"

// RUN: %clangxx -mllvm -loopopt=2 -xAVX2 -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=CHECK_LOOPOPT2 %s
// CHECK_LOOPOPT2: "-mllvm" "-loopopt=2"

// Check -qopt-streaming-stores behavior
// RUN: %clang -qopt-streaming-stores=always -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=STREAMING_STORES_ALWAYS %s
// RUN: %clang_cl -Qopt-streaming-stores:always -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=STREAMING_STORES_ALWAYS %s
// RUN: %clang -qopt-streaming-stores=never -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=STREAMING_STORES_NEVER %s
// RUN: %clang_cl -Qopt-streaming-stores:never -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=STREAMING_STORES_NEVER %s
// RUN: %clang -qno-opt-streaming-stores -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=STREAMING_STORES_NEVER %s
// RUN: %clang_cl -Qopt-streaming-stores- -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=STREAMING_STORES_NEVER %s
// STREAMING_STORES_ALWAYS: "-mllvm" "-hir-nontemporal-cacheline-count=0"
// STREAMING_STORES_NEVER: "-mllvm" "-disable-hir-nontemporal-marking"

// Check for a binary "name" match
// RUN: not %clangxx --intel --- -### -c %s 2>&1 | FileCheck -check-prefix SUPPORT-CHECK1 %s
// RUN: not %clangxx --intel --dpcpp --- -### -c %s 2>&1 | FileCheck -check-prefix SUPPORT-CHECK1 %s
// SUPPORT-CHECK1: icpx: error: unsupported option '---'

// warn to use icpx when compiling .cpp files
// RUN: %clang --intel -### %s 2>&1 | FileCheck -check-prefix=WARN-TO-USE-ICPX %s
// RUN: %clang --intel -c -### %s 2>&1 | FileCheck -check-prefix=WARN-TO-USE-ICPX-OFF %s
// WARN-TO-USE-ICPX: warning: Use of 'icx' with a C++ source input file 'intel-opt-options.cpp' will not link with required C++ library. Use 'icpx' for C++ source compilation and link [-Wincompatible-compiler]
// WARN-TO-USE-ICPX-OFF-NOT: warning: Use of 'icx' with a C++ source input file 'intel-opt-options.cpp' will not link with required C++ library. Use 'icpx' for C++ source compilation and link [-Wincompatible-compiler]
