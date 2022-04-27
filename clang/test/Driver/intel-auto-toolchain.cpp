// Test that when in Intel mode, use the toolchain in which gcc is found
//
// UNSUPPORTED: windows
// We will make a copy of the ubuntu tree and add what we need for the
// toolchain check.
// RUN: mkdir -p %t_dir
// RUN: cp -r %S/Inputs/ubuntu_14.04_multiarch_tree %t_dir
// RUN: mkdir -p %t_dir/ubuntu_14.04_multiarch_tree/usr/bin
// RUN: touch %t_dir/ubuntu_14.04_multiarch_tree/usr/bin/g++
// RUN: chmod 777 %t_dir/ubuntu_14.04_multiarch_tree/usr/bin/g++
// RUN: env PATH=%t_dir/ubuntu_14.04_multiarch_tree/usr/bin \
// RUN: %clangxx --intel -no-pie %s --target=x86_64-linux-gnu -### -o %t 2>&1 \
// RUN:   | FileCheck %s
//
// Test for header search toolchain detection.
// CHECK: "-internal-isystem"
// CHECK: "[[TOOLCHAIN:[^"]+]]/usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../include/c++/4.8"
// CHECK: "-internal-isystem"
// CHECK: "[[TOOLCHAIN]]/usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../include/x86_64-linux-gnu/c++/4.8"
// CHECK: "-internal-isystem"
// CHECK: "[[TOOLCHAIN]]/usr/lib/gcc/x86_64-linux-gnu/4.8/../../../../include/c++/4.8/backward"
// CHECK: "-internal-isystem" "/usr/local/include"
//
// Test for linker toolchain detection. Note that only the '-L' flags will use
// the same precise formatting of the path as the '-internal-system' flags
// above, so we just blanket wildcard match the 'crtbegin.o'.
// CHECK: "{{[^"]*}}ld{{(.exe)?}}"
// CHECK: "{{[^"]*}}/usr/lib/gcc/x86_64-linux-gnu/4.8{{/|\\\\}}crtbegin.o"
// CHECK: "-L[[TOOLCHAIN]]/usr/lib/gcc/x86_64-linux-gnu/4.8"
