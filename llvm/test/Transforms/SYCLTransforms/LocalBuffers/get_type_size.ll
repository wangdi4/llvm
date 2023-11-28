; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-local-buffers,check-debugify' -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-local-buffers' -S < %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; Check local buffer and local buffer analysis passes uses
;; getTypeAllocSize function instead of getTypeSizeInBits/8
;; Note: current implementation assures max alignment for
;;       each variable in the local buffer, which is 128 bytes.
;;       Thus currently we should assume each variable starts with
;;       offset of 128 bytes
;; Checked types:
;;                i1          --> starts at offset 0
;;                <2 x    i1> --> starts at offset 128
;;                <3 x float> --> starts at offset 256
;;                i32         --> starts at offset 384
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Used local variables
@foo.localBool = internal addrspace(3) global i1 0, align 1
@foo.localBool2 = internal addrspace(3) global <2 x i1> zeroinitializer, align 1
@foo.localFloat3 = internal addrspace(3) global <3 x float> zeroinitializer, align 16
@foo.localInt = internal addrspace(3) global i32 zeroinitializer, align 4

define void @foo(ptr addrspace(1) %pBool, ptr addrspace(1) %pVBool, ptr addrspace(1) %pVFloat, ptr addrspace(1) %pInt) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  br label %BB0
BB0:
  %dummyBool = load i1, ptr addrspace(3) @foo.localBool, align 1
  store i1 %dummyBool, ptr addrspace(1) %pBool
  br label %BB1
BB1:
  %dummyBool2 = load <2 x i1>, ptr addrspace(3) @foo.localBool2, align 1
  store <2 x i1> %dummyBool2, ptr addrspace(1) %pVBool
  br label %BB2
BB2:
  %dummyFloat3 = load <3 x float>, ptr addrspace(3) @foo.localFloat3, align 16
  store <3 x float> %dummyFloat3, ptr addrspace(1) %pVFloat
  br label %BB3
BB3:
  %dummyInt = load i32, ptr addrspace(3) @foo.localInt, align 4
  store i32 %dummyInt, ptr addrspace(1) %pInt
  ret void

; CHECK:        define void @foo
; CHECK-NEXT: entry:
; CHECK-NEXT:   [[VAR0:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK-NEXT:   [[VAR1:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 1
; CHECK-NEXT:   [[VAR2:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 16
; CHECK-NEXT:   [[VAR3:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 32
; CHECK-NEXT:   br label %BB0
; CHECK:      BB0:                                              ; preds = %entry
; CHECK-NEXT:   %dummyBool = load i1, ptr addrspace(3) [[VAR0]], align 1
; CHECK-NEXT:   store i1 %dummyBool, ptr addrspace(1) %pBool
; CHECK-NEXT:   br label %BB1
; CHECK:      BB1:                                              ; preds = %BB0
; CHECK-NEXT:   %dummyBool2 = load <2 x i1>, ptr addrspace(3) [[VAR1]], align 1
; CHECK-NEXT:   store <2 x i1> %dummyBool2, ptr addrspace(1) %pVBool
; CHECK-NEXT:   br label %BB2
; CHECK:      BB2:                                              ; preds = %BB1
; CHECK-NEXT:   %dummyFloat3 = load <3 x float>, ptr addrspace(3) [[VAR2]], align 16
; CHECK-NEXT:   store <3 x float> %dummyFloat3, ptr addrspace(1) %pVFloat
; CHECK-NEXT:   br label %BB3
; CHECK:      BB3:                                              ; preds = %BB2
; CHECK-NEXT:   %dummyInt = load i32, ptr addrspace(3) [[VAR3]], align 4
; CHECK-NEXT:   store i32 %dummyInt, ptr addrspace(1) %pInt
; CHECK-NEXT:   ret void

}

!sycl.kernels = !{!0}

!0 = !{ptr @foo}
!1 = !{!"bool*", !"bool2*", !"float3*", !"int*"}
!2 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
