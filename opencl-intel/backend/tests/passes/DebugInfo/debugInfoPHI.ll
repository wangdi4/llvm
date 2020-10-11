; RUN: %oclopt -debug-info -verify -S %s | FileCheck %s

;;  -----  debugInfoPHI.cl   -------
;; Steps:
;;   1. clang  -cc1 -cc1 -cl-std=CL2.0 -x cl -emit-llvm -triple=spir64-unknown-unknown -debug-info-kind=limited -O0 -D__OPENCL_C_VERSION__=200 -finclude-default-header -disable-O0-optnone debugInfoPHI.cl -o debugInfoPHITmp.ll
;;   2. oclopt -mem2reg -verify debugInfoPHITmp.ll -S -o debugInfoPHI.ll
;;   3. add debug info for PHI instruction in test2 function (same as ret instruction).
;;
;;int* test2(int **p1, int **p2) {
;;  if (to_global(p1[5]))
;;    return p1[6];
;;  return p2[7];
;;}
;;
;;__kernel void func(__global int *pGlobal, __local int *pLocal, float param) {
;;  int* ptrs1[10];
;;  int* ptrs2[10];
;;  for (int i = 0; i < 10; i++) {
;;    // Initialization
;;    if (i%2) {
;;       ptrs1[i] = pGlobal + i;
;;       ptrs2[i] = pLocal + i;
;;    } else {
;;       ptrs1[i] = pLocal + i;
;;       ptrs2[i] = pGlobal + i;
;;    }
;;  }
;;  int* pGen5 = test2(ptrs1, ptrs2);
;;  __private int* d = to_private(pGen5);
;;}



target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent noinline norecurse nounwind
define spir_func i32 addrspace(4)* @test2(i32 addrspace(4)* addrspace(4)* %p1, i32 addrspace(4)* addrspace(4)* %p2) #0 !dbg !6 {
entry:
  call void @llvm.dbg.value(metadata i32 addrspace(4)* addrspace(4)* %p1, metadata !13, metadata !DIExpression()), !dbg !14
  call void @llvm.dbg.value(metadata i32 addrspace(4)* addrspace(4)* %p2, metadata !15, metadata !DIExpression()), !dbg !14
  %ptridx = getelementptr inbounds i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %p1, i64 5, !dbg !16
  %0 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %ptridx, align 8, !dbg !16
  %1 = bitcast i32 addrspace(4)* %0 to i8 addrspace(4)*, !dbg !16
  %2 = call i8 addrspace(1)* @__to_global(i8 addrspace(4)* %1), !dbg !16
  %3 = bitcast i8 addrspace(1)* %2 to i32 addrspace(1)*, !dbg !16
  %tobool = icmp ne i32 addrspace(1)* %3, null, !dbg !16
  br i1 %tobool, label %if.then, label %if.end, !dbg !18

if.then:                                          ; preds = %entry
  %ptridx1 = getelementptr inbounds i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %p1, i64 6, !dbg !19
  %4 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %ptridx1, align 8, !dbg !19
  br label %return, !dbg !20

if.end:                                           ; preds = %entry
  %ptridx2 = getelementptr inbounds i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %p2, i64 7, !dbg !21
  %5 = load i32 addrspace(4)*, i32 addrspace(4)* addrspace(4)* %ptridx2, align 8, !dbg !21
  br label %return, !dbg !22

return:                                           ; preds = %if.end, %if.then
  %retval.0 = phi i32 addrspace(4)* [ %4, %if.then ], [ %5, %if.end ], !dbg !23

; CHECK:  %retval.0 = phi i32 addrspace(4)* [ %{{.*}}, %if.then ], [ %{{.*}}, %if.end ], !dbg !23
; CHECK-NEXT:  call void @__opencl_dbg_stoppoint(i64 {{.*}}, i64 %{{.*}}, i64 %{{.*}}, i64 %{{.*}})

  ret i32 addrspace(4)* %retval.0, !dbg !23
}

; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare i8 addrspace(1)* @__to_global(i8 addrspace(4)*)


; Function Attrs: nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { convergent noinline norecurse nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="none" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable willreturn }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!4}
!opencl.spir.version = !{!4}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !1, producer: "clang version 11.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "<stdin>", directory: "DebugInfo")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 2, i32 0}
!5 = !{!"clang version 11.0.0"}
!6 = distinct !DISubprogram(name: "test2", scope: !7, file: !7, line: 1, type: !8, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!7 = !DIFile(filename: "debugInfoPHI.cl", directory: "DebugInfo")
!8 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !9)
!9 = !{!10, !12, !12}
!10 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !11, size: 64)
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !10, size: 64)
!13 = !DILocalVariable(name: "p1", arg: 1, scope: !6, file: !7, line: 1, type: !12)
!14 = !DILocation(line: 0, scope: !6)
!15 = !DILocalVariable(name: "p2", arg: 2, scope: !6, file: !7, line: 1, type: !12)
!16 = !DILocation(line: 2, column: 17, scope: !17)
!17 = distinct !DILexicalBlock(scope: !6, file: !7, line: 2, column: 17)
!18 = !DILocation(line: 2, column: 17, scope: !6)
!19 = !DILocation(line: 3, column: 12, scope: !17)
!20 = !DILocation(line: 3, column: 5, scope: !17)
!21 = !DILocation(line: 4, column: 10, scope: !6)
!22 = !DILocation(line: 4, column: 3, scope: !6)
!23 = !DILocation(line: 5, column: 1, scope: !6)
