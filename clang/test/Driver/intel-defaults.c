// default behavior with --intel
// RUN: %clang -### -c --intel %s 2>&1 | FileCheck -check-prefixes=CHECK-INTEL,CHECK-INTEL-LIN %s
// RUN: %clang_cl -### -c --intel %s 2>&1 | FileCheck -check-prefixes=CHECK-INTEL,CHECK-INTEL-WIN %s
// RUN: %clang -### -c --dpcpp %s 2>&1 | FileCheck -check-prefix CHECK-INTEL %s
// RUN: %clang_cl -### -c --dpcpp %s 2>&1 | FileCheck -check-prefixes=CHECK-INTEL,CHECK-INTEL-WIN %s
// CHECK-INTEL: "-fveclib=SVML"
// CHECK-INTEL-NOT: "-relaxed-aliasing"
// CHECK-INTEL-WIN: "-ffunction-sections"
// CHECK-INTEL: "-O2"
// CHECK-INTEL-WIN: "-Wno-c++11-narrowing"
// CHECK-INTEL-WIN: "-malign-double"
// CHECK-INTEL-WIN: "-fuse-line-directives"
// CHECK-INTEL-LIN: "-fheinous-gnu-extensions"
// CHECK-INTEL: "-vectorize-loops"
// CHECK-INTEL: "-fintel-compatibility"
// CHECK-INTEL: "-fintel-libirc-allowed"
// CHECK-INTEL: "-mllvm" "-disable-hir-generate-mkl-call"
// CHECK-INTEL: "-mllvm" "-intel-abi-compatible=true"

// RUN: %clang -### -c --intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-ZP-LIN %s
// RUN: %clang_cl -### -c --intel %s 2>&1 | FileCheck -check-prefixes=CHECK-INTEL-ZP-WIN %s
// CHECK-INTEL-ZP-WIN: "-fpack-struct=16"
// CHECK-INTEL-ZP-LIN-NOT: "-fpack-struct=16"

// RUN: %clang -### -c --intel -g %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-G %s
// RUN: %clang_cl -### -c --intel -Zi %s 2>&1 | FileCheck -check-prefixes=CHECK-INTEL-G %s
// RUN: %clang -### -c --intel -g -O2 %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-G-O2 %s
// RUN: %clang_cl -### -c --intel -Zi -O2 %s 2>&1 | FileCheck -check-prefixes=CHECK-INTEL-G-O2 %s
// CHECK-INTEL-G-NOT: "-O2"
// CHECK-INTEL-G-O2: "-O2"

// /GS is not default
// RUN: %clang_cl -### -c --intel %s 2>&1 | FileCheck -check-prefixes=CHECK-INTEL-GS %s
// CHECK-INTEL-GS-NOT: "-stack-protector"

// ms-volatile is not default
// RUN: %clang_cl -### -c --intel %s 2>&1 | FileCheck -check-prefixes=CHECK-INTEL-VOLATILE %s
// CHECK-INTEL-VOLATILE-NOT: "-fms-volatile"

// -S does implies -fno-verbose-asm and does not set syntax=intel
// RUN: %clang_cl -### -S --intel %s 2>&1 | FileCheck -check-prefixes=CHECK-INTEL-ASM %s
// CHECK-INTEL-ASM: "-fno-verbose-asm"
// CHECK-INTEL-ASM-NOT: "-x86-asm-syntax=intel"

// default header behavior with --intel
// RUN: %clang -### -c --intel -target x86_64-unknown-linux-gnu %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-HEADER %s
// RUN: %clang_cl -### -c --intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-HEADER %s
// CHECK-INTEL-HEADER: "-internal-isystem" "{{.*}}..{{(/|\\\\)}}compiler{{(/|\\\\)}}include"

// -O2 should be not be set when any other -O is passed
// RUN: %clang -### -c --intel -O0 %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-O0 %s
// RUN: %clang -### -c --intel -O1 %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-O1 %s
// RUN: %clang_cl -### -c --intel -Od %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-O0 %s
// RUN: %clang_cl -### -c --intel -O1 %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-OS %s
// CHECK-INTEL-O0: "-O0"
// CHECK-INTEL-O0-NOT: "-O2"
// CHECK-INTEL-O1: "-O{{(1|s)}}"
// CHECK-INTEL-O1-NOT: "-O2"
// CHECK-INTEL-OS: "-Os"
// CHECK-INTEL-OS-NOT: "-O2"

// default libs with --intel (Linux)
// RUN: touch %t.o
// RUN: %clang -### -no-canonical-prefixes --intel -target x86_64-unknown-linux --gcc-toolchain="" --sysroot=%S/Inputs/basic_linux_tree %t.o 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// CHECK-INTEL-LIBS: "{{.*}}ld{{(.exe)?}}" "--sysroot=[[SYSROOT:[^"]+]]"
// CHECK-INTEL-LIBS: "-L{{.*}}..{{(/|\\\\)}}compiler{{(/|\\\\)}}lib{{(/|\\\\)}}intel64_lin" "-L{{.*}}bin{{(/|\\\\)}}..{{(/|\\\\)}}lib"
// CHECK-INTEL-LIBS: "-L[[SYSROOT]]/usr/lib/gcc/x86_64-unknown-linux/10.2.0"
// CHECK-INTEL-LIBS: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-INTEL-LIBS: "-Bstatic" "-lirng" "-Bdynamic"
// CHECK-INTEL-LIBS: "-lgcc"
// CHECK-INTEL-LIBS: "-Bstatic" "-lirc" "-Bdynamic"
// CHECK-INTEL-LIBS: "-ldl"
// CHECK-INTEL-LIBS: "-Bstatic" "-lirc_s" "-Bdynamic"

// RUN: touch %t.o
// RUN: %clang -### -shared --intel -target x86_64-unknown-linux %t.o 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED %s
// RUN: %clang -### -mcmodel=medium --intel -target x86_64-unknown-linux %t.o 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED %s
// CHECK-INTEL-LIBS-SHARED: "-lsvml"
// CHECK-INTEL-LIBS-SHARED: "-lirng"
// CHECK-INTEL-LIBS-SHARED: "-lgcc"
// CHECK-INTEL-LIBS-SHARED: "-lintlc"
// CHECK-INTEL-LIBS-SHARED: "-ldl"
// CHECK-INTEL-LIBS-SHARED: "-lirc_s"

// RUN: %clang -### -shared --intel -target i386-unknown-linux %t.o 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-SHARED32 %s
// CHECK-INTEL-LIBS-SHARED32: "-lsvml"
// CHECK-INTEL-LIBS-SHARED32: "-lirng"
// CHECK-INTEL-LIBS-SHARED32: "-lgcc"
// CHECK-INTEL-LIBS-SHARED32: "-lirc_pic"
// CHECK-INTEL-LIBS-SHARED32: "-ldl"
// CHECK-INTEL-LIBS-SHARED32: "-lirc_s"

// default libs with --intel (Windows)
// RUN: %clang_cl -### --intel -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-WIN %s
// CHECK-INTEL-LIBS-WIN: "--dependent-lib=libircmt"
// CHECK-INTEL-LIBS-WIN: "--dependent-lib=svml_dispmt"
// CHECK-INTEL-LIBS-WIN: "--dependent-lib=libdecimal"

// default libs with --intel (Windows)
// RUN: touch %t.obj
// RUN: %clang -### -target x86_64-pc-windows-msvc --intel %t.obj 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS-WIN2 %s
// CHECK-INTEL-LIBS-WIN2: "-defaultlib:libircmt"
// CHECK-INTEL-LIBS-WIN2: "-defaultlib:svml_dispmt"
// CHECK-INTEL-LIBS-WIN2: "-defaultlib:libdecimal"

// RUN: touch %t.o
// RUN: %clang -### --intel -target i386-unknown-linux-gnu %t.o 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS32 %s
// CHECK-INTEL-LIBS32: "-L{{.*}}..{{(/|\\\\)}}compiler{{(/|\\\\)}}lib{{(/|\\\\)}}ia32_lin" "-L{{.*}}bin{{(/|\\\\)}}..{{(/|\\\\)}}lib"
// CHECK-INTEL-LIBS32: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-INTEL-LIBS32: "-Bstatic" "-lirng" "-Bdynamic"
// CHECK-INTEL-LIBS32: "-Bstatic" "-lirc" "-Bdynamic"
// CHECK-INTEL-LIBS32: "-Bstatic" "-lirc_s" "-Bdynamic"

// -fveclib=SVML can be overridden
// RUN: %clang -### -c --intel -fveclib=none %s 2>&1 | FileCheck -check-prefix CHECK-VECLIB %s
// CHECK-VECLIB: "-fveclib=none"
// CHECK-VECLIB-NOT: "-fveclib=SVML"

// -stdlib=libc++ settings
// RUN: %clangxx -### --intel -target x86_64-unknown-linux -stdlib=libc++ -### %t.o 2>&1 | FileCheck -check-prefix=CHECK-LIBCXX %s
// CHECK-LIBCXX: "-lc++" "-lc++abi"
// CHECK-LIBCXX" "-lpthread"

// loopopt settings
// RUN: %clang -### --intel -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT %s
// RUN: %clang_cl -### --intel -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT %s
// CHECK-INTEL-LOOPOPT: "-mllvm" "-loopopt=1" "-floopopt-pipeline=light"

// RUN: %clang -### --intel -mllvm -loopopt=1 -xAVX2 -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT1 %s
// RUN: %clang_cl -### --intel -mllvm -loopopt=1 -QxAVX2 -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT1 %s
// CHECK-INTEL-LOOPOPT1: "-mllvm" "-loopopt=1" {{.*}} "-floopopt-pipeline=light"
// CHECK-INTEL-LOOPOPT1-NOT: "-mllvm" "-loopopt=0"
// CHECK-INTEL-LOOPOPT1-NOT: floopopt-pipeline=none

// RUN: %clang -### --intel -mllvm -loopopt -xAVX2 -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT2 %s
// RUN: %clang_cl -### --intel -mllvm -loopopt -QxAVX2 -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT2 %s
// CHECK-INTEL-LOOPOPT2: "-mllvm" "-loopopt" {{.*}} "-floopopt-pipeline=full"
// CHECK-INTEL-LOOPOPT2-NOT: "-mllvm" "-loopopt=0" "-floopopt-pipeline=none"

// RUN: %clang -### --intel -c -xAVX %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT-AVX %s
// RUN: %clang_cl -### --intel -c -QxAVX %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT-AVX %s
// CHECK-INTEL-LOOPOPT-AVX: "-mllvm" "-loopopt" "-floopopt-pipeline=full"
// CHECK-INTEL-LOOPOPT-AVX-NOT: "-mllvm" "-enable-lv"

// RUN: %clang -### -Xclang -floopopt-pipeline=none --intel -c -xAVX %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT-PIPELINE-AVX %s
// RUN: %clang_cl -### -Xclang -floopopt-pipeline=none --intel -c -QxAVX %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT-PIPELINE-AVX %s
// CHECK-INTEL-LOOPOPT-PIPELINE-AVX-DAG: "-mllvm" "-loopopt"
// CHECK-INTEL-LOOPOPT-PIPELINE-AVX-DAG: "-floopopt-pipeline=none"
// CHECK-INTEL-LOOPOPT-PIPELINE-AVX-NOT: "-floopopt-pipeline=full"

// RUN: %clang -### --intel -c -flto -qopt-mem-layout-trans=4 -Ofast %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT-LIGHT %s
// RUN: %clang_cl -### --intel -c -Qipo -Qopt-mem-layout-trans:4 -Ofast %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT-LIGHT %s
// CHECK-INTEL-LOOPOPT-LIGHT: "-mllvm" "-loopopt=1" "-floopopt-pipeline=light"
// CHECK-INTEL-LOOPOPT-LIGHT-NOT: "-mllvm" "-enable-lv"

// RUN: %clang_cl -### --intel -c -O3 %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-O3 %s
// CHECK-INTEL-O3: "-O3"

// -fast settings
// RUN: %clang -### --intel -c -fast %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT-FAST %s
// RUN: %clang_cl -### --intel -c -fast %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT-FAST %s
// CHECK-INTEL-LOOPOPT-FAST: "-flto=full" "-flto-unit"
// CHECK-INTEL-LOOPOPT-FAST: "-O3"
// CHECK-INTEL-LOOPOPT-FAST: "-mllvm" "-loopopt" "-floopopt-pipeline=full"
// CHECK-INTEL-LOOPOPT-FAST-NOT: "-target-cpu" "x86_64"
// CHECK-INTEL-LOOPOPT-FAST-NOT: "-mllvm" "-enable-lv"

// disable LTO in fast
// RUN: %clang -### --intel -c -fast -fno-lto %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-FAST-NOLTO %s
// RUN: %clang_cl -### --intel -c -fast -Qipo- %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-FAST-NOLTO %s
// CHECK-INTEL-FAST-NOLTO-NOT: "-flto=full"
// CHECK-INTEL-FAST-NOLTO: "-O3"
// CHECK-INTEL-FAST-NOLTO: "-mllvm" "-loopopt" "-floopopt-pipeline=full"

// Profiling lib not linked in with -nodefaultlibs
// RUN: %clang -target i686-pc-linux-gnu -### %s --intel -fprofile-generate -nodefaultlibs 2>&1 \
// RUN:   | FileCheck -check-prefix CHECK-PROFLIB %s
// RUN: %clang -target i686-pc-linux-gnu -### %s --intel -fprofile-instr-generate -nodefaultlibs 2>&1 \
// RUN:   | FileCheck -check-prefix CHECK-PROFLIB %s
// CHECK-PROFLIB-NOT: clang_rt.profile

// Debug kind is limited
// RUN: %clang -### --intel -c -g %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-DEBUG %s
// RUN: %clang_cl -### --intel -c -Zi %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-DEBUG %s
// CHECK-INTEL-DEBUG: "-debug-info-kind=limited"

// help information
// RUN: %clang -help 2>&1 | FileCheck -check-prefix CHECK-INTEL-HELP %s
// CHECK-INTEL-HELP: Intel(R) oneAPI DPC++/C++ Compiler

// -fp-model=fast and -ffast-math should set preserve-sign
// RUN: %clang -### -ffp-model=fast -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix FP_MODEL_FAST %s
// RUN: %clang -### -ffast-math -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix FP_MODEL_FAST %s
// FP_MODEL_FAST: "-fdenormal-fp-math=preserve-sign,preserve-sign"

// Optimization level O1 or higher should set preserve-sign
// RUN: %clang -### --intel -O0 -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix NO_FP_MODEL_FAST %s
// RUN: %clang -### --intel -O1 -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix FP_MODEL_FAST %s
// RUN: %clang -### --intel -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix FP_MODEL_FAST %s
// NO_FP_MODEL_FAST-NOT: "-fdenormal-fp-math=preserve-sign,preserve-sign"

// -fp-model options should override -fdenormal-fp-math settings set by optimization level regardless of ordering
// RUN: %clang -### --intel -O0 -ffp-model=fast -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix FP_MODEL_FAST %s
// RUN: %clang -### --intel -ffp-model=fast -O0 -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix FP_MODEL_FAST %s
// RUN: %clang -### --intel -O1 -ffp-model=precise -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix NO_FP_MODEL_FAST %s
// RUN: %clang -### --intel -ffp-model=precise -O1 -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix NO_FP_MODEL_FAST %s
// RUN: %clang -### --intel -O1 -ffp-model=strict -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix NO_FP_MODEL_FAST %s
// RUN: %clang -### --intel -ffp-model=strict -O1 -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix NO_FP_MODEL_FAST %s

// print-search-dirs should contain the Intel lib dir
// RUN: %clang --intel -target x86_64-unknown-linux-gnu --print-search-dirs 2>&1 | FileCheck -check-prefix CHECK-SEARCH-DIRS %s
// CHECK-SEARCH-DIRS: libraries:{{.*}} {{.*}}compiler{{(/|\\)}}lib{{(/|\\)}}intel64_lin{{.*}}

// Add -header-base-path for Windows
// RUN: %clang_cl --intel -### -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK-HEADER-BASE %s
// RUN: %clangxx --intel -### -target x86_64-pc-windows-msvc -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix CHECK-HEADER-BASE %s
// CHECK-HEADER-BASE: "-header-base-path"

// -fp-model=fast should be the default for Intel mode, this also makes sure
// that the intel defaults match -fp-model=fast
// RUN: %clangxx -### --intel -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=FP_MODEL_FAST_INTEL %s
// RUN: %clangxx -### -fp-model=fast -c %s 2>&1 \
// RUN:  | FileCheck -check-prefix=FP_MODEL_FAST_INTEL %s
// FP_MODEL_FAST_INTEL: "-menable-no-infs"
// FP_MODEL_FAST_INTEL: "-menable-no-nans"
// FP_MODEL_FAST_INTEL: "-fapprox-func"
// FP_MODEL_FAST_INTEL: "-menable-unsafe-fp-math"
// FP_MODEL_FAST_INTEL: "-fno-signed-zeros"
// FP_MODEL_FAST_INTEL: "-mreassociate"
// FP_MODEL_FAST_INTEL: "-freciprocal-math"
// FP_MODEL_FAST_INTEL: "-fdenormal-fp-math=preserve-sign,preserve-sign"
// FP_MODEL_FAST_INTEL: "-ffp-contract=fast"
// FP_MODEL_FAST_INTEL: "-ffast-math"
// FP_MODEL_FAST_INTEL: "-ffinite-math-only"

// Use of -ffreestanding should disable -fveclib=SVML and libirc extensions
// RUN: %clang -### -c -ffreestanding --intel %s 2>&1 | FileCheck -check-prefixes=CHECK_FREESTANDING %s
// RUN: %clang_cl -### -c -Qfreestanding --intel %s 2>&1 | FileCheck -check-prefixes=CHECK_FREESTANDING %s
// CHECK_FREESTANDING-NOT: "-fveclib=SVML"
// CHECK-FREESTANDING-NOT: "-fintel-libirc-allowed"
