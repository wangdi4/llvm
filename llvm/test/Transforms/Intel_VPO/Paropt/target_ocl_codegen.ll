; RUN: opt < %s -switch-to-offload=true -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
;
; It tests whether the kernel is generate in the form of OpenCL form.
;
; #include<stdio.h>
; int y=2;
; main()
; {
; int x=1;
;    #pragma omp target map(from:y) map(to:x)
;    y = x + 1; // The copy of y on the device has a value of 2.
;    printf("After the target region is executed, x = %d y = %d\n", x, y);
;    return 0;
; }
;
target triple = "spir64"
target device_triples = "spir64"

@y = global i32 2, align 4
@.str = private unnamed_addr constant [52 x i8] c"After the target region is executed, x = %d y = %d\0A\00", align 1
@"@tid.addr" = external global i32

; Function Attrs: nounwind uwtable
define i32 @main() #0 {
entry:
  %x = alloca i32, align 4
  %0 = bitcast i32* %x to i8*
  call void @llvm.lifetime.start.p0i8(i64 4, i8* %0) #2
  store i32 1, i32* %x, align 4, !tbaa !4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %1 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.MAP.FROM"(i32* @y), "QUAL.OMP.MAP.TO"(i32* %x) ]
  %2 = load i32, i32* %x, align 4, !tbaa !4
  %add = add nsw i32 %2, 1
  store i32 %add, i32* @y, align 4, !tbaa !4
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %DIR.OMP.TARGET.1
  call void @llvm.directive.region.exit(token %1) [ "DIR.OMP.END.TARGET"() ]
  %3 = load i32, i32* %x, align 4, !tbaa !4
  %4 = load i32, i32* @y, align 4, !tbaa !4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([52 x i8], [52 x i8]* @.str, i32 0, i32 0), i32 %3, i32 %4)
  call void @llvm.lifetime.end.p0i8(i64 4, i8* %0) #2
  ret i32 0
}

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0i8(i64, i8* nocapture) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

declare i32 @printf(i8*, ...) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0i8(i64, i8* nocapture) #1

attributes #0 = { nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { argmemonly nounwind }
attributes #2 = { nounwind }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0, !1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 7, !"PIC Level", i32 2}
!2 = !{}
!3 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e2fbab09687d3e8e93ab718ffbf01812c0f41264) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 5fdb09623b02fda9c7aa1122cad8a42de58282b1)"}
!4 = !{!5, !5, i64 0}
!5 = !{!"int", !6, i64 0}
!6 = !{!"omnipotent char", !7, i64 0}
!7 = !{!"Simple C/C++ TBAA"}

; CHECK: store i32 %{{.*}}, i32 addrspace(1)* %{{.*}}
; CHECK:  !spirv.Source = !{!{{.*}}}
