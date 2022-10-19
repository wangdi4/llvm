; REQUIRES: asserts
; RUN: opt -opaque-pointers -disable-output -whole-program-assume -intel-libirc-allowed -passes=dtrans-ptrtypeanalyzertest -dtrans-print-pta-results < %s 2>&1 | FileCheck %s

target triple = "x86_64-unknown-linux-gnu"

; Test inferring the type of an input argument on PAROPT generated
; functions that lack metadata based on the uses of the argument. This
; case is similar to ptrtype-arg-infer07.ll, but allows the argument to
; have multiple type aliases.

%struct._TimerInfo = type { %struct._Timer, %struct._Timer, i32, i64 }
%struct._Timer = type { double, double, double }


; This case uses %arg2 as %struct._TimerInfo* and also directly accesses the element
; zero field, causing there to be multiple types associated with the argument. However,
; the element zero detection should resolve the additional type as safe, rather than
; marking it as 'unhandled'

; CHECK:  Input Parameters: test1.DIR.OMP.PARALLEL.LOOP.2.split
; CHECK:    Arg 2: ptr %arg2
; CHECK:    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK:      Aliased types:
; CHECK:        %struct._TimerInfo**
; CHECK:        double**
; CHECK:      No element pointees.
; CHECK;    Arg 3: i32 %arg3
define internal void @test1.DIR.OMP.PARALLEL.LOOP.2.split(ptr %arg0, ptr %arg1, ptr %arg2, i32 %arg3) #0 {
  %i28 = load ptr, ptr %arg2

  %i47 = getelementptr inbounds %struct._TimerInfo, ptr %i28, i64 0, i32 2
  %i48 = load i32, ptr %i47

  %i50 = getelementptr inbounds %struct._TimerInfo, ptr %i28, i64 0, i32 1, i32 2
  %i51 = load double, ptr %i50

  %i54 = getelementptr inbounds %struct._TimerInfo, ptr %i28, i64 0, i32 0, i32 1
  store double 0.000000e+00, ptr %i54

  %i55 = load double, ptr %i28
  %i56 = fsub fast double 1.000000e-15, %i55

  ret void
}

; In this case %arg2 is not used as a pointer to an aggregrate type, so will just
; collect the types the argument gets used as.

; CHECK  Input Parameters: test2.DIR.OMP.PARALLEL.LOOP.2.split
; CHECK;    Arg 2: ptr %arg2
; CHECK;    LocalPointerInfo:
; CHECK-NOT: UNHANDLED
; CHECK;      Aliased types:
; CHECK;        float**
; CHECK;        i8**
; CHECK;      No element pointees.
; CHECK;    Arg 3: i32 %arg3
define internal void @test2.DIR.OMP.PARALLEL.LOOP.2.split(ptr %arg0, ptr %arg1, ptr %arg2, i32 %arg3) #0 {
  %i29 = load ptr, ptr %arg2
  %i193 = load float, ptr %i29
  %i216 = getelementptr i8, ptr %i29, i32 %arg3
  %i217 = load float, ptr %i216

  ret void
}

attributes #0 = { norecurse nounwind uwtable "processed-by-vpo" }
!1 = !{%struct._Timer zeroinitializer, i32 0}  ; %struct._Timer
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i64 0, i32 0}  ; i64
!4 = !{double 0.0e+00, i32 0}  ; double
!5 = !{!"S", %struct._TimerInfo zeroinitializer, i32 4, !1, !1, !2, !3} ; { %struct._Timer, %struct._Timer, i32, i64 }
!6 = !{!"S", %struct._Timer zeroinitializer, i32 3, !4, !4, !4} ; { double, double, double }

!intel.dtrans.types = !{!5, !6}
