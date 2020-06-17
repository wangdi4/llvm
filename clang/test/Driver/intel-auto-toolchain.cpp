// Test that when in Intel mode, use the toolchain in which gcc is found
//
// UNSUPPORTED: windows
// We will make a copy of the ubuntu tree and add what we need for the
// toolchain check.
// RUN: mkdir -p %t_dir
// RUN: cp -r %S/Inputs/ubuntu_11.04_multiarch_tree %t_dir
// RUN: mkdir -p %t_dir/ubuntu_11.04_multiarch_tree/usr/bin
// RUN: touch %t_dir/ubuntu_11.04_multiarch_tree/usr/bin/g++
// RUN: chmod 777 %t_dir/ubuntu_11.04_multiarch_tree/usr/bin/g++
// RUN: env PATH=%t_dir/ubuntu_11.04_multiarch_tree/usr/bin \
// RUN:  %clangxx --intel %s --target=i386-unknown-linux -### -o %t 2>&1 \
// RUN:   | FileCheck %s
//
// Test for header search toolchain detection.
// CHECK: "-internal-isystem"
// CHECK: "[[TOOLCHAIN:[^"]+]]/usr/lib/i386-linux-gnu/gcc/i686-linux-gnu/4.5/../../../../../include/c++/4.5"
// CHECK: "-internal-isystem"
// CHECK: "[[TOOLCHAIN]]/usr/lib/i386-linux-gnu/gcc/i686-linux-gnu/4.5/../../../../../include/c++/4.5/i686-linux-gnu"
// CHECK: "-internal-isystem"
// CHECK: "[[TOOLCHAIN]]/usr/lib/i386-linux-gnu/gcc/i686-linux-gnu/4.5/../../../../../include/c++/4.5/backward"
// CHECK: "-internal-isystem" "/usr/local/include"
//
// Test for linker toolchain detection. Note that only the '-L' flags will use
// the same precise formatting of the path as the '-internal-system' flags
// above, so we just blanket wildcard match the 'crtbegin.o'.
// CHECK: "{{[^"]*}}ld{{(.exe)?}}"
// CHECK: "{{[^"]*}}/usr/lib/i386-linux-gnu/gcc/i686-linux-gnu/4.5{{/|\\\\}}crtbegin.o"
// CHECK: "-L[[TOOLCHAIN]]/usr/lib/i386-linux-gnu/gcc/i686-linux-gnu/4.5"
// CHECK: "-L[[TOOLCHAIN]]/usr/lib/i386-linux-gnu/gcc/i686-linux-gnu/4.5/../../../.."
