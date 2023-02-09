; RUN: opt -opaque-pointers=0 -passes="function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt" -S %s | FileCheck %s

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
; CHECK: %loadrez = load i32, i32* %a.gep, align 4

; i32, aligned to 4
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %arraydecay, i8* align 4 %

; [ 4 x i32 ], also aligned to 4
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %arraydecay1, i8* align 4 %

; 16-byte struct, aligned to 8 (largest element)
; CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 8 %arraydecay2, i8* align 8 %

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

%struct.str2 = type { float*, float* }

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
  %0 = bitcast i32* %a to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  store i32 0, i32* %a, align 4
  %1 = bitcast [4 x i32]* %b to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %1) #2
  %2 = bitcast %struct.str2* %c to i8*
  call void @llvm.lifetime.start.p0i8(i64 16, i8* %2) #2
  br label %DIR.OMP.TASK.1

DIR.OMP.TASK.1:                                   ; preds = %entry
  br label %DIR.OMP.TASK.2

DIR.OMP.TASK.2:                                   ; preds = %DIR.OMP.TASK.1
  br label %DIR.OMP.TASK.13

DIR.OMP.TASK.13:                                  ; preds = %DIR.OMP.TASK.2
  %3 = call token @llvm.directive.region.entry() [ "DIR.OMP.TASK"(), "QUAL.OMP.FIRSTPRIVATE"(i32* %a), "QUAL.OMP.FIRSTPRIVATE"([4 x i32]* %b), "QUAL.OMP.FIRSTPRIVATE"(%struct.str2* %c), "QUAL.OMP.PRIVATE"([64 x i8]* %buf), "QUAL.OMP.PRIVATE"([64 x i8]* %buf2), "QUAL.OMP.PRIVATE"([64 x i8]* %buf3) ]
  br label %DIR.OMP.TASK.3

DIR.OMP.TASK.3:                                   ; preds = %DIR.OMP.TASK.13
  %4 = bitcast [64 x i8]* %buf to i8*
  call void @llvm.lifetime.start.p0i8(i64 64, i8* %4) #2
  %5 = bitcast [64 x i8]* %buf2 to i8*
  call void @llvm.lifetime.start.p0i8(i64 64, i8* %5) #2
  %6 = bitcast [64 x i8]* %buf3 to i8*
  call void @llvm.lifetime.start.p0i8(i64 64, i8* %6) #2
  %arraydecay = getelementptr inbounds [64 x i8], [64 x i8]* %buf, i64 0, i64 0
  %7 = bitcast i32* %a to i8*
  %loadrez = load i32, i32* %a, align 4
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 64 %arraydecay, i8* align 64 %7, i64 4, i1 false)
  %arraydecay1 = getelementptr inbounds [64 x i8], [64 x i8]* %buf2, i64 0, i64 0
  %8 = bitcast [4 x i32]* %b to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 64 %arraydecay1, i8* align 64 %8, i64 4, i1 false)
  %arraydecay2 = getelementptr inbounds [64 x i8], [64 x i8]* %buf3, i64 0, i64 0
  %9 = bitcast %struct.str2* %c to i8*
  call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 64 %arraydecay2, i8* align 64 %9, i64 4, i1 false)
  %10 = bitcast [64 x i8]* %buf3 to i8*
  call void @llvm.lifetime.end.p0i8(i64 64, i8* %10) #2
  %11 = bitcast [64 x i8]* %buf2 to i8*
  call void @llvm.lifetime.end.p0i8(i64 64, i8* %11) #2
  %12 = bitcast [64 x i8]* %buf to i8*
  call void @llvm.lifetime.end.p0i8(i64 64, i8* %12) #2
  br label %DIR.OMP.END.TASK.4

DIR.OMP.END.TASK.4:                               ; preds = %DIR.OMP.TASK.3
  call void @llvm.directive.region.exit(token %3) [ "DIR.OMP.END.TASK"() ]
  br label %DIR.OMP.END.TASK.5

DIR.OMP.END.TASK.5:                               ; preds = %DIR.OMP.END.TASK.4
  %13 = bitcast %struct.str2* %c to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %13) #2
  %14 = bitcast [4 x i32]* %b to i8*
  call void @llvm.lifetime.end.p0i8(i64 16, i8* %14) #2
  %15 = bitcast i32* %a to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %15) #2
  ret void
}

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.start.p0i8(i64 immarg, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

; Function Attrs: argmemonly mustprogress nofree nounwind willreturn
declare void @llvm.memcpy.p0i8.p0i8.i64(i8* noalias nocapture writeonly, i8* noalias nocapture readonly, i64, i1 immarg) #3

; Function Attrs: argmemonly mustprogress nocallback nofree nosync nounwind willreturn
declare void @llvm.lifetime.end.p0i8(i64 immarg, i8* nocapture) #1

attributes #0 = { mustprogress nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "frame-pointer"="none" "loopopt-pipeline"="light" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "pre_loopopt" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { argmemonly mustprogress nocallback nofree nosync nounwind willreturn }
attributes #2 = { nounwind }
attributes #3 = { argmemonly mustprogress nofree nounwind willreturn }

!llvm.module.flags = !{!0, !1, !2}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"openmp", i32 50}
!2 = !{i32 7, !"uwtable", i32 2}
