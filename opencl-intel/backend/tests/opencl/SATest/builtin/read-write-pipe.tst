; RUN: SATest -BUILD --config=%s.cfg -tsize=0 --dump-llvm-file - | FileCheck %s

; This test checks that there is no bitcast of __ocl_wpipe2ptr in
; __store_write_pipe_use and no bitcast of __ocl_rpipe2ptr in
; __store_read_pipe_use.

CHECK-LABEL: __store_write_pipe_use
CHECK: call {{.*}} addrspace(1){{.*}} @__ocl_wpipe2ptr(

CHECK-LABEL: __store_read_pipe_use
CHECK: call {{.*}} addrspace(1){{.*}} @__ocl_rpipe2ptr(
