;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -dtrans-reorderfieldsop | FileCheck %s
;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-reorderfieldsop | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Check that field reordering is not performed for structures tht use new/delete.

; CHECK-NOT: %__DFR_struct.test = type { i64, i64, i64, i32, i32, i32, i16 }
; CHECK: %struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

; CHECK-LABEL: define i32 @main
; CHECK:  %call = call ptr @_Znwm(i64 48)
; CHECK: call void @llvm.memset.p0.i64(ptr align 16 %call, i8 0, i64 48, i1 false)

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.exc = type { i8 }
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

define i32 @_Z11doSomethingP4testi(ptr "intel_dtrans_func_index"="1" %p, i32 %cond) !intel.dtrans.func.type !6 {
entry:
  %i1 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 0
  store i32 10, ptr %i1, align 8
  %i11 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 0
  %tmp = load i32, ptr %i11, align 8
  %add = add nsw i32 %tmp, 20
  %conv = sext i32 %add to i64
  %i2 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 1
  store i64 %conv, ptr %i2, align 8
  %i12 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 0
  %tmp1 = load i32, ptr %i12, align 8
  %add3 = add nsw i32 %tmp1, 30
  %i3 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 2
  store i32 %add3, ptr %i3, align 8
  %i14 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 0
  %tmp2 = load i32, ptr %i14, align 8
  %add5 = add nsw i32 %tmp2, 40
  %i4 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 3
  store i32 %add5, ptr %i4, align 4
  %i16 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 0
  %tmp3 = load i32, ptr %i16, align 8
  %add7 = add nsw i32 %tmp3, 50
  %conv8 = trunc i32 %add7 to i16
  %i5 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 4
  store i16 %conv8, ptr %i5, align 8
  %i19 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 0
  %tmp4 = load i32, ptr %i19, align 8
  %add10 = add nsw i32 %tmp4, 60
  %conv11 = sext i32 %add10 to i64
  %i6 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 5
  store i64 %conv11, ptr %i6, align 8
  %i112 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 0
  %tmp5 = load i32, ptr %i112, align 8
  %add13 = add nsw i32 %tmp5, 70
  %conv14 = sext i32 %add13 to i64
  %i7 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 6
  store i64 %conv14, ptr %i7, align 8
  %i315 = getelementptr inbounds %struct.test, ptr %p, i32 0, i32 2
  %tmp6 = load i32, ptr %i315, align 8
  ret i32 %tmp6
}

define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !8 {
entry:
  %call = call ptr @_Znwm(i64 48)
  call void @llvm.memset.p0.i64(ptr align 16 %call, i8 0, i64 48, i1 false)
  %call2 = call i32 @_Z11doSomethingP4testi(ptr %call, i32 %argc)

  %isnull = icmp eq ptr %call, null
  br i1 %isnull, label %delete.end, label %delete.notnull

delete.notnull:
  call void @_ZdlPv(ptr %call)
  br label %delete.end

delete.end:
  br label %try.cont

try.cont:
  ret i32 %call2
}

declare !intel.dtrans.func.type !10 "intel_dtrans_func_index"="1" ptr @_Znwm(i64)
declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !11 void @llvm.memset.p0.i64(ptr "intel_dtrans_func_index"="1" nocapture writeonly, i8, i64, i1)
declare !intel.dtrans.func.type !12 void @_ZdlPv(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !13 i32 @llvm.eh.typeid.for(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !14 "intel_dtrans_func_index"="1" ptr @__cxa_begin_catch(ptr "intel_dtrans_func_index"="2")
declare void @__cxa_end_catch()

!1 = !{i8 0, i32 0}  ; i8
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i64 0, i32 0}  ; i64
!4 = !{i16 0, i32 0}  ; i16
!5 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!6 = distinct !{!5}
!7 = !{i8 0, i32 2}  ; i8**
!8 = distinct !{!7}
!9 = !{i8 0, i32 1}  ; i8*
!10 = distinct !{!9}
!11 = distinct !{!9}
!12 = distinct !{!9}
!13 = distinct !{!9}
!14 = distinct !{!9, !9}
!15 = !{!"S", %struct.exc zeroinitializer, i32 1, !1} ; { i8 }
!16 = !{!"S", %struct.test zeroinitializer, i32 7, !2, !3, !2, !2, !4, !3, !3} ; { i32, i64, i32, i32, i16, i64, i64 }

!intel.dtrans.types = !{!15, !16}
