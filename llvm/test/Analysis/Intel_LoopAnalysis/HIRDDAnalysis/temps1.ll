;
;  Testing for temps 
;  for  i 
;    for j {
;          = xx
;      xx = 
;   }
; RUN:  opt < %s -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK-DAG:  %xx.140 ANTI (= =)
; CHECK-DAG:  %xx.140 FLOW (<= *)
;
; ModuleID = 't.c'
source_filename = "t.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
; Function Attrs: nounwind readnone uwtable
define i32 @sub() #0 {
entry:
  %yv8 = alloca [100 x i32], align 16
  %0 = bitcast [100 x i32]* %yv8 to i8*
  %q = alloca [100 x i32], align 16
  call void @llvm.lifetime.start(i64 400, i8* %0) #2
  %1 = bitcast [100 x i32]* %q to i8*
  call void @llvm.lifetime.start(i64 400, i8* %1) #2
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %entry, %for.inc14
  %xx.042 = phi i32 [ 8, %entry ], [ %4, %for.inc14 ]
  %jg.041 = phi i32 [ 31, %entry ], [ %dec15, %for.inc14 ]
  br label %for.body3

for.body3:                                        ; preds = %for.cond1.preheader, %for.body3
  %indvars.iv = phi i64 [ 43, %for.cond1.preheader ], [ %indvars.iv.next, %for.body3 ]
  %xx.140 = phi i32 [ %xx.042, %for.cond1.preheader ], [ %4, %for.body3 ]
  %indvars.iv.next = add nsw i64 %indvars.iv, -1
  %arrayidx4 = getelementptr inbounds [100 x i32], [100 x i32]* %yv8, i64 0, i64 %indvars.iv.next
  %2 = load i32, i32* %arrayidx4, align 4, !tbaa !1
  %3 = trunc i64 %indvars.iv to i32
  %sum = add i32 %xx.140, %3
  %sub5 = sub i32 %2, %sum
  store i32 %sub5, i32* %arrayidx4, align 4, !tbaa !1
  %arrayidx8 = getelementptr inbounds [100 x i32], [100 x i32]* %q, i64 0, i64 %indvars.iv.next
  %4 = load i32, i32* %arrayidx8, align 4, !tbaa !1
  %arrayidx10 = getelementptr inbounds [100 x i32], [100 x i32]* %yv8, i64 0, i64 %indvars.iv
  %5 = load i32, i32* %arrayidx10, align 4, !tbaa !1
  %6 = add nsw i64 %indvars.iv, -2
  %arrayidx13 = getelementptr inbounds [100 x i32], [100 x i32]* %yv8, i64 0, i64 %6
  store i32 %5, i32* %arrayidx13, align 4, !tbaa !1
  %cmp2 = icmp ugt i64 %indvars.iv.next, 1
  br i1 %cmp2, label %for.body3, label %for.inc14

for.inc14:                                        ; preds = %for.body3
  %dec15 = add nsw i32 %jg.041, -1
  %cmp = icmp ugt i32 %dec15, 1
  br i1 %cmp, label %for.cond1.preheader, label %for.end16

for.end16:                                        ; preds = %for.inc14
  %arrayidx17 = getelementptr inbounds [100 x i32], [100 x i32]* %yv8, i64 0, i64 5
  %7 = load i32, i32* %arrayidx17, align 4, !tbaa !1
  call void @llvm.lifetime.end(i64 400, i8* nonnull %1) #2
  call void @llvm.lifetime.end(i64 400, i8* nonnull %0) #2
  ret i32 %7
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind readnone uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.9.0 (trunk 12506) (llvm/branches/loopopt 15736)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
