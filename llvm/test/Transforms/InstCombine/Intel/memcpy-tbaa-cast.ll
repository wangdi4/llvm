; RUN: opt -passes=instcombine -S %s | FileCheck %s
; CHECK: load i64{{.*}}!tbaa [[LOADTYPE:![0-9]+]]
; CHECK: [[SHORTTYPE:![0-9]+]] ={{.*}}"short"
; CHECK: [[LOADTYPE]] = {{.*}}[[SHORTTYPE]]

;struct array_of_short {
;  short x[4];
;};
;
;struct single_short {
;  short x;
;};
;
;array_of_short bar(array_of_short *p, int idx) {
;  single_short *bad_ptr = (single_short *)(&(p->x[idx]));
;  bad_ptr->x = 1;
;  return *p;
;}
; 
; In the above code, the user has (incorrectly) cast the array_of_short
; elements to a "boxed" short type. The function return is implemented
; internally with a memcpy of type "array@short". After InstCombine lowers
; the memcpy to a load/store pair, we want the load/store types to be
; "short", not "array@short". This allows the lowered memcpy to alias
; memory accesses of "single_short" type, as well as "array@short".

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.array_of_short = type { [4 x i16] }
%struct.single_short = type { i16 }

; Function Attrs: mustprogress nounwind uwtable
define dso_local i64 @_Z3barP14array_of_shorti(ptr noundef %p, i32 noundef %idx) {
entry:
  %retval = alloca %struct.array_of_short, align 2
  %x = getelementptr inbounds %struct.array_of_short, ptr %p, i32 0, i32 0, !intel-tbaa !9
  %idxprom = sext i32 %idx to i64
  %arrayidx = getelementptr inbounds [4 x i16], ptr %x, i64 0, i64 %idxprom, !intel-tbaa !13
  %x1 = getelementptr inbounds %struct.single_short, ptr %arrayidx, i32 0, i32 0, !intel-tbaa !16
  store i16 1, ptr %x1, align 2, !tbaa !16
  call void @llvm.memcpy.p0.p0.i64(ptr align 2 %retval, ptr align 2 %p, i64 8, i1 false), !tbaa.struct !18
  %coerce.dive = getelementptr inbounds %struct.array_of_short, ptr %retval, i32 0, i32 0
  %rint = load i64, ptr %coerce.dive, align 2
  ret i64 %rint
}

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite)
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #2

; Function Attrs: mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { mustprogress nounwind uwtable }
attributes #1 = { mustprogress nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { mustprogress nocallback nofree nounwind willreturn memory(argmem: readwrite) }
attributes #3 = { nounwind }

!llvm.module.flags = !{!0, !1}
!llvm.ident = !{!2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"uwtable", i32 2}
!2 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.2.0 (2023.x.0.YYYYMMDD)"}
!3 = !{!4, !4, i64 0}
!4 = !{!"pointer@_ZTSP14array_of_short", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C++ TBAA"}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !5, i64 0}
!9 = !{!10, !11, i64 0}
!10 = !{!"struct@_ZTS14array_of_short", !11, i64 0}
; this is "array of short"
!11 = !{!"array@_ZTSA4_s", !12, i64 0}
!12 = !{!"short", !5, i64 0}
!13 = !{!11, !12, i64 0}
!14 = !{!15, !15, i64 0}
!15 = !{!"pointer@_ZTSP12single_short", !5, i64 0}
!16 = !{!17, !12, i64 0}
!17 = !{!"struct@_ZTS12single_short", !12, i64 0}
!18 = !{i64 0, i64 8, !19}
!19 = !{!11, !11, i64 0}
