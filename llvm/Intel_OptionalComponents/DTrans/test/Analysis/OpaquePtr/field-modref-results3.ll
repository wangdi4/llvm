; REQUIRES: asserts
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -dtrans-fieldmodrefop-analysis -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck --check-prefix=CHECK-TYPED  %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodrefop-analysis>' -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck --check-prefix=CHECK-TYPED %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -dtrans-fieldmodrefop-analysis -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck --check-prefix=CHECK-OPAQUE %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -dtrans-outofboundsok=false -dtrans-usecrulecompat -passes='require<dtrans-fieldmodrefop-analysis>' -dtrans-fieldmodref-eval -disable-output 2>&1 | FileCheck --check-prefix=CHECK-OPAQUE %s

; Check handling of address taken functions for field based Mod/Ref analysis
; for a case using a broker function.

; Pass the address of a function, using a bitcast function type, to use a
; callback function, and the address of a function for a parameter that will
; be forwarded to the callback function. This case should be safe for the
; mod/ref analysis because the addresses will only be used to call the functions.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.test01 = type { i32, i32*, i64 }

%struct.ident_t = type { i32, i32, i32, i32, i8* }
@.kmpc_loc.0.0.27 = private global %struct.ident_t { i32 0, i32 838860802, i32 0, i32 0, i8* getelementptr inbounds ([22 x i8], [22 x i8]* @.source.0.0.694, i32 0, i32 0) }
@.source.0.0.694 = private constant [22 x i8] c";unknown;unknown;0;0;;"

define internal void @testbase(%struct.test01* "intel_dtrans_func_index"="1" %in) !intel.dtrans.func.type !7 {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %in, i64 0, i32 0
  %ld1 = load i32, i32* %fieldaddr

  ; ModRef queries of this call site should identify the result as 'ModRef' because
  ; 'test01' will directly modify the field, and it will reach a function that is
  ; indirectly called that will reference the field.
  call void @test01()
  ret void
}

define internal void @test01() {
  %st_mem = call i8* @malloc(i64 24)
  %st = bitcast i8* %st_mem to %struct.test01*

  %f0 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  store i32 8, i32* %f0

  %ar1_mem = call i8* @malloc(i64 64)
  %ar1_mem2 = bitcast i8* %ar1_mem to i32*
  %f1 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 1
  store i32* %ar1_mem2, i32** %f1

  %f2 = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 2
  store i64 1, i64* %f2

  tail call void (%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...) @broker(
    %struct.ident_t* @.kmpc_loc.0.0.27, i32 6, void (i32*, i32*, ...)* bitcast
      (void (i32*, i32*, i64, %struct.test01*, void (%struct.test01*)*, void (%struct.test01*)*)* @use01a to void (i32*, i32*, ...)*),
    i64 1,
    %struct.test01* %st,
	  void (%struct.test01*)* @filter01a,
	  void (%struct.test01*)* @filter01b
  )

  ret void
}

; Function that will be address taken because it is a callback for the broker
; function.
define void @use01a(i32* "intel_dtrans_func_index"="1" %in0, i32* "intel_dtrans_func_index"="2" %in1, i64 %in2, %struct.test01* "intel_dtrans_func_index"="3" %in3, void (%struct.test01*)* "intel_dtrans_func_index"="4" nocapture %filter1, void (%struct.test01*)* "intel_dtrans_func_index"="5" nocapture %filter2) !intel.dtrans.func.type !11 {
  call void %filter1(%struct.test01* %in3), !intel_dtrans_type !8
  call void %filter2(%struct.test01* %in3), !intel_dtrans_type !8

  ; use field 1 within this callback routine.
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %in3, i64 0, i32 1
  %ld1 = load i32*, i32** %fieldaddr

  ret void
}

; Function that will be address taken because it is a pass-through to the
; callback function.
define void @filter01a(%struct.test01* "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !12 {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 0
  %ld1 = load i32, i32* %fieldaddr
  ret void
}

; Function that will be address taken because it is a pass-through to the
; callback function.
define void @filter01b(%struct.test01* "intel_dtrans_func_index"="1" %st) !intel.dtrans.func.type !13 {
  %fieldaddr = getelementptr %struct.test01, %struct.test01* %st, i64 0, i32 2
  store i64 0, i64* %fieldaddr
  ret void
}

declare !intel.dtrans.func.type !17 !callback !0 void @broker(%struct.ident_t* "intel_dtrans_func_index"="1" %0, i32 %1, void (i32*, i32*, ...)* "intel_dtrans_func_index"="2" %2, ...)
declare !intel.dtrans.func.type !18 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

; CHECK-TYPED: FieldModRefQuery: - ModRef     : [testbase]   %ld1 = load i32, i32* %fieldaddr, align 4 --   call void @test01()
; CHECK-TYPED: FieldModRefQuery: - Ref        : [test01]   store i32 8, i32* %f0, align 4 --   tail call void (%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...) @broker(%struct.ident_t* @.kmpc_loc.0.0.27, i32 6, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i64, %struct.test01*, void (%struct.test01*)*, void (%struct.test01*)*)* @use01a to void (i32*, i32*, ...)*), i64 1, %struct.test01* %st, void (%struct.test01*)* @filter01a, void (%struct.test01*)* @filter01b)
; CHECK-TYPED: FieldModRefQuery: - Ref        : [test01]   store i32* %ar1_mem2, i32** %f1, align 8 --   tail call void (%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...) @broker(%struct.ident_t* @.kmpc_loc.0.0.27, i32 6, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i64, %struct.test01*, void (%struct.test01*)*, void (%struct.test01*)*)* @use01a to void (i32*, i32*, ...)*), i64 1, %struct.test01* %st, void (%struct.test01*)* @filter01a, void (%struct.test01*)* @filter01b)
; CHECK-TYPED: FieldModRefQuery: - Mod        : [test01]   store i64 1, i64* %f2, align 8 --   tail call void (%struct.ident_t*, i32, void (i32*, i32*, ...)*, ...) @broker(%struct.ident_t* @.kmpc_loc.0.0.27, i32 6, void (i32*, i32*, ...)* bitcast (void (i32*, i32*, i64, %struct.test01*, void (%struct.test01*)*, void (%struct.test01*)*)* @use01a to void (i32*, i32*, ...)*), i64 1, %struct.test01* %st, void (%struct.test01*)* @filter01a, void (%struct.test01*)* @filter01b)

; CHECK-OPAQUE: FieldModRefQuery: - ModRef     : [testbase]   %ld1 = load i32, ptr %fieldaddr, align 4 --   call void @test01()
; CHECK-OPAQUE: FieldModRefQuery: - Ref        : [test01]   store i32 8, ptr %f0, align 4 --   tail call void (ptr, i32, ptr, ...) @broker(ptr @.kmpc_loc.0.0.27, i32 6, ptr @use01a, i64 1, ptr %st, ptr @filter01a, ptr @filter01b)
; CHECK-OPAQUE: FieldModRefQuery: - Ref        : [test01]   store ptr %ar1_mem2, ptr %f1, align 8 --   tail call void (ptr, i32, ptr, ...) @broker(ptr @.kmpc_loc.0.0.27, i32 6, ptr @use01a, i64 1, ptr %st, ptr @filter01a, ptr @filter01b)
; CHECK-OPAQUE: FieldModRefQuery: - Mod        : [test01]   store i64 1, ptr %f2, align 8 --   tail call void (ptr, i32, ptr, ...) @broker(ptr @.kmpc_loc.0.0.27, i32 6, ptr @use01a, i64 1, ptr %st, ptr @filter01a, ptr @filter01b)

!0 = !{!1}
!1 = !{i64 2, i64 -1, i64 -1, i1 true}
!2 = !{i32 0, i32 0}  ; i32
!3 = !{i32 0, i32 1}  ; i32*
!4 = !{i64 0, i32 0}  ; i64
!5 = !{i8 0, i32 1}  ; i8*
!6 = !{%struct.test01 zeroinitializer, i32 1}  ; %struct.test01*
!7 = distinct !{!6}
!8 = !{!"F", i1 false, i32 1, !9, !6}  ; void (%struct.test01*)
!9 = !{!"void", i32 0}  ; void
!10 = !{!8, i32 1}  ; void (%struct.test01*)*
!11 = distinct !{!3, !3, !6, !10, !10}
!12 = distinct !{!6}
!13 = distinct !{!6}
!14 = !{%struct.ident_t zeroinitializer, i32 1}  ; %struct.ident_t*
!15 = !{!"F", i1 true, i32 2, !9, !3, !3}  ; void (i32*, i32*, ...)
!16 = !{!15, i32 1}  ; void (i32*, i32*, ...)*
!17 = distinct !{!14, !16}
!18 = distinct !{!5}
!19 = !{!"S", %struct.test01 zeroinitializer, i32 3, !2, !3, !4} ; { i32, i32*, i64 }
!20 = !{!"S", %struct.ident_t zeroinitializer, i32 5, !2, !2, !2, !2, !5} ; { i32, i32, i32, i32, i8* }

!intel.dtrans.types = !{!19, !20}
