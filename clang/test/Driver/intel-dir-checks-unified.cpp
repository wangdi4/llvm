// REQUIRES: intel_unified_layout

// default header behavior with --intel
// RUN: %clang -### -c --intel -target x86_64-unknown-linux-gnu %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-HEADER %s
// RUN: %clang_cl -### -c --intel %s 2>&1 | FileCheck -check-prefix CHECK-INTEL-HEADER %s
// CHECK-INTEL-HEADER: "-internal-isystem" "{{.*}}..{{(/|\\\\)}}opt{{/|\\\\}}compiler{{(/|\\\\)}}include"

// default libs with --intel (Linux)
// RUN: touch %t.o
// RUN: %clang -### -no-canonical-prefixes --intel -target x86_64-unknown-linux --gcc-toolchain="" --sysroot=%S/Inputs/basic_linux_tree %t.o 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS %s
// CHECK-INTEL-LIBS: "{{.*}}ld{{(.exe)?}}" "--sysroot=[[SYSROOT:[^"]+]]"
// CHECK-INTEL-LIBS: -L{{.*}}bin{{(/|\\\\)}}..{{(/|\\\\)}}..{{(/|\\\\)}}lib"
// CHECK-INTEL-LIBS: "-L[[SYSROOT]]/usr/lib/gcc/x86_64-unknown-linux/10.2.0"
// CHECK-INTEL-LIBS: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-INTEL-LIBS: "-Bstatic" "-lirng" "-Bdynamic"
// CHECK-INTEL-LIBS: "-lgcc"
// CHECK-INTEL-LIBS: "-Bstatic" "-lirc" "-Bdynamic"
// CHECK-INTEL-LIBS: "-ldl"
// CHECK-INTEL-LIBS: "-Bstatic" "-lirc_s" "-Bdynamic"

// RUN: touch %t.o
// RUN: %clang -### --intel -target i386-unknown-linux-gnu %t.o 2>&1 | FileCheck -check-prefix CHECK-INTEL-LIBS32 %s
// CHECK-INTEL-LIBS32: "-L{{.*}}bin{{(/|\\\\)}}..{{(/|\\\\)}}..{{(/|\\\\)}}lib32"
// CHECK-INTEL-LIBS32: "-Bstatic" "-lsvml" "-Bdynamic"
// CHECK-INTEL-LIBS32: "-Bstatic" "-lirng" "-Bdynamic"
// CHECK-INTEL-LIBS32: "-Bstatic" "-lirc" "-Bdynamic"
// CHECK-INTEL-LIBS32: "-Bstatic" "-lirc_s" "-Bdynamic"

// print-search-dirs should contain the Intel lib dir
// RUN: %clang --intel -target x86_64-unknown-linux-gnu --print-search-dirs 2>&1 | FileCheck -check-prefix CHECK-SEARCH-DIRS %s
// CHECK-SEARCH-DIRS: libraries:{{.*}} {{.*}}bin{{(/|\\)}}..{{(/|\\)}}..{{(/|\\)}}lib{{.*}}

// --dpcpp link defaults
// RUN: touch %t.o
// RUN: %clang -### -no-canonical-prefixes --dpcpp -target x86_64-unknown-linux --gcc-toolchain="" --sysroot=%S/Inputs/basic_linux_tree %t.o 2>&1 | FileCheck -check-prefix CHECK-DPCPP-LIBS %s
// CHECK-DPCPP-LIBS: "{{.*}}ld{{(.exe)?}}" "--sysroot=[[SYSROOT:[^"]+]]"
// CHECK-DPCPP-LIBS: "-L{{.*}}bin{{(/|\\\\)}}..{{(/|\\\\)}}..{{(/|\\\\)}}lib"
// CHECK-DPCPP-LIBS: "-L[[SYSROOT]]/usr/lib/gcc/x86_64-unknown-linux/10.2.0"
// CHECK-DPCPP-LIBS: "-lsvml"
// CHECK-DPCPP-LIBS: "-lirc"
