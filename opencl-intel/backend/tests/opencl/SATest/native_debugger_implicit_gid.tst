;;; Test implict gid variables inserted for GDB (native debugger):
;;; __ocl_dbg_gid0, __ocl_dbg_gid1, __ocl_dbg_gid2

; RUN: SATest -BUILD --vectorizer-type=vpo -cpuarch=skx -config=%s.cfg --dump-llvm-file - | FileCheck %s

; Test kernel
; __kernel void main_kernel(__global int* buf_in, __global int* buf_out) {
;     int lid = get_local_id(0);
;     barrier(CLK_LOCAL_MEM_FENCE);
; }

; alloca for pointers to gids
; CHECK-LABEL: wrapper_entry:
; CHECK: [[GID0_ADDR:%__ocl_dbg_gid0.*]] = alloca i64, align 8
; CHECK: [[GID1_ADDR:%__ocl_dbg_gid1.*]] = alloca i64, align 8
; CHECK: [[GID2_ADDR:%__ocl_dbg_gid2.*]] = alloca i64, align 8

; store gids
; CHECK: store volatile i64 %GlobalID{{.*}}, {{.*}} [[GID0_ADDR]]
; CHECK: store volatile i64 %GlobalID{{.*}}, {{.*}} [[GID1_ADDR]]
; CHECK: store volatile i64 %GlobalID{{.*}}, {{.*}} [[GID2_ADDR]]
