; RUN: opt -S -whole-program-assume -intel-libirc-allowed -passes=dtrans-aostosoaop %s 2>&1 | FileCheck %s --check-prefix=CHECK

target triple = "x86_64-unknown-linux-gnu"

; This test verifies that the AOS-to-SOA transformation accepts structures
; that meet all the required safety conditions.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.test01 = type { i32, i64, i32 }
%struct.dep01 = type { i64, ptr }
@glob = internal global %struct.dep01 zeroinitializer

define i32 @main() {
  call void @test01(i64 10)
  ret i32 0
}

define void @test01(i64 %count) {
  %mem = call ptr @calloc(i64 %count, i64 24)
  %cmp = icmp eq ptr %mem, null
  br i1 %cmp, label %done, label %init
init:
  store ptr %mem, ptr getelementptr (%struct.dep01, ptr @glob, i64 0, i32 1)
  br label %loop_top
loop_top:
  %n = phi i64 [0, %init], [%n1, %loop_top]
  %n1 = add i64 %n, 1
  %f0 = getelementptr %struct.test01, ptr %mem, i64 %n, i32 0
  store i32 0, ptr %f0
  %f1 = getelementptr %struct.test01, ptr %mem, i64 %n, i32 1
  store i64 0, ptr %f1
  %f2 = getelementptr %struct.test01, ptr %mem, i64 %n, i32 2
  store i32 0, ptr %f2
  %cmp2 = icmp ne i64 %count, %n1
  br i1 %cmp2, label %loop_top, label %done
done:
  ret void
}

; CHECK: %__SOA_struct.test01 = type { ptr, ptr, ptr }
; CHECK: %__SOADT_struct.dep01 = type { i64, i32 }
; CHECK: @__soa_struct.test01 = internal global %__SOA_struct.test01 zeroinitializer

declare !intel.dtrans.func.type !5 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64) #0

attributes #0 = { allockind("alloc,zeroed") allocsize(0,1) "alloc-family"="malloc" }

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!4 = !{i8 0, i32 1}  ; i8*
!5 = distinct !{!4}
!6 = !{!"S", %struct.test01 zeroinitializer, i32 3, !1, !2, !1} ; { i32, i64, i32 }
!7 = !{!"S", %struct.dep01 zeroinitializer, i32 2, !2, !3} ; { i64, %struct.test01* }

!intel.dtrans.types = !{!6, !7}
