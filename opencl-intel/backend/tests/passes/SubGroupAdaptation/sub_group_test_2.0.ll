; RUN: %oclopt -sub-group-adaptation -S < %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: %oclopt -sub-group-adaptation -verify -S < %s | FileCheck %s
;;*****************************************************************************
;; This test checks the SubGroupAdaptation pass
;; Four cases:
;;    get_sub_group_size
;;    get_max_sub_group_size
;;    get_num_sub_groups
;;    get_enqueued_num_sub_groups
;;*****************************************************************************
; ModuleID = 'Program'
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; CHECK: @sw_test
; CHECK: entry
; CHECK: call spir_func i32 @_Z18get_sub_group_sizev()
; CHECK: call spir_func i32 @_Z22get_max_sub_group_sizev()
; CHECK-NOT: call spir_func i32 @_Z18get_num_sub_groupsv()
; CHECK-NOT: call spir_func i32 @_Z27get_enqueued_num_sub_groupsv()

; Function Attrs: nounwind
define spir_kernel void @sw_test(i32 %res) #0 {
entry:
  %res.addr = alloca i32, align 4
  %sgsize = alloca i32, align 4
  %msgsize = alloca i32, align 4
  %nsg = alloca i32, align 4
  %ensg = alloca i32, align 4
  store i32 %res, i32* %res.addr, align 4
  %call = call spir_func i32 @_Z18get_sub_group_sizev()
  store i32 %call, i32* %sgsize, align 4
  %call1 = call spir_func i32 @_Z22get_max_sub_group_sizev()
  store i32 %call1, i32* %msgsize, align 4
  %0 = load i32, i32* %sgsize, align 4
  %1 = load i32, i32* %msgsize, align 4
  %cmp = icmp eq i32 %0, %1
  br i1 %cmp, label %if.then, label %if.end6

if.then:                                          ; preds = %entry
  %call2 = call spir_func i32 @_Z18get_num_sub_groupsv()
  store i32 %call2, i32* %nsg, align 4
  %call3 = call spir_func i32 @_Z27get_enqueued_num_sub_groupsv()
  store i32 %call3, i32* %ensg, align 4
  %2 = load i32, i32* %nsg, align 4
  %3 = load i32, i32* %ensg, align 4
  %cmp4 = icmp eq i32 %2, %3
  br i1 %cmp4, label %if.then5, label %if.end

if.then5:                                         ; preds = %if.then
  %4 = load i32, i32* %nsg, align 4
  store i32 %4, i32* %res.addr, align 4
  br label %if.end

if.end:                                           ; preds = %if.then5, %if.then
  br label %if.end6

if.end6:                                          ; preds = %if.end, %entry
  ret void
}

; CHECK: define spir_func i32 @_Z18get_sub_group_sizev()
; CHECK: entry
; CHECK: %lsz0 = call spir_func i64 @_Z14get_local_sizej(i32 0)
; CHECK: %lsz1 = call spir_func i64 @_Z14get_local_sizej(i32 1)
; CHECK: %lsz2 = call spir_func i64 @_Z14get_local_sizej(i32 2)
; CHECK: %op0 = mul i64 %lsz0, %lsz1
; CHECK: %res = mul i64 %op0, %lsz2
; CHECK: %cast = trunc i64 %res to i32
; CHECK: ret i32 %cast

; CHECK: define spir_func i32 @_Z22get_max_sub_group_sizev()
; CHECK: entry
; CHECK: %elsz0 = call spir_func i64 @_Z23get_enqueued_local_sizej(i32 0)
; CHECK: %elsz1 = call spir_func i64 @_Z23get_enqueued_local_sizej(i32 1)
; CHECK: %elsz2 = call spir_func i64 @_Z23get_enqueued_local_sizej(i32 2)
; CHECK: %op0 = mul i64 %elsz0, %elsz1
; CHECK: %res = mul i64 %op0, %elsz2
; CHECK: %cast = trunc i64 %res to i32
; CHECK: ret i32 %cast

declare spir_func i32 @_Z18get_sub_group_sizev() #1

declare spir_func i32 @_Z22get_max_sub_group_sizev() #1

declare spir_func i32 @_Z18get_num_sub_groupsv() #1

declare spir_func i32 @_Z27get_enqueued_num_sub_groupsv() #1

; CHECK-NOT: declare spir_func i32 @_Z18get_sub_group_sizev()
; CHECK-NOT: declare spir_func i32 @_Z22get_max_sub_group_sizev()
; CHECK-NOT: declare spir_func i32 @_Z18get_num_sub_groupsv()
; CHECK-NOT: declare spir_func i32 @_Z27get_enqueued_num_sub_groupsv() 
; CHECK: declare spir_func i64 @_Z14get_local_sizej(i32)
; CHECK: declare spir_func i64 @_Z23get_enqueued_local_sizej(i32)

attributes #0 = { nounwind "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-realign-stack" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!sycl.kernels = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.spir.version = !{!6}
!opencl.ocl.version = !{!7}
!opencl.used.extensions = !{!8}
!opencl.used.optional.core.features = !{!8}
!opencl.compiler.options = !{!8}

!0 = !{void (i32)* @sw_test, !1, !2, !3, !4, !5}
!1 = !{!"kernel_arg_addr_space", i32 0}
!2 = !{!"kernel_arg_access_qual", !"none"}
!3 = !{!"kernel_arg_type", !"uint"}
!4 = !{!"kernel_arg_base_type", !"uint"}
!5 = !{!"kernel_arg_type_qual", !""}
!6 = !{i32 1, i32 2}
!7 = !{i32 2, i32 0}
!8 = !{}

;;; --- OpenCL source (compilation options: "-cl-std=CL2.0 -D__OPENCL_C_VERSION__=200"
;;;__attribute__((overloadable)) uint get_sub_group_size();
;;;__attribute__((overloadable)) uint get_max_sub_group_size();
;;;__attribute__((overloadable)) uint get_num_sub_groups();
;;;__attribute__((overloadable)) uint get_enqueued_num_sub_groups();
;;;
;;;__kernel void sw_test(uint res) {
;;;  uint sgsize = get_sub_group_size();
;;;  uint msgsize = get_max_sub_group_size();
;;;  if (sgsize == msgsize) {
;;;	uint nsg = get_num_sub_groups();
;;;	uint ensg = get_enqueued_num_sub_groups();
;;;		if (nsg == ensg)
;;;			res = nsg;
;;;	}
;;;}

; Instructions in these two functions are added to replace the originl instructions. No DebugLoc for them.
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _Z18get_sub_group_sizev
; DEBUGIFY: WARNING: Instruction with empty DebugLoc in function _Z22get_max_sub_group_sizev
; DEBUGIFY: WARNING: Missing line 15
; DEBUGIFY: WARNING: Missing line 17
; DEBUGIFY-NOT: WARNING
