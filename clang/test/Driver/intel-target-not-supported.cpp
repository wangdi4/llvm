/// nvptx and amdgcn are not supported targets, unless SYCL is involved.

// RUN: %clangxx --intel --target=nvptx64-nvidia-cuda -c %s -### 2>&1 \
// RUN:  | FileCheck %s --check-prefix=NVPTX
// RUN: %clang_cl --intel -target nvptx64-nvidia-cuda -c %s -### 2>&1 \
// RUN:  | FileCheck %s --check-prefix=NVPTX
// RUN: %clangxx --intel --target=amdgcn-amd-amdhsa -c %s -### 2>&1 \
// RUN:  | FileCheck %s --check-prefix=AMDGCN
// RUN: %clang_cl --intel -target amdgcn-amd-amdhsa -c %s -### 2>&1 \
// RUN:  | FileCheck %s --check-prefix=AMDGCN
// NVPTX: error: target 'nvptx64-nvidia-cuda' is not supported
// AMDGCN: error: target 'amdgcn-amd-amdhsa' is not supported

// RUN: %clangxx --intel -fsycl -fsycl-targets=nvptx64-nvidia-cuda -c %s -### 2>&1 \
// RUN:  | FileCheck %s --check-prefix=NOT_SUPPORTED
// RUN: %clangxx --intel -fsycl -fsycl-targets=amdgcn-amd-amdhsa -c %s -### 2>&1 \
// RUN:  | FileCheck %s --check-prefix=NOT_SUPPORTED
// NOT_SUPPORTED-NOT: not supported
