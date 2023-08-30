; RUN: opt -passes='sycl-kernel-local-buffers' -S %s -enable-debugify -disable-output 2>&1 | FileCheck %s -check-prefix=DEBUGIFY
; RUN: opt -passes='sycl-kernel-local-buffers' -S %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; not mapping used non-local variables to local buffer

; Used non-local variables
@foo.localInt = internal addrspace(1) global i32 0, align 4
@bar.localLong16 = internal addrspace(1) global <16 x i64> zeroinitializer, align 128

define void @foo(ptr addrspace(1) %pInt, ptr addrspace(1) %pChar, ptr addrspace(1) %pFloat, ptr addrspace(3) %pLocalMem, ptr %pWorkDim, ptr %pWGId, ptr %pBaseGlbId, ptr %pLocalIds, ptr %contextpointer, i32 %iterCount, ptr %pSpecialBuf, ptr %pCurrWI, ptr %extExecContextPointer) !kernel_arg_base_type !0 !arg_type_null_val !1 {
entry:
  %dummyInt = load i32, ptr addrspace(1) @foo.localInt, align 4
  store i32 %dummyInt, ptr addrspace(1) %pInt

  ret void
}

define void @bar(ptr addrspace(1) %pInt4, ptr addrspace(1) %pLong16, ptr addrspace(3) %pLocalMem, ptr %pWorkDim, ptr %pWGId, ptr %pBaseGlbId, ptr %pLocalIds, ptr %contextpointer, i32 %iterCount, ptr %pSpecialBuf, ptr %pCurrWI, ptr %extExecContextPointer) !kernel_arg_base_type !2 !arg_type_null_val !3 {
entry:
  %dummyLong16 = load <16 x i64>, ptr addrspace(1) @bar.localLong16, align 128
  store <16 x i64> %dummyLong16, ptr addrspace(1) %pLong16

  ret void
}

!0 = !{!"int*", !"char*", !"float*"}
!1 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null}
!2 = !{!"int4*", !"long16*"}
!3 = !{ptr addrspace(1) null, ptr addrspace(1) null}

; CHECK:           define void @foo(ptr addrspace(1) %pInt, ptr addrspace(1) %pChar, ptr addrspace(1) %pFloat,
; CEHCK:   entry:
; CHECK:   %dummyInt = load i32, ptr addrspace(1) @foo.localInt, align 4
; CHECK-NEXT:   store i32 %dummyInt, ptr addrspace(1) %pInt
; CHECK-NEXT:   ret void

; CHECK:           define void @bar(ptr addrspace(1) %pInt4, ptr addrspace(1) %pLong16,
; CHECK-NEXT:   entry:
; CHECK-NEXT:   %dummyLong16 = load <16 x i64>, ptr addrspace(1) @bar.localLong16, align 128
; CHECK-NEXT:   store <16 x i64> %dummyLong16, ptr addrspace(1) %pLong16
; CHECK-NEXT:   ret void

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
