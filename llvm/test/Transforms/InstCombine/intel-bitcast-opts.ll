; RUN: opt -instcombine -instcombine-preserve-for-dtrans=false < %s -S 2>&1 | FileCheck %s --check-prefix=CHECK-FALSE
; RUN: opt -instcombine -instcombine-preserve-for-dtrans=true < %s -S 2>&1 | FileCheck %s --check-prefix=CHECK-TRUE

; Check that with -instcombine-preserve-for-dtrans=true, the store does not
; become an i8* reference, but with -instcombine-preserve-for-dtrans=false, it
; does.

%struct.x264_t = type { i64, [129 x %struct.x264_t*] }

declare dso_local noalias i8* @malloc(i64)

define dso_local void @foo() local_unnamed_addr {
  %t5 = tail call fastcc i8* @malloc(i64 33344)
  %t6 = bitcast i8* %t5 to %struct.x264_t*
  %t0 = getelementptr inbounds %struct.x264_t, %struct.x264_t* %t6, i64 0, i32 1
  %t1 = getelementptr inbounds [129 x %struct.x264_t*], [129 x %struct.x264_t*]* %t0, i64 0, i64 0
  store %struct.x264_t* %t6, %struct.x264_t** %t1, align 16
  ret void
}
; CHECK-TRUE: store %struct.x264_t*
; CHECK-FALSE: store i8*

