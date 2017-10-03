; RUN: %oclopt -B-ImplicitGlobalIdPass -S %s | FileCheck %s
;
;; This test was generated using the following cl code with this command:
;;
;; ./clang -cc1 -cl-std=CL1.2 -x cl -emit-llvm -triple=spir64-unknown-unknown -debug-info-kind=limited  -O0 -D__OPENCL_C_VERSION__=120 -include opencl-c.h -include opencl-c-intel.h implicitGidPass.cl -I ../include/cclang
;;
;;__kernel void foo()
;;{
;;}
;;
;;

;; This test checks that we have a gid_alloca and a corresponding llvm.dbg.declare
;; (with correct metadata, e.g. DILocation)

; ModuleID = 'implicitGidPass.cl'
source_filename = "implicitGidPass.cl"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: noinline nounwind
define spir_kernel void @foo() #0 !dbg !6 !kernel_arg_addr_space !2 !kernel_arg_access_qual !2 !kernel_arg_type !2 !kernel_arg_base_type !2 !kernel_arg_type_qual !2 {
entry:
; CHECK: %__ocl_dbg_gid0 = alloca i64
; CHECK: call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid0{{.*}}
; CHECK: %__ocl_dbg_gid1 = alloca i64
; CHECK: call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid1{{.*}}
; CHECK: %__ocl_dbg_gid2 = alloca i64
; CHECK: call void @llvm.dbg.declare(metadata i64* %__ocl_dbg_gid2{{.*}}
  ret void, !dbg !10
}

attributes #0 = { noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 4.0.1 ", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "<stdin>", directory: "")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, i32 2}
!5 = !{!"clang version 4.0.1 "}
!6 = distinct !DISubprogram(name: "foo", scope: !7, file: !7, line: 1, type: !8, isLocal: false, isDefinition: true, scopeLine: 2, isOptimized: false, unit: !0, variables: !2)
!7 = !DIFile(filename: "implicitGidPass.cl", directory: "")
!8 = !DISubroutineType(types: !9)
!9 = !{null}
!10 = !DILocation(line: 3, scope: !6)

; CHECK: !{{[0-9]+}} = !DILocalVariable(name: "__ocl_dbg_gid0", {{.*}}
; CHECK: !{{[0-9]+}} = !DILocalVariable(name: "__ocl_dbg_gid1", {{.*}}
; CHECK: !{{[0-9]+}} = !DILocalVariable(name: "__ocl_dbg_gid2", {{.*}}


