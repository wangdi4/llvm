; This test verifies that other structs are skipped when dtrans-soatoaosop-typename
; option is specified. Checks that "%class.test" is rejected when
; -dtrans-soatoaosop-typename=noname option is specified.

; RUN: opt -S < %s -dtrans-soatoaosop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -dtrans-soatoaosop-typename=noname -disable-output 2>&1 | FileCheck %s
; RUN: opt -S < %s -passes=dtrans-soatoaosop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -dtrans-soatoaosop-typename=noname -disable-output 2>&1 | FileCheck %s
; RUN: opt -S < %s -force-opaque-pointers -dtrans-soatoaosop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -dtrans-soatoaosop-typename=noname -disable-output 2>&1 | FileCheck %s
; RUN: opt -S < %s -force-opaque-pointers -passes=dtrans-soatoaosop -whole-program-assume -enable-intel-advanced-opts -mtriple=i686-- -mattr=+avx2 -debug-only=dtrans-soatoaosop -dtrans-soatoaosop-typename=noname -disable-output 2>&1 | FileCheck %s
; REQUIRES: asserts

; CHECK: Rejecting %class.test based on dtrans-soatoaosop-typename option.

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%class.test = type { i32*, i32* }

; Function Attrs: mustprogress nofree nounwind uwtable willreturn
define dso_local noalias "intel_dtrans_func_index"="1" %class.test* @_Z3foov() !intel.dtrans.func.type !5 {
entry:
  %call = tail call noalias align 16 dereferenceable_or_null(16) i8* @malloc(i64 16)
  %0 = bitcast i8* %call to %class.test*
  ret %class.test* %0
}

; Function Attrs: inaccessiblememonly mustprogress nofree nounwind willreturn
declare !intel.dtrans.func.type !7 dso_local noalias noundef align 16 "intel_dtrans_func_index"="1" i8* @malloc(i64 noundef)

!llvm.module.flags = !{!0, !1}
!intel.dtrans.types = !{!2}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 1}
!2 = !{!"S", %class.test zeroinitializer, i32 2, !3, !3}
!3 = !{i32 0, i32 1}
!4 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2021.4.0 (2021.x.0.YYYYMMDD)"}
!5 = distinct !{!6}
!6 = !{%class.test zeroinitializer, i32 1}
!7 = distinct !{!8}
!8 = !{i8 0, i32 1}
