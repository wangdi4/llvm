;  RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -S -passes=dtrans-reorderfieldsop | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Verify that fields are not reordered when any invoke instruction is involved
;   (_Z11doSomethingP4testi).

; CHECK-NOT: %__DFR_struct.test = type { i64, i64, i64, i32, i32, i32, i16 }
; CHECK: %struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.exc = type { i8 }
%struct.test = type { i32, i64, i32, i32, i16, i64, i64 }

$_ZTS3exc = comdat any

$_ZTI3exc = comdat any

@_ZTVN10__cxxabiv117__class_type_infoE = external global ptr
@_ZTS3exc = constant [5 x i8] c"3exc\00", comdat
@_ZTI3exc = constant { ptr, ptr } { ptr getelementptr inbounds (ptr, ptr @_ZTVN10__cxxabiv117__class_type_infoE, i64 2), ptr getelementptr inbounds ([5 x i8], ptr @_ZTS3exc, i32 0, i32 0) }, comdat, !intel_dtrans_type !5

define void @_Z8tryThrowi(i32 %cond) {
entry:
  %tobool = icmp ne i32 %cond, 0
  br i1 %tobool, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  %exception = call ptr @__cxa_allocate_exception(i64 1)
  call void @__cxa_throw(ptr %exception, ptr @_ZTI3exc, ptr null)
  unreachable

if.end:                                           ; preds = %entry
  ret void
}

declare !intel.dtrans.func.type !7 "intel_dtrans_func_index"="1" ptr @__cxa_allocate_exception(i64)

declare !intel.dtrans.func.type !8 void @__cxa_throw(ptr "intel_dtrans_func_index"="1", ptr "intel_dtrans_func_index"="2", ptr "intel_dtrans_func_index"="3")

define i32 @_Z11doSomethingP4testi(ptr "intel_dtrans_func_index"="1" %p, i32 %cond) #0 !intel.dtrans.func.type !10 {
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
  call void @_Z8tryThrowi(i32 %cond)
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

define i32 @main(i32 %argc, ptr "intel_dtrans_func_index"="1" %argv) personality ptr @__gxx_personality_v0 !intel.dtrans.func.type !12 {
entry:
  %call = call noalias ptr @calloc(i64 1, i64 48) #2
  %call1 = invoke i32 @_Z11doSomethingP4testi(ptr %call, i32 %argc)
          to label %invoke.cont unwind label %lpad

invoke.cont:                                      ; preds = %entry
  call void @free(ptr %call) #2
  br label %try.cont

lpad:                                             ; preds = %entry
  %tmp2 = landingpad { ptr, i32 }
          catch ptr @_ZTI3exc
  %tmp3 = extractvalue { ptr, i32 } %tmp2, 0
  %tmp4 = extractvalue { ptr, i32 } %tmp2, 1
  br label %catch.dispatch

catch.dispatch:                                   ; preds = %lpad
  %tmp5 = call i32 @llvm.eh.typeid.for(ptr @_ZTI3exc)
  %matches = icmp eq i32 %tmp4, %tmp5
  br i1 %matches, label %catch, label %eh.resume

catch:                                            ; preds = %catch.dispatch
  %tmp6 = call ptr @__cxa_begin_catch(ptr %tmp3)
  call void @free(ptr %call)
  call void @__cxa_end_catch()
  br label %try.cont

try.cont:                                         ; preds = %catch, %invoke.cont
  %retval.0 = phi i32 [ %call1, %invoke.cont ], [ 0, %catch ]
  ret i32 %retval.0

eh.resume:                                        ; preds = %catch.dispatch
  %lpad.val = insertvalue { ptr, i32 } undef, ptr %tmp3, 0
  %lpad.val4 = insertvalue { ptr, i32 } %lpad.val, i32 %tmp4, 1
  resume { ptr, i32 } %lpad.val4
}

declare !intel.dtrans.func.type !13 "intel_dtrans_func_index"="1" ptr @calloc(i64, i64)
declare i32 @__gxx_personality_v0(...)
declare !intel.dtrans.func.type !14 void @free(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !15 i32 @llvm.eh.typeid.for(ptr "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !16 "intel_dtrans_func_index"="1" ptr @__cxa_begin_catch(ptr "intel_dtrans_func_index"="2")
declare void @__cxa_end_catch()

!1 = !{i8 0, i32 0}  ; i8
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i64 0, i32 0}  ; i64
!4 = !{i16 0, i32 0}  ; i16
!5 = !{!"L", i32 2, !6, !6}  ; { i8*, i8* }
!6 = !{i8 0, i32 1}  ; i8*
!7 = distinct !{!6}
!8 = distinct !{!6, !6, !6}
!9 = !{%struct.test zeroinitializer, i32 1}  ; %struct.test*
!10 = distinct !{!9}
!11 = !{i8 0, i32 2}  ; i8**
!12 = distinct !{!11}
!13 = distinct !{!6}
!14 = distinct !{!6}
!15 = distinct !{!6}
!16 = distinct !{!6, !6}
!17 = !{!"S", %struct.exc zeroinitializer, i32 1, !1} ; { i8 }
!18 = !{!"S", %struct.test zeroinitializer, i32 7, !2, !3, !2, !2, !4, !3, !3} ; { i32, i64, i32, i32, i16, i64, i64 }

!intel.dtrans.types = !{!17, !18}
