; This test try to find the ptr and ptrofptr's global variables and validate isValidPtrOfPtr function.
; REQUIRES: asserts
; RUN: opt -opaque-pointers -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -whole-program-assume < %s -passes=dtrans-reusefieldop -debug-only=dtrans-reusefieldop -disable-output 2>&1 | FileCheck %s

; CHECK:Reused structure: %struct.test
; CHECK-NEXT:    Field mapping are (From:To): { 3:3 4:3 }
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
; CHECK-NEXT:New GEP (after):
; CHECK-NEXT:  %0 = getelementptr %struct.test, ptr %tp, i64 0, i32 3
; CHECK-NEXT:Load:
; CHECK-NEXT:  %b = load i64, ptr %0, align 8
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
; CHECK-NEXT:New GEP (after):
; CHECK-NEXT:  %1 = getelementptr %struct.test, ptr %tp, i64 0, i32 3
; CHECK-NEXT:Load:
; CHECK-NEXT:  %d = load i64, ptr %1, align 8
; CHECK-NEXT:GEP (before):
; CHECK-NEXT:  %g = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
; CHECK-NEXT:New GEP (after):
; CHECK-NEXT:  %0 = getelementptr %struct.test, ptr %tp, i64 0, i32 3
; CHECK-NEXT:Load:
; CHECK-NEXT:  %b = load i64, ptr %0, align 8

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test = type { i32, i64, i32, i64, i64 }
%struct.ptr = type { i32, i64, i32, ptr, ptr }
%struct.ptr2ptr = type { i32, ptr }

@__intel_dtrans_aostosoa_index = private constant [41 x i8] c"{dtrans} AOS-to-SOA peeling index {id:0}\00"
@0 = private constant [1 x i8] zeroinitializer

@net = internal global %struct.ptr zeroinitializer, align 8
@node = internal global %struct.ptr2ptr zeroinitializer, align 8

define void @foo_0(%struct.test* "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !6 {
entry:
  %i = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 0
  %0 = load i32, ptr %i, align 8
  %add = add nsw i32 %0, 20
  %conv = sext i32 %add to i64
  %f = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 3
  store i64 %conv, ptr %f, align 8
  %o = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
  store i64 %conv, ptr %o, align 8
  ret void
}

define i64 @cal_0(%struct.test* "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !6 {
entry:
  %f = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 3
  %a = load i64, ptr %f, align 8
  %g = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
  %b = load i64, ptr %g, align 8
  %h = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 1
  %d = load i64, ptr %g, align 8
  %e = add i64 %a, %b
  %ret = add i64 %d, %e
  ret i64 %ret
}

define void @init_net_and_node(%struct.ptr* "intel_dtrans_func_index"="1" %net0, i64 %idx) !intel.dtrans.func.type !15 {
entry:
  %ptr = load %struct.test*, ptr getelementptr inbounds (%struct.ptr, ptr @net, i64 0, i32 3), align 8
  %ptridx = getelementptr inbounds %struct.test, ptr %ptr, i64 %idx
  %basic = getelementptr %struct.ptr2ptr, %struct.ptr2ptr* @node, i64 0, i32 1
  %ptrofptr = load %struct.test**, ptr %basic, align 8
  %ptrofptridx = getelementptr %struct.test*, ptr %ptrofptr, i64 %idx
  store %struct.test* %ptridx, ptr %ptrofptridx, align 8
  %f = getelementptr inbounds %struct.test, ptr %ptridx, i64 0, i32 3
  store i64 %idx, ptr %f, align 8

  ret void
}

define void @ptrofptr_0(%struct.test** "intel_dtrans_func_index"="1" %stv, i64 %idx) !intel.dtrans.func.type !16 {
entry:
  %ptrofptr = getelementptr %struct.ptr2ptr, ptr @node, i64 0, i32 1
  %ld0 = load %struct.test**, ptr %ptrofptr, align 8
  %ptr = getelementptr %struct.test*, ptr %ld0, i64 %idx
  %ld1 = load %struct.test*, ptr %ptr, align 8
  %field = getelementptr inbounds %struct.test, ptr %ld1, i64 0, i32 0
  store i32 100, i32* %field, align 8
  ret void
}

define void @ptrofptr_1(%struct.test** "intel_dtrans_func_index"="1" %stv, %struct.test* noundef "intel_dtrans_func_index"="2" %p, i64 %idx) !intel.dtrans.func.type !17 {
entry:
  br label %loop
loop:
  %phi = phi %struct.test* [ %ld1, %loop ], [ %p, %entry ]
  %ptrofptr = getelementptr %struct.ptr2ptr, %struct.ptr2ptr* @node, i64 0, i32 1
  %ld0 = load %struct.test**, ptr %ptrofptr, align 8
  %ptr = getelementptr %struct.test*, ptr %ld0, i64 %idx
  %ld1 = load %struct.test*, ptr %ptr, align 8
  store %struct.test* %phi, ptr %ptr, align 8
  %p1 = ptrtoint %struct.test* %ld1 to i64
  %p2 = ptrtoint %struct.test* %p to i64
  %sub = sub i64 %p2, %p1
  %div = sdiv exact i64 %sub, 40
  %gep = getelementptr inbounds %struct.test, ptr %p, i64 %div
  store %struct.test* %gep, ptr %ptr, align 8
  %field = getelementptr inbounds %struct.test, ptr %ld1, i64 0, i32 0
  store i32 100, ptr %field, align 8
  %icmp = icmp eq %struct.test* %ld1, null
  br i1 %icmp, label %loop, label %exit
exit:
  ret void
}

define i64 @cal_1(%struct.test* "intel_dtrans_func_index"="1" %tp) !intel.dtrans.func.type !6 {
entry:
  %f = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 1
  %a = load i64, ptr %f, align 8
  %g = getelementptr inbounds %struct.test, ptr %tp, i64 0, i32 4
  %b = load i64, ptr %g, align 8
  %c = sub i64 %a, %b
  ret i64 %c
}

define i64 @main() {
entry:
  %call = tail call noalias i8* @calloc(i64 10, i64 40)
  store i32 10, ptr %call, align 8
  tail call void @foo_0(ptr %call)
  %res_0 = tail call i64 @cal_0(ptr %call)
  tail call void @init_net_and_node(%struct.ptr* @net, i64 101)
  %ptrofptr = tail call noalias i8* @calloc(i64 10, i64 8)
  %0 = bitcast i8* %ptrofptr to %struct.test**
  tail call void @ptrofptr_0(%struct.test** %0, i64 101)
  tail call void @ptrofptr_1(%struct.test** %0, ptr %call, i64 101)
  %res_1 = tail call i64 @cal_1(ptr %call)
  %res = add i64 %res_0, %res_1
  ret i64 %res
}

; Function Attrs: nounwind
declare !intel.dtrans.func.type !9 "intel_dtrans_func_index"="1" i8* @calloc(i64, i64)

!1 = !{i32 0, i32 0}  ; i32
!2 = !{i64 0, i32 0}  ; i64
!3 = !{i16 0, i32 0}  ; i16
!4 = !{i64 0, i32 1}  ; i64*
!5 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!6 = distinct !{!5}
!7 = !{%struct.test zeroinitializer, i32 2}  ; %struct.test**
!8 = !{i8 0, i32 1}  ; i8*
!9 = distinct !{!8}
!10 = distinct !{!8}
!11 = !{!"S", %struct.test zeroinitializer, i32 5, !1, !2, !1, !2, !2} ; { i32, i64, i32, i64, i64 }
!12 = !{!"S", %struct.ptr zeroinitializer, i32 5, !1, !2, !1, !5, !5} ; { i32, i64, i32, %struct.test*, %struct.test* }
!13 = !{!"S", %struct.ptr2ptr zeroinitializer, i32 2, !1, !7} ; { i32, %struct.test** }
!14 = !{%struct.ptr zeroinitializer, i32 1}  ; %struct.test*
!15 = distinct !{!14}
!16 = distinct !{!7}
!17 = distinct !{!7, !5}
!intel.dtrans.types = !{!11, !12, !13}
