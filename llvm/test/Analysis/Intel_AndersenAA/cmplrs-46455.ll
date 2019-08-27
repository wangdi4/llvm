; RUN: opt < %s -anders-aa -inline -globalopt -functionattrs -S | FileCheck %s

;
; This is a regression test for the scenario in cmplrs-46455. The scenario
; is as follows:
; 1) Andersen points-to analysis is run over the program, and detects that
; a memory location is created within test_512 and passed to merge_marking_i_w.
;
; 2) Inlining occurs, which eliminates the function test_512, and places the
; the body to be within do_test. (This currently has the effect of needing to
; to modify the Andersen points-to information because Value objects are
; destroyed, and replaced with new ones. In the future, if changes are made
; or the AndersensAA does not rely on this tracking of Value objects directly
; this test case may no longer be necessary.
;
; 3) The GlobalOpt pass is run to cause all the inlining to complete before
; running function passes, which are going to make use of the AndersensAA.
; Whatever pass is run here, must be a Module Level, AndersensAA preserving
; pass.
;
; 4) The deduce function attributes pass is run, which will check whether
; the function merge_masking_i_w touches memory. Because it does, the function
; attribute 'readnone' should not be placed on the function. An error in the
; AndersensAA is detected if the function gets marked as 'readnone'

; ModuleID = 'ld-temp.o'
source_filename = "ld-temp.o"
target datalayout = "e-m:e-p:32:32-f64:32:64-f80:32-n8:16:32-S128"
target triple = "i386-unknown-linux-gnu"

%union.union512i_w = type { <8 x i64> }


; Function Attrs: nounwind
define i32 @main() local_unnamed_addr #0 {
  tail call fastcc void @do_test()
  ret i32 0
}

; Function Attrs: noinline nounwind
define internal fastcc void @do_test() unnamed_addr #1 {
  tail call fastcc void @test_512()
  ret void
}

; Function Attrs: nounwind
define internal fastcc void @test_512() unnamed_addr #2 {
  %1 = alloca %union.union512i_w, align 64
  %2 = alloca %union.union512i_w, align 64
  %3 = alloca %union.union512i_w, align 64
  %4 = alloca [512 x i16], align 2

  %5 = getelementptr inbounds [512 x i16], [512 x i16]* %4, i32 0, i32 0
  call fastcc void @merge_masking_i_w(i16* nonnull %5)
  ret void
  }

; CHECK: define internal fastcc void @merge_masking_i_w(i16* nocapture %0) unnamed_addr #2 {
; CHECK-NOT: attributes #2 { .*readnone.* }
; Function Attrs: noinline norecurse nounwind
define internal fastcc void @merge_masking_i_w(i16* nocapture) unnamed_addr #3 {
  store i16 -1, i16* %0, align 2
  ret void
}

attributes #0 = { nounwind  }
attributes #1 = { noinline nounwind }
attributes #2 = { alwaysinline nounwind }
attributes #3 = { noinline norecurse nounwind }
