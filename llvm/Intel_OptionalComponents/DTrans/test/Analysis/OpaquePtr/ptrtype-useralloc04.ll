; REQUIRES: asserts
; RUN: opt -disable-output -debug-only=dtrans-alloc-collector -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test recognition of user function that wraps the allocation call
; for cases where the actual memory address of the allocated memory
; is embedded as metadata into the allocation, and an offset pointer
; is returned.

; CHECK: Identified as user allocation function: x264_malloc
; CHECK: Identified as user free function: x264_free

%struct.test = type { i64, i64 }
@.str.288 = private unnamed_addr constant [6 x i8] c"Hello\0A"

; Address returned is offset to allow storing some metadata into
; allocated memory.
define internal "intel_dtrans_func_index"="1" ptr @x264_malloc(i32 %arg) !intel.dtrans.func.type !2 {
bb:
  %i = add nsw i32 %arg, 15
  %i1 = sext i32 %i to i64
  %i2 = add nsw i64 12, %i1
  %i3 = tail call noalias ptr @malloc(i64 %i2)
  %i4 = icmp eq ptr %i3, null
  br i1 %i4, label %bb15, label %bb5

bb5:                                              ; preds = %bb
  %i6 = getelementptr inbounds i8, ptr %i3, i64 27
  %i7 = ptrtoint ptr %i6 to i64
  %i8 = and i64 %i7, 15
  %i9 = sub nsw i64 0, %i8
  %i10 = getelementptr inbounds i8, ptr %i6, i64 %i9
  %i11 = getelementptr inbounds i8, ptr %i10, i64 -8
  %i12 = bitcast ptr %i11 to ptr
  store ptr %i3, ptr %i12, align 8
  %i13 = getelementptr inbounds i8, ptr %i11, i64 -4
  %i14 = bitcast ptr %i13 to ptr
  store i32 %arg, ptr %i14, align 4
  br label %bb16

bb15:                                             ; preds = %bb
  %call = call i32 (ptr, ...) @printf(ptr getelementptr inbounds ([6 x i8], ptr @.str.288, i32 0, i32 0))
  br label %bb16

bb16:                                             ; preds = %bb15, %bb5
  %i17 = phi ptr [ %i10, %bb5 ], [ null, %bb15 ]
  ret ptr %i17
}

; Raw address to free is embedded prior to the address passed in.
define internal void @x264_free(ptr readonly "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !6 {
  %isnull = icmp eq ptr %in, null
  br i1 %isnull, label %done, label %do_free

do_free:
  %addr = bitcast ptr %in to ptr
  %offset = getelementptr inbounds ptr, ptr %addr, i64 -1
  %metaptr = load ptr, ptr %offset
  tail call void @free(ptr %metaptr)
  br label %done

done:
  ret void
}


define i64 @test() {
bb:
  %p = call ptr @x264_malloc(i32 16)
  %ps = bitcast ptr %p to ptr
  %f0 = getelementptr %struct.test, ptr %ps, i64 0, i32 0
  %f1 = getelementptr %struct.test, ptr %ps, i64 0, i32 1
  %v = load i64, ptr %f1, align 4
  store i64 0, ptr %f0, align 4
  store i64 1, ptr %f1, align 4
  call void @x264_free(ptr %p)
  ret i64 %v
}

declare !intel.dtrans.func.type !4 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0
declare !intel.dtrans.func.type !5 i32 @printf(ptr nocapture readonly "intel_dtrans_func_index"="1", ...)
declare !intel.dtrans.func.type !7 void @free(ptr "intel_dtrans_func_index"="1") #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { allockind("free") "alloc-family"="malloc" }

!intel.dtrans.types = !{!0}

!0 = !{!"S", %struct.test zeroinitializer, i32 2, !1, !1}
!1 = !{i64 0, i32 0}
!2 = distinct !{!3}
!3 = !{i8 0, i32 1}
!4 = distinct !{!3}
!5 = distinct !{!3}
!6 = distinct !{!3}
!7 = distinct !{!3}
