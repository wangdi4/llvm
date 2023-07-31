; RUN: opt -S -passes="vplan-vec" -vplan-force-vf=4 < %s | FileCheck %s

; CHECK: vector.body:
; CHECK:  %wide.masked.load = call {{.*}} @llvm.masked.load
; CHECK:  call {{.*}} @llvm.masked.store
; CHECK:  %wide.masked.load{{.*}} = call {{.*}} @llvm.masked.load
; CHECK:  call {{.*}} @llvm.masked.store

; ModuleID = 'ts.c'
source_filename = "ts.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@arr2p = external local_unnamed_addr global ptr, align 8
@arr3p = external local_unnamed_addr global ptr, align 8

; Function Attrs: norecurse nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  %tok = call token @llvm.directive.region.entry() [ "DIR.OMP.SIMD"() ]
  br label %for.body.preheader

for.body.preheader:
  %0 = load ptr, ptr @arr3p, align 8
  %1 = load ptr, ptr @arr2p, align 8
  br label %for.body

for.body:                                         ; preds = %for.inc, %for.body.preheader
  %indvars.iv = phi i64 [ 0, %for.body.preheader ], [ %indvars.iv.next, %for.inc ]
  %2 = trunc i64 %indvars.iv to i32
  %rem = srem i32 %2, 3
  %tobool = icmp eq i32 %rem, 0
  br i1 %tobool, label %if.else, label %if.then

if.then:                                          ; preds = %for.body
  %arrayidx = getelementptr inbounds i32, ptr %1, i64 %indvars.iv
  %3 = load i32, ptr %arrayidx, align 4, !tbaa !1
  %4 = trunc i64 %indvars.iv to i32
  %add = add nsw i32 %3, %4
  store i32 %add, ptr %arrayidx, align 4, !tbaa !1
  br label %for.inc

if.else:                                          ; preds = %for.body
  %arrayidx2 = getelementptr inbounds i32, ptr %0, i64 %indvars.iv
  %5 = load i32, ptr %arrayidx2, align 4, !tbaa !1
  %6 = trunc i64 %indvars.iv to i32
  %sub = sub nsw i32 %5, %6
  store i32 %sub, ptr %arrayidx2, align 4, !tbaa !1
  br label %for.inc

for.inc:                                          ; preds = %if.then, %if.else
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 50
  br i1 %exitcond, label %for.end, label %for.body

for.end:                                          ; preds = %for.inc
  call void @llvm.directive.region.exit(token %tok) [ "DIR.OMP.END.SIMD"() ]
  br label %for.cleanup

for.cleanup:                              ; preds = %for.end
  ret void
}

declare token @llvm.directive.region.entry()
declare void @llvm.directive.region.exit(token)

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20949)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
