// REQUIRES: !system-solaris

// The test cannot be run on Darwin because /Users will be treated as a MSVC option.

// RUN: %clang_cl -### -target x86_64-pc-windows-msvc --offload-arch=sm_35 -fgpu-rdc \
// RUN:   --cuda-path=%S/Inputs/CUDA/usr/local/cuda \
// RUN:   /Wall -x cuda -- %s 2>&1 \
// RUN:   | FileCheck %s -check-prefix=CUDA

// RUN: %clang_cl -### -target x86_64-pc-windows-msvc --offload-arch=gfx1010 -fgpu-rdc --hip-link \
// RUN:   --rocm-path=%S/Inputs/rocm /Wall -x hip -- %s 2>&1 \
// RUN:   | FileCheck %s -check-prefix=HIP

// CUDA: "-cc1" "-triple" "nvptx64-nvidia-cuda" "-aux-triple" "x86_64-pc-windows-msvc"
// INTEL CUDA-SAME: "-Wall"
// CUDA: ptxas
// CUDA: "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-aux-triple" "nvptx64-nvidia-cuda"
// INTEL CUDA-SAME: "-Wall"
// CUDA: link

// HIP: "-cc1" "-triple" "x86_64-pc-windows-msvc{{.*}}" "-aux-triple" "amdgcn-amd-amdhsa"
// INTEL HIP-SAME: "-Wall"
// HIP: "-cc1" "-triple" "amdgcn-amd-amdhsa" "-aux-triple" "x86_64-pc-windows-msvc"
// INTEL HIP-SAME: "-Wall"
// HIP: {{lld.* "-flavor" "gnu" "-m" "elf64_amdgpu"}}
// HIP: {{link.* "amdhip64.lib"}}

// CMake uses this option when finding packages for HIP, so
// make sure it does not cause error.

// RUN: %clang_cl --print-libgcc-file-name
