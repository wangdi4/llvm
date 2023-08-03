; RUN: opt -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt" -S %s | FileCheck %s

; int a = 0;
; int b[4];
; struct { float *p,*q; } c;
; #pragma omp task firstprivate(a,b,c)
;
; For this test's purposes, the user has aligned the objects above, to
; 64-bytes.
; When paropt packs the objects into a task thunk, it must check the uses of
; the private objects, to remove any alignment assumptions that aren't
; in the thunk. It is not clear whether the user should make this assumption,
; but it happens.

; CHECK: define{{.*}}split

; i32, aligned to 4
; CHECK: %loadrez = load i32, ptr %a.gep, align 4

; i32, aligned to 1 due to default ptr assumptions
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 1 %buf.gep, ptr align 1 %

; [ 4 x i32 ], also aligned to 1 due to default ptr assumptions
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 1 %buf2.gep, ptr align 1 %

; 16-byte struct, also aligned to 1 due to default ptr assumptions
; CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 1 %buf3.gep, ptr align 1 %

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.str2 = type { ptr, ptr }

@"@tid.addr" = external global i32

; Function Attrs: mustprogress nounwind uwtable
define dso_local void @_Z3foov() #0 {
entry:
  %a = alloca i32, align 64
  %b = alloca [4 x i32], align 64
  %c = alloca %struct.str2, align 64
  %buf = alloca [64 x i8], align 16
  %buf2 = alloca [64 x i8], align 16
  %buf3 = alloca [64 x i8], align 16
  call void @llvm.lifetime.start.p0(i64 4, ptr %a) #2
  store i32 0, ptr %a, align 4
  call void @llvm.lifetime.start.p0(i64 16, ptr %b) #2
  call void @llvm.lifetime.start.p0(i64 16, ptr %c) #2
  br label %DIR.OMP.TASK.1

DIR.OMP.TASK.1:                                   ; preds = %entry
  br label %DIR.OMP.TASK.2

DIR.OMP.TASK.2:                                   ; preds = %DIR.OMP.TASK.1
  br label %DIR.OMP.TASK.13

DIR.OMP.TASK.13:                                  ; preds = %DIR.OMP.TASK.2
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %a, i32 0, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %b, [4 x i32] zeroinitializer, i32 1),
    "QUAL.OMP.FIRSTPRIVATE:TYPED"(ptr %c, %struct.str2 zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %buf, [64 x i8] zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %buf2, [64 x i8] zeroinitializer, i32 1),
    "QUAL.OMP.PRIVATE:TYPED"(ptr %buf3, [64 x i8] zeroinitializer, i32 1) ]

  br label %DIR.OMP.TASK.3

DIR.OMP.TASK.3:                                   ; preds = %DIR.OMP.TASK.13
  call void @llvm.lifetime.start.p0(i64 64, ptr %buf) #2
  call void @llvm.lifetime.start.p0(i64 64, ptr %buf2) #2
  call void @llvm.lifetime.start.p0(i64 64, ptr %buf3) #2
  %loadrez = load i32, ptr %a, align 4
  call void @llvm.memcpy.p0.p0.i64(ptr align 64 %buf, ptr align 64 %a, i64 4, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 64 %buf2, ptr align 64 %b, i64 4, i1 false)
  call void @llvm.memcpy.p0.p0.i64(ptr align 64 %buf3, ptr align 64 %c, i64 4, i1 false)
  call void @llvm.lifetime.end.p0(i64 64, ptr %buf3) #2
  call void @llvm.lifetime.end.p0(i64 64, ptr %buf2) #2
  call void @llvm.lifetime.end.p0(i64 64, ptr %buf) #2
  br label %DIR.OMP.END.TASK.4

DIR.OMP.END.TASK.4:                               ; preds = %DIR.OMP.TASK.3
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TASK"() ]

  br label %DIR.OMP.END.TASK.5

DIR.OMP.END.TASK.5:                               ; preds = %DIR.OMP.END.TASK.4
  call void @llvm.lifetime.end.p0(i64 16, ptr %c) #2
  call void @llvm.lifetime.end.p0(i64 16, ptr %b) #2
  call void @llvm.lifetime.end.p0(i64 4, ptr %a) #2
  ret void
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn
declare void @llvm.memcpy.p0.p0.i64(ptr noalias nocapture writeonly, ptr noalias nocapture readonly, i64, i1 immarg) #3

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { argmemonly mustprogress nofree nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
