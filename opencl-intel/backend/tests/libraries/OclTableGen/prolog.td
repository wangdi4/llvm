// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=shared | FileCheck %s --check-prefix=CHECK-SHARED
// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=sse | FileCheck %s --check-prefix=CHECK-TARGET
// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=avx | FileCheck %s --check-prefix=CHECK-TARGET
// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=avx2 | FileCheck %s --check-prefix=CHECK-TARGET
// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=avx512 | FileCheck %s --check-prefix=CHECK-TARGET
// RUN: %ocltblgen %s --gen-ocl-bi --handle-impl-target-inheritance --emit-target-name=amx | FileCheck %s --check-prefix=CHECK-TARGET

include "GENERIC/opencl-builtin.td"
include "source/core/targets.td"

// CHECK-SHARED: #include "svml_shared.h"

// CHECK-TARGET:      #ifdef __x86_64__
// CHECK-TARGET-NEXT:   #include "svml_ia32e.h"
// CHECK-TARGET-NEXT: #else
// CHECK-TARGET-NEXT:   #include "svml_ia32.h"
// CHECK-TARGET-NEXT: #endif
