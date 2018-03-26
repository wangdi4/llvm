; RUN: opt -S -VPlanDriver < %s | FileCheck %s

; CHECK: vector.body:
; CHECK: VPlannedBB:{{.*}} preds = %VPlannedBB{{.*}}, %vector.body
; CHECK:  store <4 x i32>
; CHECK:  icmp eq {{.*}}, 100
; CHECK: VPlannedBB{{.*}} preds = %VPlannedBB{{.*}}
; CHECK:  icmp eq {{.*}}, 200
; CHECK: VPlannedBB{{.*}} preds = %VPlannedBB{{.*}}
; CHECK:  icmp eq {{.*}}, 300
; CHECK: middle.block:

; ModuleID = 'krtest2.c'
source_filename = "krtest2.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@ip = common local_unnamed_addr global [100 x [200 x [300 x i32]]] zeroinitializer, align 16

; Function Attrs: nounwind uwtable
define void @foo() local_unnamed_addr #0 {
entry:
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %for.cond1.preheader

for.cond1.preheader:                              ; preds = %for.inc15, %entry
  %indvars.iv37 = phi i64 [ 0, %entry ], [ %indvars.iv.next38, %for.inc15 ]
  br label %for.cond4.preheader

for.cond4.preheader:                              ; preds = %for.inc12, %for.cond1.preheader
  %indvars.iv33 = phi i64 [ 0, %for.cond1.preheader ], [ %indvars.iv.next34, %for.inc12 ]
  %0 = add nuw nsw i64 %indvars.iv33, %indvars.iv37
  br label %for.body6

for.body6:                                        ; preds = %for.body6, %for.cond4.preheader
  %indvars.iv = phi i64 [ 0, %for.cond4.preheader ], [ %indvars.iv.next, %for.body6 ]
  %1 = add nuw nsw i64 %0, %indvars.iv
  %arrayidx11 = getelementptr inbounds [100 x [200 x [300 x i32]]], [100 x [200 x [300 x i32]]]* @ip, i64 0, i64 %indvars.iv, i64 %indvars.iv33, i64 %indvars.iv37
  %2 = trunc i64 %1 to i32
  store i32 %2, i32* %arrayidx11, align 4, !tbaa !1
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1
  %exitcond = icmp eq i64 %indvars.iv.next, 100
  br i1 %exitcond, label %for.inc12, label %for.body6

for.inc12:                                        ; preds = %for.body6
  %indvars.iv.next34 = add nuw nsw i64 %indvars.iv33, 1
  %exitcond36 = icmp eq i64 %indvars.iv.next34, 200
  br i1 %exitcond36, label %for.inc15, label %for.cond4.preheader

for.inc15:                                        ; preds = %for.inc12
  %indvars.iv.next38 = add nuw nsw i64 %indvars.iv37, 1
  %exitcond39 = icmp eq i64 %indvars.iv.next38, 300
  br i1 %exitcond39, label %for.end17, label %for.cond1.preheader

for.end17:                                        ; preds = %for.inc15
  tail call void @llvm.intel.directive(metadata !"DIR.OMP.END.SIMD")
  tail call void @llvm.intel.directive(metadata !"DIR.QUAL.LIST.END")
  br label %DIR.QUAL.LIST.END.1

DIR.QUAL.LIST.END.1:                              ; preds = %for.end17
  ret void
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start(i64, i8* nocapture) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual(metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.intel.directive.qual.opndlist(metadata, ...) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="core-avx2" "target-features"="+aes,+avx,+avx2,+bmi,+bmi2,+cx16,+f16c,+fma,+fsgsbase,+fxsr,+lzcnt,+mmx,+movbe,+pclmul,+popcnt,+rdrnd,+rtm,+sse,+sse2,+sse3,+sse4.1,+sse4.2,+ssse3,+x87,+xsave,+xsaveopt" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }

!llvm.ident = !{!0}

!0 = !{!"clang version 4.0.0 (branches/vpo 20965)"}
!1 = !{!2, !2, i64 0}
!2 = !{!"int", !3, i64 0}
!3 = !{!"omnipotent char", !4, i64 0}
!4 = !{!"Simple C/C++ TBAA"}
!5 = !{!6, !2, i64 0}
!6 = !{!"array@_ZTSA100_A100_i", !7, i64 0}
!7 = !{!"array@_ZTSA100_i", !2, i64 0}
