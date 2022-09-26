/// When using -fsycl, -mllvm -sycl-host should be passed for the host compile
// RUN: %clangxx -fsycl -c %s -### 2>&1 \
// RUN:   | FileCheck --check-prefix=SYCL_HOST %s
// SYCL_HOST-NOT: clang{{.*}} "-fsycl-is-device" {{.*}} "-sycl-host"
// SYCL_HOST: clang{{.*}} "-fsycl-is-host"
// SYCL_HOST-SAME: "-mllvm" "-sycl-host"

// RUN: %clangxx -target x86_64-unknown-linux-gnu -fsycl -flto %s -### 2>&1 \
// RUN:   | FileCheck --check-prefix=SYCL_HOST_LTO %s
// SYCL_HOST_LTO: ld{{.*}} "-plugin-opt=-sycl-host"

// RUN: %clang_cl -fsycl -flto -fuse-ld=lld %s -### 2>&1 \
// RUN:   | FileCheck --check-prefix=SYCL_HOST_LTO_WIN %s
// SYCL_HOST_LTO_WIN: lld-link{{.*}} "-mllvm:-sycl-host"
