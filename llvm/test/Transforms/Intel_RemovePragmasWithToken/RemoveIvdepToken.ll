; The source code of the program is below:
;
; __kernel void kernel(__write_only pipe char woPipe)
; {
;    int Array[10];
;    #pragma ivdep array(Array)
;    for (int i = 0; i < 10; ++i) {}
;    int anotherArray[10];
;    #pragma ivdep array(anotherArray)
;    for (int j = 0; j < 10; ++j) {}
; }

; Compile options: clang -cc1 -O0 -triple spir-unknown-unknown-intelfpga test.cl -emit-llvm -o test.ll

; RUN: opt -remove-region-directives -S %s -o %t.ll
; RUN: FileCheck %s --input-file=%t.ll
; REQUIRES: fpga-emulator

%opencl.pipe_wo_t = type opaque

; Function Attrs: convergent noinline nounwind optnone
define spir_kernel void @kernel(ptr addrspace(1) %woPipe) #0 !kernel_arg_addr_space !5 !kernel_arg_access_qual !6 !kernel_arg_type !7 !kernel_arg_base_type !7 !kernel_arg_type_qual !8 !kernel_arg_host_accessible !9 !kernel_arg_pipe_depth !10 !kernel_arg_pipe_io !11 !kernel_arg_buffer_location !11 {
entry:
  %woPipe.addr = alloca ptr addrspace(1), align 4
  %Array = alloca [10 x i32], align 4
  %i = alloca i32, align 4
  %anotherArray = alloca [10 x i32], align 4
  %j = alloca i32, align 4
  store ptr addrspace(1) %woPipe, ptr %woPipe.addr, align 4
  %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"(ptr %Array) ]
; CHECK-LABEL: entry:
; CHECK-NOT: %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"(ptr %Array) ]
  store i32 0, ptr %i, align 4
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %entry
  %1 = load i32, ptr %i, align 4
  %cmp = icmp slt i32 %1, 10
  br i1 %cmp, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %2 = load i32, ptr %i, align 4
  %inc = add nsw i32 %2, 1
  store i32 %inc, ptr %i, align 4
  br label %for.cond

for.end:                                          ; preds = %for.cond
  call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.IVDEP"() ]
; CHECK-LABEL: for.end:
; CHECK-NOT: call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.IVDEP"() ]
  %3 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"(ptr %anotherArray) ]
; CHECK-LABEL: entry:
; CHECK-NOT: %0 = call token @llvm.directive.region.entry() [ "DIR.PRAGMA.IVDEP"(), "QUAL.PRAGMA.ARRAY"(ptr %anotherArray) ]
  store i32 0, ptr %j, align 4
  br label %for.cond1

for.cond1:                                        ; preds = %for.inc4, %for.end
  %4 = load i32, ptr %j, align 4
  %cmp2 = icmp slt i32 %4, 10
  br i1 %cmp2, label %for.body3, label %for.end6

for.body3:                                        ; preds = %for.cond1
  br label %for.inc4

for.inc4:                                         ; preds = %for.body3
  %5 = load i32, ptr %j, align 4
  %inc5 = add nsw i32 %5, 1
  store i32 %inc5, ptr %j, align 4
  br label %for.cond1

for.end6:                                         ; preds = %for.cond1
  call void @llvm.directive.region.exit(token %3) [ "DIR.PRAGMA.END.IVDEP"() ]
; CHECK-LABEL: for.end:
; CHECK-NOT: call void @llvm.directive.region.exit(token %0) [ "DIR.PRAGMA.END.IVDEP"() ]
  ret void
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { convergent noinline nounwind optnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.module.flags = !{!0}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!1}
!opencl.spir.version = !{!2}
!opencl.used.extensions = !{!3}
!opencl.used.optional.core.features = !{!3}
!opencl.compiler.options = !{!3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 1, i32 0}
!2 = !{i32 1, i32 2}
!3 = !{}
!4 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang e2f587278a0121c1832f4bdc5d0d9f1e3bf33511) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 5f0a6f00de61a2fc8abd119e8e8c7e755ec3663e)"}
!5 = !{i32 1}
!6 = !{!"write_only"}
!7 = !{!"char"}
!8 = !{!"pipe"}
!9 = !{i1 false}
!10 = !{i32 0}
!11 = !{!""}
