// default behavior with --intel
// RUN: %clang -### -c --intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL %s
// CHECK-INTEL: "-O2"
// CHECK-INTEL: "-fintel-compatibility"
// CHECK-INTEL: "-mllvm" "-intel-libirc-allowed"

// default behavior with --intel (Linux)
// RUN: %clang -### -c --intel -target x86_64-unknown-linux-gnu %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-LINUX %s
// CHECK-INTEL-LINUX: "-internal-isystem" "{{.*}}../compiler/include"

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
// CHECK-INTEL-LIBS: "-L{{.*}}../compiler/lib/intel64_lin"
// CHECK-INTEL-LIBS: "-L[[SYSROOT]]/usr/lib/gcc/x86_64-unknown-linux/4.6.0"
// CHECK-INTEL-LIBS: "-Bstatic" "-lirc" "-Bdynamic"
// CHECK-INTEL-LIBS: "-Bstatic" "-lsvml" "-Bdynamic"

// RUN: touch %t.o
// RUN: %clang -### --intel -target i386-unknown-linux-gnu %t.o 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS32 %s
// CHECK-INTEL-LIBS32: "-L{{.*}}../compiler/lib/ia32_lin"
// CHECK-INTEL-LIBS32: "-Bstatic" "-lirc" "-Bdynamic"
// CHECK-INTEL-LIBS32: "-Bstatic" "-lsvml" "-Bdynamic"
