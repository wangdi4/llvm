// default behavior with --intel
// RUN: %clang -### -c --intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL %s
// RUN: %clang -### -c -qnextgen %s 2>&1 | FileCheck -check-prefix CHECK-INTEL %s
// RUN: %clang_cl -### -c -Qnextgen %s 2>&1 | FileCheck -check-prefixes=CHECK-INTEL,CHECK-INTEL-WIN %s
// CHECK-INTEL: "-fveclib=SVML"
// CHECK-INTEL: "-O2"
// CHECK-INTEL-WIN: "-fpack-struct=16"
// CHECK-INTEL: "-vectorize-loops"
// CHECK-INTEL: "-fintel-compatibility"
// CHECK-INTEL: "-mllvm" "-disable-hir-generate-mkl-call"
// CHECK-INTEL: "-mllvm" "-intel-libirc-allowed"

// default header behavior with --intel
// RUN: %clang -### -c --intel -target x86_64-unknown-linux-gnu %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-HEADER %s
// RUN: %clang_cl -### -c --intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-HEADER %s
// CHECK-INTEL-HEADER: "-internal-isystem" "{{.*}}../compiler/include"

// -O2 should be not be set when any other -O is passed
// RUN: %clang -### -c --intel -O0 %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-O0 %s
// RUN: %clang -### -c --intel -O1 %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-O1 %s
// CHECK-INTEL-O0: "-O0"
// CHECK-INTEL-O0-NOT: "-O2"
// CHECK-INTEL-O1: "-O1"
// CHECK-INTEL-O1-NOT: "-O2"

// default libs with --intel (Linux)
// RUN: touch %t.o
// RUN: %clang -### -no-canonical-prefixes --intel -target x86_64-unknown-linux --gcc-toolchain="" --sysroot=%S/Inputs/basic_linux_tree %t.o 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// CHECK-INTEL-LIBS: "{{.*}}ld{{(.exe)?}}" "--sysroot=[[SYSROOT:[^"]+]]"
// CHECK-INTEL-LIBS: "-L{{.*}}../compiler/lib/intel64_lin" "-L{{.*}}bin/../lib"
// CHECK-INTEL-LIBS: "-L[[SYSROOT]]/usr/lib/gcc/x86_64-unknown-linux/4.6.0"
// CHECK-INTEL-LIBS: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-INTEL-LIBS: "-Bstatic" "-lirc" "-Bdynamic"

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
// CHECK-INTEL-LIBS32: "-L{{.*}}../compiler/lib/ia32_lin" "-L{{.*}}bin/../lib"
// CHECK-INTEL-LIBS32: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-INTEL-LIBS32: "-Bstatic" "-lirc" "-Bdynamic"

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
// CHECK-INTEL-LOOPOPT: "-mllvm" "-loopopt=0" "-mllvm" "-enable-lv"

// RUN: %clang -### --intel -mllvm -loopopt=1 -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT1 %s
// RUN: %clang_cl -### --intel -mllvm -loopopt=1 -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT1 %s
// CHECK-INTEL-LOOPOPT1: "-mllvm" "-loopopt=1"
// CHECK-INTEL-LOOPOPT1-NOT: "-mllvm" "-loopopt=0"

// RUN: %clang -### --intel -mllvm -loopopt -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT2 %s
// RUN: %clang_cl -### --intel -mllvm -loopopt -c %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT2 %s
// CHECK-INTEL-LOOPOPT2: "-mllvm" "-loopopt"
// CHECK-INTEL-LOOPOPT2-NOT: "-mllvm" "-loopopt=0"

// RUN: %clang -### --intel -c -xAVX %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT-AVX %s
// RUN: %clang_cl -### --intel -c -QxAVX %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT-AVX %s
// CHECK-INTEL-LOOPOPT-AVX: "-mllvm" "-loopopt"
// CHECK-INTEL-LOOPOPT-AVX-NOT: "-mllvm" "-enable-lv"

// RUN: %clang -### --intel -c -flto -qopt-mem-layout-trans=4 -Ofast %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT-LIGHT %s
// RUN: %clang_cl -### --intel -c -Qipo -Qopt-mem-layout-trans:4 -Ofast %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LOOPOPT-LIGHT %s
// CHECK-INTEL-LOOPOPT-LIGHT: "-mllvm" "-loopopt=1"
// CHECK-INTEL-LOOPOPT-AVX-NOT: "-mllvm" "-enable-lv"
