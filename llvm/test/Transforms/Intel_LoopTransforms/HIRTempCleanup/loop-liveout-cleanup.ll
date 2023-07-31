; Check that TempCleanup updated liveouts for the loopnest

; RUN: opt < %s -passes="hir-ssa-deconstruction,print<hir-framework>,hir-temp-cleanup,print<hir-framework>" -disable-output -hir-details 2>&1 | FileCheck %s

; CHECK: Function
; CHECK: + LiveOut symbases: 6
; CHECK-NOT: ,
; CHECK-SAME: {{$}}
; CHECK: |   + LiveOut symbases: 12
; CHECK-NOT: ,
; CHECK-SAME: {{$}}
; CHECK: |   + END LOOP
; CHECK: |   + LiveOut symbases: 6
; CHECK-NOT: ,
; CHECK-SAME: {{$}}
; CHECK: |   + END LOOP

; CHECK: Function
; CHECK: + LiveOut symbases: 4
; CHECK-NOT: ,
; CHECK-SAME: {{$}}
; CHECK: |   + LiveOut symbases: 3
; CHECK-NOT: ,
; CHECK-SAME: {{$}}
; CHECK: |   + END LOOP
; CHECK: |   + LiveOut symbases: 4
; CHECK-NOT: ,
; CHECK-SAME: {{$}}
; CHECK: |   + END LOOP

;Module Before HIR; ModuleID = 'loop-liveout-cleanup.c'
source_filename = "loop-liveout-cleanup.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@k = common global [2000 x i32] zeroinitializer, align 16
@.str = private unnamed_addr constant [21 x i8] c"5-failed mix.c x=%f\0A\00", align 1
@.str.1 = private unnamed_addr constant [11 x i8] c"passed %f\0A\00", align 1
@a = common local_unnamed_addr global [1000 x float] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define i32 @main() local_unnamed_addr #0 {
entry:
  %0 = load i32, ptr getelementptr inbounds ([2000 x i32], ptr @k, i64 0, i64 2), align 8
  %cmp1.i = icmp eq i32 %0, 2
  br label %for.cond.i.preheader

for.cond.i.preheader:                             ; preds = %amix5.exit, %entry
  %i.016 = phi i32 [ 0, %entry ], [ %inc, %amix5.exit ]
  br label %for.body.i

for.body.i:                                       ; preds = %for.inc.i, %for.cond.i.preheader
  %indvars.iv = phi i64 [ 0, %for.cond.i.preheader ], [ %indvars.iv.next, %for.inc.i ]
  %x.0.i13 = phi float [ 0.000000e+00, %for.cond.i.preheader ], [ %x.1.i, %for.inc.i ]
  br i1 %cmp1.i, label %if.then.i, label %for.inc.i

if.then.i:                                        ; preds = %for.body.i
  %arrayidx.i = getelementptr inbounds [1000 x float], ptr @a, i64 0, i64 %indvars.iv
  %1 = load float, ptr %arrayidx.i, align 4
  %add.i = fadd float %x.0.i13, %1
  br label %for.inc.i

for.inc.i:                                        ; preds = %if.then.i, %for.body.i
  %x.1.i = phi float [ %add.i, %if.then.i ], [ %x.0.i13, %for.body.i ]
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 1000
  br i1 %exitcond, label %for.body4.i.preheader, label %for.body.i

for.body4.i.preheader:                            ; preds = %for.inc.i
  %x.1.i.lcssa = phi float [ %x.1.i, %for.inc.i ]
  br label %for.body4.i

for.body4.i:                                      ; preds = %for.body4.i.preheader, %for.inc12.i
  %indvars.iv17 = phi i64 [ %indvars.iv.next18, %for.inc12.i ], [ 0, %for.body4.i.preheader ]
  %x.2.i15 = phi float [ %x.3.i, %for.inc12.i ], [ %x.1.i.lcssa, %for.body4.i.preheader ]
  br i1 %cmp1.i, label %if.then7.i, label %for.inc12.i

if.then7.i:                                       ; preds = %for.body4.i
  %arrayidx9.i = getelementptr inbounds [1000 x float], ptr @a, i64 0, i64 %indvars.iv17
  %2 = load float, ptr %arrayidx9.i, align 4
  %add10.i = fadd float %x.2.i15, %2
  br label %for.inc12.i

for.inc12.i:                                      ; preds = %if.then7.i, %for.body4.i
  %x.3.i = phi float [ %add10.i, %if.then7.i ], [ %x.2.i15, %for.body4.i ]
  %indvars.iv.next18 = add nuw nsw i64 %indvars.iv17, 1
  %exitcond19 = icmp eq i64 %indvars.iv.next18, 1000
  br i1 %exitcond19, label %amix5.exit, label %for.body4.i

amix5.exit:                                       ; preds = %for.inc12.i
  %x.3.i.lcssa = phi float [ %x.3.i, %for.inc12.i ]
  %inc = add nuw nsw i32 %i.016, 1
  %exitcond20 = icmp eq i32 %inc, 1000
  br i1 %exitcond20, label %for.end, label %for.cond.i.preheader

for.end:                                          ; preds = %amix5.exit
  %x.3.i.lcssa.lcssa = phi float [ %x.3.i.lcssa, %amix5.exit ]
  %phitmp.le = fpext float %x.3.i.lcssa.lcssa to double
  %cmp1 = fcmp une float %x.3.i.lcssa.lcssa, 4.995000e+05
  br i1 %cmp1, label %if.then, label %if.end

if.then:                                          ; preds = %for.end
  %call4 = tail call i32 (ptr, ...) @printf(ptr @.str, double %phitmp.le)
  tail call void @exit(i32 1) #4
  unreachable

if.end:                                           ; preds = %for.end
  %call6 = tail call i32 (ptr, ...) @printf(ptr @.str.1, double %phitmp.le)
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, ptr nocapture) #1

; Function Attrs: nounwind
declare i32 @printf(ptr nocapture readonly, ...) local_unnamed_addr #2

; Function Attrs: noreturn nounwind
declare void @exit(i32) local_unnamed_addr #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, ptr nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { noreturn nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { noreturn nounwind }


