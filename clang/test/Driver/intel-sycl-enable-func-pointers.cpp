/// test for -fsycl-enable-function-pointers
// RUN: %clang -c -target x86_64-unknown-linux-gnu -fsycl -fsycl-enable-function-pointers %s -### 2>&1 \
// RUN:  | FileCheck -check-prefix ENABLE_FUNC_POINTERS %s
// RUN: %clang_cl -c --target=x86_64-unknown-linux-gnu -fsycl -fsycl-enable-function-pointers %s -### 2>&1 \
// RUN:  | FileCheck -check-prefix ENABLE_FUNC_POINTERS %s
// ENABLE_FUNC_POINTERS: "-fsycl-allow-func-ptr"
// ENABLE_FUNC_POINTERS: "-fenable-variant-function-pointers"
// ENABLE_FUNC_POINTERS: "-fenable-variant-virtual-calls"
// ENABLE_FUNC_POINTERS: "-triple" "x86_64-unknown-linux-gnu"
// ENABLE_FUNC_POINTERS: "-fenable-variant-function-pointers"
// ENABLE_FUNC_POINTERS: "-fenable-variant-virtual-calls"
