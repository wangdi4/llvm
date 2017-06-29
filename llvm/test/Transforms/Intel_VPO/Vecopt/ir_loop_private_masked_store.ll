; void foo(long *ip, long *ip2)
; {
;   long index;
; 
; #pragma omp simd simdlen(4)
;   for (index = 0; index < 1024; index++) {
;     long val;
; 
;     val = index;
;     if (ip[index])
;       val = ip2[index];
; 
;     ip[index] = val;
;   }
; }
;
; RUN: opt -VPlanDriver -S %s | FileCheck %s
; CHECK: vector.ph:
; CHECK: vector.body:
; CHECK: store <4 x i64> %vec.ind, <4 x i64>* %val.vec
; CHECK: call void @llvm.masked.store.v4i64.p0v4i64(<4 x i64> {{.*}}, <4 x i64>* %val.vec, {{.*}})
;     
; ModuleID = 't1.c'
source_filename = "t1.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind uwtable
define void @foo(i64* nocapture %ip, i64* nocapture readonly %ip2) local_unnamed_addr #0 {
entry:
  %val = alloca i64, align 8
  br label %DIR.OMP.SIMD.1

DIR.OMP.SIMD.1:                                   ; preds = %entry
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive.qual.opnd.i32(metadata !"QUAL.OMP.SIMDLEN", i32 4)
  call void (metadata, ...) @llvm.intel.directive.qual.opndlist(metadata !"QUAL.OMP.PRIVATE", i64* nonnull %val)
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.2

DIR.QUAL.LIST.END.2:                              ; preds = %DIR.OMP.SIMD.1
  %0 = bitcast i64* %val to i8*
  br label %omp.inner.for.body

omp.inner.for.body:                               ; preds = %if.end, %DIR.QUAL.LIST.END.2
  %.omp.iv.012 = phi i64 [ 0, %DIR.QUAL.LIST.END.2 ], [ %add3, %if.end ]
  call void @llvm.lifetime.start(i64 8, i8* nonnull %0) #2
  store i64 %.omp.iv.012, i64* %val, align 8, !tbaa !1
  %arrayidx = getelementptr inbounds i64, i64* %ip, i64 %.omp.iv.012
  %1 = load i64, i64* %arrayidx, align 8, !tbaa !1
  %tobool = icmp eq i64 %1, 0
  br i1 %tobool, label %if.end, label %if.then

if.then:                                          ; preds = %omp.inner.for.body
  %arrayidx1 = getelementptr inbounds i64, i64* %ip2, i64 %.omp.iv.012
  %2 = load i64, i64* %arrayidx1, align 8, !tbaa !1
  store i64 %2, i64* %val, align 8, !tbaa !1
  br label %if.end

if.end:                                           ; preds = %omp.inner.for.body, %if.then
  %3 = phi i64 [ %.omp.iv.012, %omp.inner.for.body ], [ %2, %if.then ]
  store i64 %3, i64* %arrayidx, align 8, !tbaa !1
  call void @llvm.lifetime.end(i64 8, i8* nonnull %0) #2
  %add3 = add nuw nsw i64 %.omp.iv.012, 1
  %exitcond = icmp eq i64 %add3, 1024
  br i1 %exitcond, label %omp.loop.exit, label %omp.inner.for.body

omp.loop.exit:                                    ; preds = %if.end
  call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.3

DIR.QUAL.LIST.END.3:                              ; preds = %omp.loop.exit
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opnd.i32(metadata, i32) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 21478)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"long", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
