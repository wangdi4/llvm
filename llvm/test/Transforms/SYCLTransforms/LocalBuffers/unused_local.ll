; RUN: opt -passes='sycl-kernel-add-implicit-args,debugify,sycl-kernel-local-buffers,check-debugify' -S < %s -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes='sycl-kernel-add-implicit-args,sycl-kernel-local-buffers' -S < %s | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32-S32"

; not mapping unused local variables to local buffer

; Used local variables
@foo.localInt = internal addrspace(3) global i32 0, align 4
@bar.localLong16 = internal addrspace(3) global <16 x i64> zeroinitializer, align 128

; Unused local variables
@foo.localChar = internal addrspace(3) global i8 0, align 1
@foo.localFloat = internal addrspace(3) global float 0.000000e+00, align 4
@bar.localInt4 = internal addrspace(3) global <4 x i32> zeroinitializer, align 16


define void @foo(ptr addrspace(1) %pInt, ptr addrspace(1) %pChar, ptr addrspace(1) %pFloat) !kernel_arg_base_type !1 !arg_type_null_val !2 {
entry:
  %dummyInt = load i32, ptr addrspace(3) @foo.localInt, align 4
  store i32 %dummyInt, ptr addrspace(1) %pInt

  ret void
}

define void @bar(ptr addrspace(1) %pInt4, ptr addrspace(1) %pLong16) !kernel_arg_base_type !3 !arg_type_null_val !4 {
entry:
  %dummyLong16 = load <16 x i64>, ptr addrspace(3) @bar.localLong16, align 128
  store <16 x i64> %dummyLong16, ptr addrspace(1) %pLong16

  ret void
}

!sycl.kernels = !{!0}

!0 = !{ptr @foo, ptr @bar}
!1 = !{!"int*", !"char*", !"float*"}
!2 = !{ptr addrspace(1) null, ptr addrspace(1) null, ptr addrspace(1) null}
!3 = !{!"int4*", !"long16*"}
!4 = !{ptr addrspace(1) null, ptr addrspace(1) null}

; CHECK:           define void @foo(ptr addrspace(1) %pInt, ptr addrspace(1) %pChar, ptr addrspace(1) %pFloat,
; CHECK-NEXT:   entry:
; CHECK-NEXT:   [[VAR0:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK-NEXT:   %dummyInt = load i32, ptr addrspace(3) [[VAR0]], align 4


; CHECK:           define void @bar(ptr addrspace(1) %pInt4, ptr addrspace(1) %pLong16,
; CHECK-NEXT:   entry:
; CHECK-NEXT:   [[VAR1:%[a-zA-Z0-9]+]] = getelementptr i8, ptr addrspace(3) %pLocalMemBase, i32 0
; CHECK-NEXT:   %dummyLong16 = load <16 x i64>, ptr addrspace(3) [[VAR1]], align 128

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: CheckModuleDebugify: PASS
