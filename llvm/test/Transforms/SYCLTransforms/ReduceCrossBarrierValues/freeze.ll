; RUN: opt -S -passes=sycl-kernel-reduce-cross-barrier-values %s | FileCheck %s
; RUN: opt -S -passes=sycl-kernel-reduce-cross-barrier-values %s -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s

; Checks that the freeze inst is also copied.

define void @test_freeze(ptr %dst) !kernel_arg_base_type !0 !arg_type_null_val !1 {
; CHECK-LABEL: define void @test_freeze
; CHECK-NEXT: entry:
; CHECK-NEXT:   br label %SyncBB
; CHECK-EMPTY:
; CHECK-NEXT: SyncBB:
; CHECK-NEXT:   call void @_Z7barrierj(i32 0)
; CHECK-NEXT:   %id.copy = call i64 @_Z13get_global_idj(i32 0)
; CHECK-NEXT:   %id.fr.copy = freeze i64 %id.copy
; CHECK-NEXT:   %add.copy = add i64 %id.fr.copy, 1
; CHECK-NEXT:   %mul.copy = mul i64 %add.copy, 10
; CHECK-NEXT:   %rem.copy = urem i64 %add.copy, 3
; CHECK-NEXT:   %gep0 = getelementptr i8, ptr %dst, i64 %mul.copy
; CHECK-NEXT:   %gep1 = getelementptr i8, ptr %dst, i64 %rem.copy
; CHECK-NEXT:   store i8 1, ptr %gep0, align 1
; CHECK-NEXT:   store i8 2, ptr %gep1, align 1
; CHECK-NEXT:   ret void
entry:
  %id = call i64 @_Z13get_global_idj(i32 0)
  %id.fr = freeze i64 %id
  %add = add i64 %id.fr, 1
  %mul = mul i64 %add, 10
  %rem = urem i64 %add, 3
  br label %SyncBB

SyncBB:
  call void @_Z7barrierj(i32 0)
  %gep0 = getelementptr i8, ptr %dst, i64 %mul
  %gep1 = getelementptr i8, ptr %dst, i64 %rem
  store i8 1, ptr %gep0
  store i8 2, ptr %gep1
  ret void
}

declare i64 @_Z13get_global_idj(i32) #0
declare void @_Z7barrierj(i32)

attributes #0 = { convergent nounwind readnone willreturn }

!0 = !{!"char*"}
!1 = !{ptr null}

; DEBUGIFY-NOT: WARNING
; DEBUGIFY: WARNING: Missing line 1
; DEBUGIFY: WARNING: Missing line 2
; DEBUGIFY: WARNING: Missing line 3
; DEBUGIFY: WARNING: Missing line 4
; DEBUGIFY: WARNING: Missing line 5
; DEBUGIFY-NOT: WARNING
