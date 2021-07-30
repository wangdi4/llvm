; LLVM IR generated from testcase below using icx -O2 -mllvm -print-module-before-loopopt
; int main() {
;   unsigned k = -1;
;   int i, j;
;   int ar[255] = {0}, yarrrr[255] = {0};
;   for (i = 0; i < 42; i++) {
;     ar[i] = yarrrr[k++];
;   }
;   return ar[0] + yarrrr[1] + ar[255];
; }
;
; Gather is expected for %yarrrr load.
;

; RUN: opt -hir-ssa-deconstruction -hir-vec-dir-insert -hir-vplan-vec -hir-cg -print-after=hir-vplan-vec -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s
; RUN: opt -passes="hir-ssa-deconstruction,hir-vec-dir-insert,hir-vplan-vec,print<hir>,hir-cg" -vplan-force-vf=4 -disable-output < %s 2>&1 | FileCheck %s

; CHECK: (<4 x i32>*)(%yarrrr)[0][i1 + <i32 0, i32 1, i32 2, i32 3> + -1];

source_filename = "hir_unsigned_wrap.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind readnone uwtable
define dso_local i32 @main() local_unnamed_addr #0 {
entry:
  %ar = alloca [255 x i32], align 16
  %yarrrr = alloca [255 x i32], align 16
  %0 = bitcast [255 x i32]* %ar to i8*
  call void @llvm.lifetime.start.p0i8(i64 1020, i8* nonnull %0) #2
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 %0, i8 0, i64 1020, i1 false)
  %1 = bitcast [255 x i32]* %yarrrr to i8*
  call void @llvm.lifetime.start.p0i8(i64 1020, i8* nonnull %1) #2
  call void @llvm.memset.p0i8.i64(i8* nonnull align 16 %1, i8 0, i64 1020, i1 false)
  br label %for.body

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %k.014 = phi i32 [ -1, %entry ], [ %inc, %for.body ]
  %inc = add nsw i32 %k.014, 1
  %idxprom = zext i32 %k.014 to i64
  %arrayidx = getelementptr inbounds [255 x i32], [255 x i32]* %yarrrr, i64 0, i64 %idxprom
  %2 = load i32, i32* %arrayidx, align 4, !tbaa !2
  %arrayidx2 = getelementptr inbounds [255 x i32], [255 x i32]* %ar, i64 0, i64 %indvars.iv
  store i32 %2, i32* %arrayidx2, align 4, !tbaa !2
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 42
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.body
  %arrayidx4 = getelementptr inbounds [255 x i32], [255 x i32]* %ar, i64 0, i64 0
  %3 = load i32, i32* %arrayidx4, align 16, !tbaa !2
  %arrayidx6 = getelementptr inbounds [255 x i32], [255 x i32]* %ar, i64 0, i64 255
  %4 = load i32, i32* %arrayidx6, align 4, !tbaa !2
  %add7 = add nsw i32 %3, %4
  call void @llvm.lifetime.end.p0i8(i64 1020, i8* nonnull %1) #2
  call void @llvm.lifetime.end.p0i8(i64 1020, i8* nonnull %0) #2
  ret i32 %add7
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.memset.p0i8.i64(i8* nocapture writeonly, i8, i64, i1) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { norecurse nounwind readnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang c878848243a118d2154deca4033daa03379bc6ac) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 70fe93875992d9b09e8ea45280bf500857825374)"}
!2 = !{!3, !4, i64 0}
!3 = !{!"array@_ZTSA255_i", !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
