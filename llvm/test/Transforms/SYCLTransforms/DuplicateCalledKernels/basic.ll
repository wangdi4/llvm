; Compiled from:
; ----------------------------------------------------
; __kernel void bar(__global float* a, __global float* b) {
;   int x = get_local_id(0);
;   a[x] = b[x];
; }
;
; __kernel void foo(__global float* a, __global float* b) {
;   bar(a, b);
; }
; ----------------------------------------------------
; Compile options:
;   -cc1 -cl-std=CL2.0 -x cl -emit-llvm -debug-info-kind=limited -dwarf-version=4 -O2 -disable-llvm-passes -finclude-default-header %s
; Optimizer options:
;   %oclopt -llvm-equalizer -verify %s -S
; ----------------------------------------------------
; RUN: opt -passes=sycl-kernel-duplicate-called-kernels %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -passes=sycl-kernel-duplicate-called-kernels %s -S | FileCheck %s

;
; This test checks the the DuplicateCalledKernels pass clone a called kernel.
; The case: kernel "bar" called from kernel "foo". Module contains debug info
; The expected result:
;      1. Kernel bar is cloned into a new function "bar.clone"
;      2. "bar.clone" is called from "foo" instead of "bar"
;      3. Metadata was not changed.
;

; CHECK: define void @bar
; CHECK: define void @foo
; CHECK-NOT: call void @bar
; CHECK: call void @bar.clone
; CHECK-NOT: call void @bar
; CHECK: define internal void @bar.clone
;
; The following checks that @bar is still a kernel.
; CHECK-DAG: !sycl.kernels = !{![[OCL_KERNELS:[0-9]+]]}
; CHECK-DAG: ![[OCL_KERNELS]] = !{{{.*}}ptr @bar{{.*}}}

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown"

; Function Attrs: convergent nounwind
define void @bar(ptr addrspace(1) %a, ptr addrspace(1) %b) #0 !dbg !9 !kernel_arg_addr_space !20 !kernel_arg_access_qual !21 !kernel_arg_type !22 !kernel_arg_base_type !22 !kernel_arg_type_qual !23 !kernel_arg_host_accessible !24 !kernel_arg_pipe_depth !25 !kernel_arg_pipe_io !23 !kernel_arg_buffer_location !23 {
entry:
  %a.addr = alloca ptr addrspace(1), align 8
  %b.addr = alloca ptr addrspace(1), align 8
  %x = alloca i32, align 4
  store ptr addrspace(1) %a, ptr %a.addr, align 8, !tbaa !26
  call void @llvm.dbg.declare(metadata ptr %a.addr, metadata !16, metadata !DIExpression()), !dbg !30
  store ptr addrspace(1) %b, ptr %b.addr, align 8, !tbaa !26
  call void @llvm.dbg.declare(metadata ptr %b.addr, metadata !17, metadata !DIExpression()), !dbg !30
  call void @llvm.lifetime.start.p0(i64 4, ptr %x) #4, !dbg !31
  call void @llvm.dbg.declare(metadata ptr %x, metadata !18, metadata !DIExpression()), !dbg !31
  %call = call i64 @_Z12get_local_idj(i32 0) #5, !dbg !31
  %conv = trunc i64 %call to i32, !dbg !31
  store i32 %conv, ptr %x, align 4, !dbg !31, !tbaa !32
  %0 = load ptr addrspace(1), ptr %b.addr, align 8, !dbg !34, !tbaa !26
  %1 = load i32, ptr %x, align 4, !dbg !34, !tbaa !32
  %idxprom = sext i32 %1 to i64, !dbg !34
  %arrayidx = getelementptr inbounds float, ptr addrspace(1) %0, i64 %idxprom, !dbg !34
  %2 = load float, ptr addrspace(1) %arrayidx, align 4, !dbg !34, !tbaa !35
  %3 = load ptr addrspace(1), ptr %a.addr, align 8, !dbg !34, !tbaa !26
  %4 = load i32, ptr %x, align 4, !dbg !34, !tbaa !32
  %idxprom1 = sext i32 %4 to i64, !dbg !34
  %arrayidx2 = getelementptr inbounds float, ptr addrspace(1) %3, i64 %idxprom1, !dbg !34
  store float %2, ptr addrspace(1) %arrayidx2, align 4, !dbg !34, !tbaa !35
  call void @llvm.lifetime.end.p0(i64 4, ptr %x) #4, !dbg !37
  ret void, !dbg !37
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.start.p0(i64, ptr nocapture) #2

; Function Attrs: convergent nounwind readnone
declare i64 @_Z12get_local_idj(i32) #3

; Function Attrs: argmemonly nounwind
declare void @llvm.lifetime.end.p0(i64, ptr nocapture) #2

; Function Attrs: convergent nounwind
define void @foo(ptr addrspace(1) %a, ptr addrspace(1) %b) #0 !dbg !38 !kernel_arg_addr_space !20 !kernel_arg_access_qual !21 !kernel_arg_type !22 !kernel_arg_base_type !22 !kernel_arg_type_qual !23 !kernel_arg_host_accessible !24 !kernel_arg_pipe_depth !25 !kernel_arg_pipe_io !23 !kernel_arg_buffer_location !23 {
entry:
  %a.addr = alloca ptr addrspace(1), align 8
  %b.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %a, ptr %a.addr, align 8, !tbaa !26
  call void @llvm.dbg.declare(metadata ptr %a.addr, metadata !40, metadata !DIExpression()), !dbg !42
  store ptr addrspace(1) %b, ptr %b.addr, align 8, !tbaa !26
  call void @llvm.dbg.declare(metadata ptr %b.addr, metadata !41, metadata !DIExpression()), !dbg !42
  %0 = load ptr addrspace(1), ptr %a.addr, align 8, !dbg !43, !tbaa !26
  %1 = load ptr addrspace(1), ptr %b.addr, align 8, !dbg !43, !tbaa !26
  call void @bar(ptr addrspace(1) %0, ptr addrspace(1) %1) #6, !dbg !43
  ret void, !dbg !44
}

attributes #0 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="false" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { argmemonly nounwind }
attributes #3 = { convergent nounwind readnone "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { nounwind }
attributes #5 = { convergent nounwind readnone }
attributes #6 = { convergent "uniform-work-group-size"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!6}
!opencl.spir.version = !{!6}
!opencl.used.extensions = !{!2}
!opencl.used.optional.core.features = !{!2}
!opencl.compiler.options = !{!2}
!llvm.ident = !{!7}
!sycl.kernels = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 8.0.0 ", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "/tmp/<stdin>", directory: "/tmp/tests")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 2, i32 0}
!7 = !{!"clang version 8.0.0 "}
!8 = !{ptr @bar, ptr @foo}
!9 = distinct !DISubprogram(name: "bar", scope: !10, file: !10, line: 1, type: !11, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !15)
!10 = !DIFile(filename: "/tmp/1.cl", directory: "/tmp/tests")
!11 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !12)
!12 = !{null, !13, !13}
!13 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !14, size: 64)
!14 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!15 = !{!16, !17, !18}
!16 = !DILocalVariable(name: "a", arg: 1, scope: !9, file: !10, line: 1, type: !13)
!17 = !DILocalVariable(name: "b", arg: 2, scope: !9, file: !10, line: 1, type: !13)
!18 = !DILocalVariable(name: "x", scope: !9, file: !10, line: 2, type: !19)
!19 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!20 = !{i32 1, i32 1}
!21 = !{!"none", !"none"}
!22 = !{!"ptr", !"ptr"}
!23 = !{!"", !""}
!24 = !{i1 false, i1 false}
!25 = !{i32 0, i32 0}
!26 = !{!27, !27, i64 0}
!27 = !{!"any pointer", !28, i64 0}
!28 = !{!"omnipotent char", !29, i64 0}
!29 = !{!"Simple C/C++ TBAA"}
!30 = !DILocation(line: 1, scope: !9)
!31 = !DILocation(line: 2, scope: !9)
!32 = !{!33, !33, i64 0}
!33 = !{!"int", !28, i64 0}
!34 = !DILocation(line: 3, scope: !9)
!35 = !{!36, !36, i64 0}
!36 = !{!"float", !28, i64 0}
!37 = !DILocation(line: 4, scope: !9)
!38 = distinct !DISubprogram(name: "foo", scope: !10, file: !10, line: 6, type: !11, isLocal: false, isDefinition: true, scopeLine: 6, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !39)
!39 = !{!40, !41}
!40 = !DILocalVariable(name: "a", arg: 1, scope: !38, file: !10, line: 6, type: !13)
!41 = !DILocalVariable(name: "b", arg: 2, scope: !38, file: !10, line: 6, type: !13)
!42 = !DILocation(line: 6, scope: !38)
!43 = !DILocation(line: 7, scope: !38)
!44 = !DILocation(line: 8, scope: !38)

; DEBUGIFY-NOT: WARNING
