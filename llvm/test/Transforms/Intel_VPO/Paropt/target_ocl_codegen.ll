; RUN: opt < %s -switch-to-offload=true -vpo-cfg-restructuring -vpo-paropt-prepare -vpo-restore-operands -vpo-cfg-restructuring -vpo-paropt  -S | FileCheck %s
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

@y = external dso_local global i32, align 4
@.str = private unnamed_addr constant [52 x i8] c"After the target region is executed, x = %d y = %d\0A\00", align 1
@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  %x = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  store i32 1, i32* %x, align 4
  br label %DIR.OMP.TARGET.1

DIR.OMP.TARGET.1:                                 ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.MAP.FROM"(i32* @y), "QUAL.OMP.MAP.TO"(i32* %x), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0) ]
  %1 = load i32, i32* %x, align 4
  %add = add nsw i32 %1, 1
  store i32 %add, i32* @y, align 4
  br label %DIR.OMP.END.TARGET.3

DIR.OMP.END.TARGET.3:                             ; preds = %DIR.OMP.TARGET.1
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ]
  %2 = load i32, i32* %x, align 4
  %3 = load i32, i32* @y, align 4
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([52 x i8], [52 x i8]* @.str, i32 0, i32 0), i32 %2, i32 %3)
  ret i32 0
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

declare dso_local i32 @printf(i8*, ...) #2

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!omp_offload.info = !{!0}
!llvm.module.flags = !{!1}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!3}

!0 = !{i32 0, i32 58, i32 -1942677497, !"main", i32 6, i32 0, i32 0}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{}
!3 = !{!"clang version 8.0.0"}

; CHECK: store i32 %{{.*}}, i32 addrspace(1)* %{{.*}}
; CHECK:  !spirv.Source = !{!{{.*}}}
