;;; Test implict gid variables inserted in subgroup emulation for GDB (native debugger):
;;; __ocl_dbg_gid0, __ocl_dbg_gid1, __ocl_dbg_gid2

; RUN: SATest -BUILD --vectorizer-type=vpo -cpuarch=skx -config=%s.cfg --dump-llvm-file - | FileCheck %s

; Test kernel
; __kernel void main_kernel(__global int* buf_in, __global int* buf_out) {
;     int lid = get_local_id(0);
;     buf_out[lid] = sub_group_scan_inclusive_add(buf_in[lid]);
; }

; alloca for pointers to gids
; CHECK-LABEL: wrapper_entry:
; CHECK: [[GID0_VEC_PTR_ADDR:%w.__ocl_dbg_gid0.addr]] = alloca

; check gid0 in subgroup emulation loop
; CHECK: [[GID0_VEC_PTR:%.*]] = load {{.*}} [[GID0_VEC_PTR_ADDR]], align 8
; CHECK: sg.loop.header{{.*}}:
; calculate actual gid0 for emulated workitem
; CHECK: [[GID:%GlobalID.*]] = add i64 %LocalId{{.*}}, %BaseGlobalID
; CHECK-NEXT: [[SGLID_EXT:%.*]] = zext i32 %sg.lid{{.*}} to i64
; CHECK-NEXT: [[GID0:%.*]] =  add i64 [[SGLID_EXT]], [[GID]]

; store volatile gid0
; CHECK-NEXT: [[GID0_PTR:%.*]] = getelementptr {{.*}} [[GID0_VEC_PTR]], i32 0, i32 %sg.lid
; CHECK-NEXT: store volatile i64 [[GID0]], {{.*}} [[GID0_PTR]], align 8
