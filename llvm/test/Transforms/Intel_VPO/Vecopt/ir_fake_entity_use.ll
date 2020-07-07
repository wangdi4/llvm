
;RUN: opt -S -VPlanDriver %s | FileCheck %s
; Check that explicit induction with the only use in "bitcast-life.time.start" is not crashed.

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #0

; Function Attrs: argmemonly nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #0

; Function Attrs: norecurse nounwind uwtable
define void @_Z20initialize_variablesiPfS_() {
entry:
; CHECK: <4 x i32>
  %0 = alloca i32, align 4
  br label %region
region:
  %1 = call token @llvm.directive.region.entry() #1 [ "DIR.OMP.SIMD"(), "QUAL.OMP.LINEAR:IV"(i32* %0, i32 1), "QUAL.OMP.SIMDLEN"(i32 4) ]
  br label %body

body:
  %2 = phi i64 [ 0, %region ], [ %4, %body ]
  %3 = bitcast i32* %0 to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* nonnull %3)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* nonnull %3)
  %4 = add nuw i64 %2, 1
  %5 = icmp eq i64 %4, 1000
  br i1 %5, label %end, label %body

end:
  call void @llvm.directive.region.exit(token %1) #1 [ "DIR.OMP.END.SIMD"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #1 = { nounwind }

