; RUN: opt -passes='require<sycl-kernel-builtin-info-analysis>,print<sycl-kernel-work-item-analysis>' %s -disable-output 2>&1 | FileCheck %s

target datalayout = "e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-f32:32:32-f64:64:64-f80:128:128-v64:64:64-v128:128:128-a0:0:64-f80:32:32-n8:16:32"
target triple = "i686-pc-win32"

; CHECK: WorkItemAnalysis for function simpleBranch:
; CHECK-NEXT: SEQ   %id = tail call i32 @_Z13get_global_idj(i32 0) #0
; CHECK-NEXT: PTR   %in1 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %id
; CHECK-NEXT: RND   %in2 = load i32, ptr addrspace(1) %in1, align 4
; CHECK-NEXT: RND   %cmp = icmp slt i32 %id, %in2
; CHECK-NEXT: RND   br i1 %cmp, label %true, label %false
; CHECK-NEXT: UNI   %x0 = add i32 %un1, 1
; CHECK-NEXT: UNI   %cmp1 = icmp slt i32 %x0, %un2
; CHECK-NEXT: UNI   br i1 %cmp1, label %next, label %next1
; CHECK-NEXT: UNI   %x2 = sub i32 %un1, 1
; CHECK-NEXT: UNI   br label %next1
; CHECK-NEXT: UNI   %x3 = phi i32 [ %x0, %true ], [ %x2, %next ]
; CHECK-NEXT: UNI   br label %end
; CHECK-NEXT: UNI   %x1 = add i32 %un2, 1
; CHECK-NEXT: UNI   br label %end
; CHECK-NEXT: RND   %x = phi i32 [ %x3, %next1 ], [ %x1, %false ]
; CHECK-NEXT: PTR   %out1 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %id
; CHECK-NEXT: RND   store i32 %x, ptr addrspace(1) %out1, align 4
; CHECK-NEXT: UNI   ret void

define void @simpleBranch(ptr addrspace(1) nocapture %in, ptr addrspace(1) nocapture %out, i32 %un1, i32 %un2) nounwind !kernel_arg_base_type !0 !arg_type_null_val !1 {
  %id = tail call i32 @_Z13get_global_idj(i32 0) nounwind

  %in1 = getelementptr inbounds i32, ptr addrspace(1) %in, i32 %id
  %in2 = load i32, ptr addrspace(1) %in1, align 4

  %cmp = icmp slt i32 %id, %in2 
  br i1 %cmp, label %true, label %false

true:
  %x0 = add i32 %un1, 1
  %cmp1 = icmp slt i32 %x0, %un2 
  br i1 %cmp1, label %next, label %next1
next:
  %x2 = sub  i32 %un1, 1
  br label %next1
next1:
%x3 = phi i32 [ %x0, %true ], [ %x2, %next ]
  br label %end
false:
  %x1 = add i32 %un2, 1
  br label %end
end:
  %x = phi i32 [ %x3, %next1 ], [ %x1, %false ]
  %out1 = getelementptr inbounds i32, ptr addrspace(1) %out, i32 %id
  store i32 %x, ptr addrspace(1) %out1, align 4
  ret void
}

declare i32 @_Z13get_global_idj(i32)

!0 = !{!"int*", !"int*", !"int", !"int"}
!1 = !{ptr addrspace(1) null, ptr addrspace(1) null, i32 0, i32 0}
