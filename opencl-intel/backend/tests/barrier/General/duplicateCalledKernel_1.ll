; RUN: opt -B-DuplicateCalledKernels -verify -S < %s | FileCheck %s

;;*****************************************************************************
;; This test checks the the DuplicateCalledKernels pass clone a called kernel.
;; The case: kernel "bar" called from kernel "foo". Module contains debug info
;; The expected result:
;;      1. Kernel bar is cloned into a new function "__internal.bar"
;;      2. "__internal.bar" is called from "foo" instead of "bar"
;;      3. Metadata was not changed.
;;*****************************************************************************

;; This test was generated using the following cl code with this command:
;; clang -cc1 -cl-std=CL2.0 -x cl -emit-llvm -debug-info-kind=limited -dwarf-version=4 -O0 -include opencl-c.h -include opencl-c-intel.h -o -
;;
;;__kernel void bar(__global float* a, __global float* b) {
;;  int x = get_local_id(0);
;;  a[x] = b[x];
;;}
;;
;;__kernel void foo(__global float* a, __global float* b) {
;;  bar(a, b);
;;}

; ModuleID = ''
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; CHECK: define void @bar
; Function Attrs: nounwind
define void @bar(float* %a, float* %b) #0 !dbg !4 {
entry:
  %a.addr = alloca float*, align 8
  %b.addr = alloca float*, align 8
  %x = alloca i32, align 4
  store float* %a, float** %a.addr, align 8
  call void @llvm.dbg.declare(metadata float** %a.addr, metadata !18, metadata !19), !dbg !20
  store float* %b, float** %b.addr, align 8
  call void @llvm.dbg.declare(metadata float** %b.addr, metadata !21, metadata !19), !dbg !20
  call void @llvm.dbg.declare(metadata i32* %x, metadata !22, metadata !19), !dbg !24
  %call = call i64 @_Z12get_local_idj(i32 0) #1, !dbg !24
  %conv = trunc i64 %call to i32, !dbg !24
  store i32 %conv, i32* %x, align 4, !dbg !24
  %0 = load i32, i32* %x, align 4, !dbg !25
  %idxprom = sext i32 %0 to i64, !dbg !25
  %1 = load float*, float** %b.addr, align 8, !dbg !25
  %arrayidx = getelementptr inbounds float, float* %1, i64 %idxprom, !dbg !25
  %2 = load float, float* %arrayidx, align 4, !dbg !25
  %3 = load i32, i32* %x, align 4, !dbg !25
  %idxprom1 = sext i32 %3 to i64, !dbg !25
  %4 = load float*, float** %a.addr, align 8, !dbg !25
  %arrayidx2 = getelementptr inbounds float, float* %4, i64 %idxprom1, !dbg !25
  store float %2, float* %arrayidx2, align 4, !dbg !25
  ret void, !dbg !26
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: nounwind readnone
declare i64 @_Z12get_local_idj(i32) #2

; CHECK: define void @foo
; Function Attrs: nounwind
define void @foo(float* %a, float* %b) #0 !dbg !10 {
entry:
  %a.addr = alloca float*, align 8
  %b.addr = alloca float*, align 8
  store float* %a, float** %a.addr, align 8
  call void @llvm.dbg.declare(metadata float** %a.addr, metadata !27, metadata !19), !dbg !28
  store float* %b, float** %b.addr, align 8
  call void @llvm.dbg.declare(metadata float** %b.addr, metadata !29, metadata !19), !dbg !28
  %0 = load float*, float** %a.addr, align 8, !dbg !30
  %1 = load float*, float** %b.addr, align 8, !dbg !30
  call void @bar(float* %0, float* %1), !dbg !30
  ret void, !dbg !31
; CHECK-NOT: call void @bar
; CHECK: call void @__internal.bar
; CHECK-NOT: call void @bar
}

attributes #0 = { nounwind "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { nounwind readnone "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-features"="+mmx,+sse,+sse2" "unsafe-fp-math"="false" "use-soft-float"="false" }

; CHECK: define void @__internal.bar

; CHECK: !llvm.dbg.cu = !{!0}

;;; Check that that debug info metadata for the old function was changed to the
;;; new function, but that there's a new debug metadata for the old function.
; CHECK-DAG: [[SrcMD:![0-9]+]] = distinct !DISubprogram(name: "bar", scope: !11, file: !11, line: 1, type: !12, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
; CHECK-DAG: [[NewMD:![0-9]+]] = distinct !DISubprogram(name: "bar", scope: !11, file: !11, line: 1, type: !12, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)

;;; The following checks that @bar is still a kernel.
; CHECK-DAG: !{void (float*, float*)* @bar}

;;; The following checks that all (include global) metadata was copy correctly.
; CHECK-DAG: !DILocation(line: [[SrcL1:[0-9]+]], scope: [[SrcMD]])
; CHECK-DAG: !DILocation(line: [[SrcL1]], scope: [[NewMD]])

!llvm.dbg.cu = !{!0}
!opencl.kernels = !{!11, !12}
!llvm.module.flags = !{!13, !14}
!opencl.compiler.options = !{!15}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!16}
!opencl.spir.version = !{!16}
!llvm.ident = !{!17}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 3.8.1 ", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2)
!1 = !DIFile(filename: "../<stdin>", directory: "/home/chbessonova/repos_llvm")
!2 = !{}
!3 = !{!4, !10}
!4 = distinct !DISubprogram(name: "bar", scope: !5, file: !5, line: 1, type: !6, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!5 = !DIFile(filename: "../kernelBarrier.cl", directory: "/home/chbessonova/repos_llvm")
!6 = !DISubroutineType(types: !7)
!7 = !{null, !8, !8}
!8 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !9, size: 64, align: 64)
!9 = !DIBasicType(name: "float", size: 32, align: 32, encoding: DW_ATE_float)
!10 = distinct !DISubprogram(name: "foo", scope: !5, file: !5, line: 6, type: !6, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: false, unit: !0, variables: !2)
!11 = !{void (float*, float*)* @bar}
!12 = !{void (float*, float*)* @foo}
!13 = !{i32 2, !"Dwarf Version", i32 4}
!14 = !{i32 2, !"Debug Info Version", i32 3}
!15 = !{!"-g", !"-cl-std=CL2.0"}
!16 = !{i32 2, i32 0}
!17 = !{!"clang version 3.8.1 "}
!18 = !DILocalVariable(name: "a", arg: 1, scope: !4, file: !5, line: 1, type: !8)
!19 = !DIExpression()
!20 = !DILocation(line: 1, scope: !4)
!21 = !DILocalVariable(name: "b", arg: 2, scope: !4, file: !5, line: 1, type: !8)
!22 = !DILocalVariable(name: "x", scope: !4, file: !5, line: 2, type: !23)
!23 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!24 = !DILocation(line: 2, scope: !4)
!25 = !DILocation(line: 3, scope: !4)
!26 = !DILocation(line: 4, scope: !4)
!27 = !DILocalVariable(name: "a", arg: 1, scope: !10, file: !5, line: 6, type: !8)
!28 = !DILocation(line: 6, scope: !10)
!29 = !DILocalVariable(name: "b", arg: 2, scope: !10, file: !5, line: 6, type: !8)
!30 = !DILocation(line: 7, scope: !10)
!31 = !DILocation(line: 8, scope: !10)
