; RUN: opt -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring,vpo-paropt-prepare,vpo-restore-operands,vpo-cfg-restructuring),vpo-paropt' -S %s | FileCheck %s
;
; Check whether the compiler generates the correct code for firstprivate global.
;
; int pvtPtr;
; int main() {
;   scanf("%d", &pvtPtr);
;   int out;
;
;   #pragma omp target firstprivate(pvtPtr) map(from: out)
;   {
;      out = pvtPtr + 1;
;   }
;   printf("%d\n", out);
; }

target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"
target device_triples = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@pvtPtr = common dso_local global i32 0, align 4
@.str.1 = private unnamed_addr constant [4 x i8] c"%d\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local i32 @main() #0 {
entry:
  %out = alloca i32, align 4
  %call = call i32 (i8*, ...) @__isoc99_scanf(i8* getelementptr inbounds ([3 x i8], [3 x i8]* @.str, i32 0, i32 0), i32* @pvtPtr)
  %0 = bitcast i32* %out to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #3
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.FIRSTPRIVATE"(i32* @pvtPtr), "QUAL.OMP.MAP.FROM"(i32* %out, i32* %out, i64 4, i64 34, i8* null, i8* null) ]
  %2 = load i32, i32* @pvtPtr, align 4, !tbaa !3
  %add = add nsw i32 %2, 1
  store i32 %add, i32* %out, align 4, !tbaa !3
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  %3 = load i32, i32* %out, align 4, !tbaa !3
  %call1 = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([4 x i8], [4 x i8]* @.str.1, i32 0, i32 0), i32 %3)
  %4 = bitcast i32* %out to i8*
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %4) #3
  ret i32 0
}

declare dso_local i32 @__isoc99_scanf(i8*, ...) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #2

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #3

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #3

declare dso_local i32 @printf(i8*, ...) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #2

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #2 = { argmemonly nounwind }
attributes #3 = { nounwind }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!llvm.ident = !{!2}

!0 = !{i32 0, i32 59, i32 -1945271149, !"main", i32 8, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{!"clang version 8.0.0"}
!3 = !{!4, !4, i64 0}
!4 = !{!"int", !5, i64 0}
!5 = !{!"omnipotent char", !6, i64 0}
!6 = !{!"Simple C/C++ TBAA"}
; CHECK: %pvtPtr.fpriv = alloca i32
