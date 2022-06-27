/// Test to be sure we work with ClearOS (x86_64-generic-linux)
// RUN: %clang -### %s -no-pie 2>&1 \
// RUN:     --target=x86_64-generic-linux -rtlib=libgcc --unwindlib=platform \
// RUN:     --gcc-toolchain="" \
// RUN:     --sysroot=%S/Inputs/intel_linux_tree \
// RUN:   | FileCheck --check-prefix=CHECK-LD-GENERIC %s
// CHECK-LD-GENERIC-NOT: warning:
// CHECK-LD-GENERIC: "{{.*}}ld{{(.exe)?}}" "--sysroot=[[SYSROOT:[^"]+]]"
// CHECK-LD-GENERIC: "--eh-frame-hdr"
// CHECK-LD-GENERIC: "-m" "elf_x86_64"
// CHECK-LD-GENERIC: "-dynamic-linker"
// CHECK-LD-GENERIC: "{{.*}}/usr/lib64/gcc/x86_64-generic-linux/11{{/|\\\\}}crtbegin.o"
// CHECK-LD-GENERIC: "-L[[SYSROOT]]/usr/lib64/gcc/x86_64-generic-linux/11"
// CHECK-LD-GENERIC: "-L[[SYSROOT]]/usr/lib64/gcc/x86_64-generic-linux/11/../../../../lib64"
// CHECK-LD-GENERIC: "-L[[SYSROOT]]/lib"
// CHECK-LD-GENERIC: "-L[[SYSROOT]]/usr/lib"
// CHECK-LD-GENERIC: "-lgcc" "--as-needed" "-lgcc_s" "--no-as-needed"
// CHECK-LD-GENERIC: "-lc"
// CHECK-LD-GENERIC: "-lgcc" "--as-needed" "-lgcc_s" "--no-as-needed"

