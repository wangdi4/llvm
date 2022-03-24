; RUN: opt -instcombine %s -S | FileCheck %s
; RUN: opt -passes=instcombine %s -S | FileCheck %s
; RUN: opt -instcombine %s -opaque-pointers -S | FileCheck %s
; RUN: opt -passes=instcombine %s -opaque-pointers -S | FileCheck %s

; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.s64 = type { i32, i16, i8, i8 }
%struct.data = type { i64, [64 x float] }

; Function Attrs: nounwind uwtable
define dso_local void @_Z3fooP3s64S0_S0_P4data(%struct.s64* %p, %struct.s64* %q, %struct.s64* %r, %struct.data* %d) local_unnamed_addr #0 {
; CHECK-LABEL: @_Z3fooP3s64S0_S0_P4data(
; CHECK:       for.body:
; With -opaque-ptrs we won't have a GEP x,0 here
; CHECK:         [[TMP3:%.*]] = load i32
; CHECK-NEXT:    store i32 [[TMP3]]
; CHECK-NEXT:    [[TMP4:%.*]] = getelementptr inbounds [[STRUCT_S64:.*]], {{.*}} [[Q:%.*]], i64 0, i32 1
; CHECK-NEXT:    [[TMP5:%.*]] = getelementptr inbounds [[STRUCT_S64]], {{.*}} [[P:%.*]], i64 0, i32 1
; CHECK-NEXT:    [[TMP6:%.*]] = load i16, {{.*}} [[TMP4]], align 2, !tbaa [[TBAA11:![0-9]+]]
; CHECK-NEXT:    store i16 [[TMP6]], {{.*}} [[TMP5]], align 2, !tbaa [[TBAA11]]
; CHECK-NEXT:    [[TMP7:%.*]] = getelementptr inbounds [[STRUCT_S64]], {{.*}} [[Q]], i64 0, i32 2
; CHECK-NEXT:    [[TMP8:%.*]] = getelementptr inbounds [[STRUCT_S64]], {{.*}} [[P]], i64 0, i32 2
; CHECK-NEXT:    [[TMP9:%.*]] = load i8, {{.*}} [[TMP7]], align 1, !tbaa [[TBAA13:![0-9]+]]
; CHECK-NEXT:    store i8 [[TMP9]], {{.*}} [[TMP8]], align 1, !tbaa [[TBAA13]]
; CHECK-NEXT:    [[TMP10:%.*]] = getelementptr inbounds [[STRUCT_S64]], {{.*}} [[Q]], i64 0, i32 3
; CHECK-NEXT:    [[TMP11:%.*]] = getelementptr inbounds [[STRUCT_S64]], {{.*}} [[P]], i64 0, i32 3
; CHECK-NEXT:    [[TMP12:%.*]] = load i8, {{.*}} [[TMP10]], align 1, !tbaa [[TBAA13]]
; CHECK-NEXT:    store i8 [[TMP12]], {{.*}} [[TMP11]], align 1, !tbaa [[TBAA13]]
;
entry:
  br label %for.cond

for.cond:                                         ; preds = %for.body, %entry
  %i.0 = phi i32 [ 0, %entry ], [ %inc, %for.body ]
  %conv = sext i32 %i.0 to i64
  %size = getelementptr inbounds %struct.data, %struct.data* %d, i32 0, i32 0, !intel-tbaa !2
  %0 = load i64, i64* %size, align 8, !tbaa !2
  %cmp = icmp ult i64 %conv, %0
  br i1 %cmp, label %for.body, label %for.cond.cleanup

for.cond.cleanup:                                 ; preds = %for.cond
  ret void

for.body:                                         ; preds = %for.cond
  %1 = bitcast %struct.s64* %p to i8*
  %2 = bitcast %struct.s64* %q to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %1, i8* align 4 %2, i64 8, i1 false), !tbaa.struct !9
  %s = getelementptr inbounds %struct.s64, %struct.s64* %r, i32 0, i32 1, !intel-tbaa !15
  %3 = load i16, i16* %s, align 4, !tbaa !15
  %conv1 = sext i16 %3 to i32
  %conv2 = sitofp i32 %conv1 to float
  %A = getelementptr inbounds %struct.data, %struct.data* %d, i32 0, i32 1, !intel-tbaa !17
  %arrayidx = getelementptr inbounds [64 x float], [64 x float]* %A, i64 0, i64 %conv, !intel-tbaa !18
  %4 = load float, float* %arrayidx, align 4, !tbaa !19
  %add = fadd float %4, %conv2
  store float %add, float* %arrayidx, align 4, !tbaa !19
  %inc = add nsw i32 %i.0, 1
  br label %for.cond
}

; Function Attrs: argmemonly nounwind
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* nocapture writeonly, i8* nocapture readonly, i64, i1 immarg) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"icx (ICX) 2019.8.2.0"}
!2 = !{!3, !4, i64 0}
!3 = !{!"struct@_ZTS4data", !4, i64 0, !7, i64 8}
!4 = !{!"long", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!"array@_ZTSA64_f", !8, i64 0}
!8 = !{!"float", !5, i64 0}
!9 = !{i64 0, i64 4, !10, i64 4, i64 2, !12, i64 6, i64 1, !14, i64 7, i64 1, !14}
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !5, i64 0}
!12 = !{!13, !13, i64 0}
!13 = !{!"short", !5, i64 0}
!14 = !{!5, !5, i64 0}
!15 = !{!16, !13, i64 4}
!16 = !{!"struct@_ZTS3s64", !11, i64 0, !13, i64 4, !5, i64 6, !5, i64 7}
!17 = !{!3, !7, i64 8}
!18 = !{!7, !8, i64 0}
!19 = !{!3, !8, i64 8}
