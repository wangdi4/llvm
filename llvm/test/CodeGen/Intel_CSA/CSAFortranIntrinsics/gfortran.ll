; RUN: opt -S -csa-fortran-intrinsics < %s | FileCheck %s

; ModuleID = 'fortran-ir-f2zovU.ll'
source_filename = "fortran_intrinsics.f90"
target datalayout = "e-m:e-i64:64-n32:64"
target triple = "csa"

@0 = internal constant i32 5
@1 = internal constant i32 100
@2 = internal constant i32 1

; Function Attrs: nounwind uwtable
define i32 @g_(i32* noalias nocapture readonly %x) unnamed_addr #0 {
; CHECK: call void @llvm.csa.parallel.loop()
; CHECK: call void @llvm.csa.spmd(i32 5, i32 100)
; CHECK: call void @llvm.csa.pipeline.loop(i32 1)
entry:
  tail call void bitcast (void (...)* @builtin_csa_parallel_loop_ to void ()*)() #1
  tail call void bitcast (void (...)* @builtin_csa_spmd_ to void (i32*, i32*)*)(i32* nonnull @0, i32* nonnull @1) #1
  tail call void bitcast (void (...)* @builtin_csa_pipeline_loop_ to void (i32*)*)(i32* nonnull @2) #1
  %0 = load i32, i32* %x, align 4
  ret i32 %0
}

declare void @builtin_csa_parallel_loop_(...) local_unnamed_addr

declare void @builtin_csa_spmd_(...) local_unnamed_addr

declare void @builtin_csa_pipeline_loop_(...) local_unnamed_addr

attributes #0 = { nounwind uwtable "no-frame-pointer-elim-non-leaf"="false" }
attributes #1 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"GCC: (GNU) 4.8.5 LLVM: 6.0.0git-aa6dee1"}
