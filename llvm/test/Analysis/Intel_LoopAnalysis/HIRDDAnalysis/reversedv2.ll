
;   for (i=0; i <  N; i++) {
;        for (j=0; j <  N; j++) {
; 					for (k=0; k <  N; k++) {
;            p[2*i - 4*j][k] = 1;
;            q[i+j+k] =  p[6*i +8 *j][k-1];
;

; RUN:  opt < %s  -loop-simplify  -hir-ssa-deconstruction | opt  -hir-dd-analysis  -hir-dd-analysis-verify=Region  -analyze  | FileCheck %s 
; CHECK: 'HIR Data Dependence Analysis' for function 'main'
; CHECK-DAG:  FLOW (<= * >)
; CHECK-DAG:  ANTI (<= * <)
; ModuleID = 'reverse2.c'
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct._IO_FILE = type { i32, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, i8*, %struct._IO_marker*, %struct._IO_FILE*, i32, i32, i64, i16, i8, [1 x i8], i8*, i64, i8*, i8*, i8*, i8*, i64, i32, [20 x i8] }
%struct._IO_marker = type { %struct._IO_marker*, %struct._IO_FILE*, i32 }

@p = common global [10000 x [100 x i64]] zeroinitializer, align 16
@q = common global [100000 x i64] zeroinitializer, align 16
@stderr = external global %struct._IO_FILE*, align 8
@.str = private unnamed_addr constant [5 x i8] c" %d \00", align 1

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  br label %for.cond.1.preheader

for.cond.1.preheader:                             ; preds = %for.inc.20, %entry
  %i.043 = phi i64 [ 0, %entry ], [ %inc21, %for.inc.20 ]
  %mul = shl nsw i64 %i.043, 1
  %mul10 = mul nuw nsw i64 %i.043, 6
  br label %for.cond.4.preheader

for.cond.4.preheader:                             ; preds = %for.inc.17, %for.cond.1.preheader
  %j.042 = phi i64 [ 0, %for.cond.1.preheader ], [ %inc18, %for.inc.17 ]
  %mul7 = shl nsw i64 %j.042, 2
  %sub = sub nsw i64 %mul, %mul7
  %mul11 = shl i64 %j.042, 3
  %add = add nuw nsw i64 %mul11, %mul10
  %add14 = add nuw nsw i64 %j.042, %i.043
  br label %for.body.6

for.body.6:                                       ; preds = %for.body.6, %for.cond.4.preheader
  %k.041 = phi i64 [ 0, %for.cond.4.preheader ], [ %inc, %for.body.6 ]
  %arrayidx8 = getelementptr inbounds [10000 x [100 x i64]], [10000 x [100 x i64]]* @p, i64 0, i64 %sub, i64 %k.041
  store i64 1, i64* %arrayidx8, align 8, !tbaa !1
  %sub9 = add nsw i64 %k.041, -1
  %arrayidx13 = getelementptr inbounds [10000 x [100 x i64]], [10000 x [100 x i64]]* @p, i64 0, i64 %add, i64 %sub9
  %0 = load i64, i64* %arrayidx13, align 8, !tbaa !1
  %add15 = add nuw nsw i64 %add14, %k.041
  %arrayidx16 = getelementptr inbounds [100000 x i64], [100000 x i64]* @q, i64 0, i64 %add15
  store i64 %0, i64* %arrayidx16, align 8, !tbaa !1
  %inc = add nuw nsw i64 %k.041, 1
  %exitcond = icmp eq i64 %inc, 100
  br i1 %exitcond, label %for.inc.17, label %for.body.6

for.inc.17:                                       ; preds = %for.body.6
  %inc18 = add nuw nsw i64 %j.042, 1
  %exitcond44 = icmp eq i64 %inc18, 100
  br i1 %exitcond44, label %for.inc.20, label %for.cond.4.preheader

for.inc.20:                                       ; preds = %for.inc.17
  %inc21 = add nuw nsw i64 %i.043, 1
  %exitcond45 = icmp eq i64 %inc21, 100
  br i1 %exitcond45, label %for.end.22, label %for.cond.1.preheader

for.end.22:                                       ; preds = %for.inc.20
  %1 = load %struct._IO_FILE*, %struct._IO_FILE** @stderr, align 8, !tbaa !5
  %2 = load i64, i64* getelementptr inbounds ([100000 x i64], [100000 x i64]* @q, i64 0, i64 100), align 16, !tbaa !1
  %call = tail call i32 (%struct._IO_FILE*, i8*, ...) @fprintf(%struct._IO_FILE* %1, i8* nonnull getelementptr inbounds ([5 x i8], [5 x i8]* @.str, i64 0, i64 0), i64 %2) #2
  ret i32 0
}

; Function Attrs: nounwind
declare i32 @fprintf(%struct._IO_FILE* nocapture, i8* nocapture readonly, ...) #1

attributes #0 = { nounwind uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { cold nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 3.8.0 (trunk 1607) (llvm/branches/loopopt 1631)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !6, i64 0}
!6 = !{!"any pointer", !3, i64 0}
