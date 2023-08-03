; RUN: opt -passes="instcombine" -instcombine-preserve-for-dtrans=false < %s -S 2>&1 | FileCheck %s --check-prefix=CHECK-FALSE
; RUN: opt -passes="instcombine" -instcombine-preserve-for-dtrans=true < %s -S 2>&1 | FileCheck %s --check-prefix=CHECK-TRUE

; Check that with -instcombine-preserve-for-dtrans=true, the store does not
; become an i8* reference, but with -instcombine-preserve-for-dtrans=false, it
; does.

%struct.x264_t = type { i64, [129 x ptr] }

declare dso_local noalias ptr @malloc(i64)

define dso_local void @foo() local_unnamed_addr {
  %t5 = tail call fastcc ptr @malloc(i64 33344)
  %t0 = getelementptr inbounds %struct.x264_t, ptr %t5, i64 0, i32 1
  store ptr %t5, ptr %t0, align 16
  ret void
}
; CHECK-TRUE: store ptr
; CHECK-FALSE: store ptr

