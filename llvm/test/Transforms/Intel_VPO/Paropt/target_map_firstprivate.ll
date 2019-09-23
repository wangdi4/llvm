; RUN: opt < %s -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
; RUN: opt < %s -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt'  -S | FileCheck %s
;
; Check whether the compiler generates the correct map type.
;
; int main(int argc, char** argv) {
;   int N = 10;
;   int result = 5;
;   #pragma omp target map(from:result) firstprivate(N)
;   {
;      result = N;
;   }
;   if (result != N)
;     { printf("failed\n"); return 1; }
;   printf("passed\n");
;   return 0;
; }
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "spir64"

@.str = private unnamed_addr constant [8 x i8] c"failed\0A\00", align 1
@.str.1 = private unnamed_addr constant [8 x i8] c"passed\0A\00", align 1
@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32 %argc, i8** %argv) #0 {
entry:
  %N = alloca i32, align 4
  %result = alloca i32, align 4
  %0 = bitcast i32* %N to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  store i32 10, i32* %N, align 4, !tbaa !3
  %1 = bitcast i32* %result to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %1) #2
  store i32 5, i32* %result, align 4, !tbaa !3
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %2 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.MAP.FROM"(i32* %result), "QUAL.OMP.FIRSTPRIVATE"(i32* %N) ]
  %3 = load i32, i32* %N, align 4, !tbaa !3
  store i32 %3, i32* %result, align 4, !tbaa !3
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %DIR.OMP.TARGET.1
  call void @llvm.directive.region.exit(token %2) [ "DIR.OMP.END.TARGET"() ]
  %4 = load i32, i32* %result, align 4, !tbaa !3
  %5 = load i32, i32* %N, align 4, !tbaa !3
  %cmp = icmp ne i32 %4, %5
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %DIR.OMP.END.TARGET.3
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0))
  br label %cleanup

if.end:                                           ; preds = %DIR.OMP.END.TARGET.3
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str.1, i32 0, i32 0))
  br label %cleanup

cleanup:                                          ; preds = %if.end, %if.then
  %retval.0 = phi i32 [ 1, %if.then ], [ 0, %if.end ]
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %1) #2
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %0) #2
  ret i32 %retval.0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare dso_local i32 @printf(i8*, ...) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 59, i32 -1943003064, !"main", i32 5, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
; CHECK: @.offload_maptypes = private unnamed_addr constant [2 x i64] [i64 34, i64 33]
