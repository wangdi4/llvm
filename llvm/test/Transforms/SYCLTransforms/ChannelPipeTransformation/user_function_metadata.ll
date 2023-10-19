; Compiled from:
; ----------------------------------------------------
; #pragma OPENCL EXTENSION cl_intel_channels: enable
; channel int a;
;
; __attribute__((noinline))
; int sendOne(channel int ch) {
;   write_channel_intel(ch, 1);
;   return 1;
; }
;
; __kernel void foo(__global int *data) {
;   data[0] = sendOne(a);
; }
; ----------------------------------------------------
; Compile options: -cc1 -emit-llvm -triple spir64-unknown-unknown-intelfpga -disable-llvm-passes -x cl -debug-info-kind=limited -dwarf-version=4
; ----------------------------------------------------

; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S -enable-debugify -disable-output 2>&1 | FileCheck -check-prefix=DEBUGIFY %s
; RUN: opt -sycl-kernel-builtin-lib=%p/../Inputs/fpga-pipes.rtl -passes=sycl-kernel-channel-pipe-transformation %s -S | FileCheck %s

target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64-unknown-unknown-intelfpga"

@a = addrspace(1) global ptr addrspace(1) null, align 4, !dbg !0, !packet_size !7, !packet_align !7

; CHECK-DAG: define {{.*}} @foo
; CHECK-DAG: call {{.*}} @sendOne{{.*}}, !dbg ![[CALL_DI:[0-9]+]]
; CHECK-DAG: define {{.*}} @sendOne{{.*}} !dbg ![[SENDONE_SCOPE:[0-9]+]]

; CHECK-DAG: ![[FOO_SCOPE:[0-9]+]] = distinct !DISubprogram(name: "foo"
; CHECK-DAG: ![[CALL_DI]] = !DILocation(line: 11, {{.*}}, scope: ![[FOO_SCOPE]])
; CHECK-DAG: ![[SENDONE_SCOPE]] = distinct !DISubprogram(name: "sendOne"

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #0

; Function Attrs: convergent norecurse nounwind
define dso_local void @foo(ptr addrspace(1) noundef align 4 %data) #1 !dbg !14 !kernel_arg_addr_space !20 !kernel_arg_access_qual !21 !kernel_arg_type !22 !kernel_arg_base_type !22 !kernel_arg_type_qual !23 !kernel_arg_host_accessible !24 !kernel_arg_pipe_depth !25 !kernel_arg_pipe_io !23 !kernel_arg_buffer_location !23 !arg_type_null_val !26 {
entry:
  %data.addr = alloca ptr addrspace(1), align 8
  store ptr addrspace(1) %data, ptr %data.addr, align 8, !tbaa !27
  call void @llvm.dbg.declare(metadata ptr %data.addr, metadata !19, metadata !DIExpression(DW_OP_constu, 0, DW_OP_swap, DW_OP_xderef)), !dbg !31
  %0 = load ptr addrspace(1), ptr addrspace(1) @a, align 4, !dbg !32, !tbaa !33
  %call = call i32 @sendOne(ptr addrspace(1) %0) #4, !dbg !34
  %1 = load ptr addrspace(1), ptr %data.addr, align 8, !dbg !35, !tbaa !27
  %arrayidx = getelementptr inbounds i32, ptr addrspace(1) %1, i64 0, !dbg !35
  store i32 %call, ptr addrspace(1) %arrayidx, align 4, !dbg !36, !tbaa !37
  ret void, !dbg !39
}

; Function Attrs: convergent noinline norecurse nounwind
define dso_local i32 @sendOne(ptr addrspace(1) %ch) #2 !dbg !40 !arg_type_null_val !45 {
entry:
  %ch.addr = alloca ptr addrspace(1), align 4
  store ptr addrspace(1) %ch, ptr %ch.addr, align 4, !tbaa !33
  call void @llvm.dbg.declare(metadata ptr %ch.addr, metadata !44, metadata !DIExpression(DW_OP_constu, 1, DW_OP_swap, DW_OP_xderef)), !dbg !46
  %0 = load ptr addrspace(1), ptr %ch.addr, align 4, !dbg !47, !tbaa !33
  call void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1) %0, i32 noundef 1) #4, !dbg !48
  ret i32 1, !dbg !49
}

; Function Attrs: convergent nounwind
declare void @_Z19write_channel_intel11ocl_channelii(ptr addrspace(1), i32 noundef) #3

attributes #0 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #1 = { convergent norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" }
attributes #2 = { convergent noinline norecurse nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { convergent nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #4 = { convergent nounwind }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!8, !9}
!opencl.ocl.version = !{!10}
!opencl.spir.version = !{!10}
!opencl.compiler.options = !{!11}
!llvm.ident = !{!12}
!sycl.kernels = !{!13}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression(DW_OP_constu, 1, DW_OP_swap, DW_OP_xderef))
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !5, line: 2, type: !6, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_OpenCL, file: !3, producer: "clang based Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, globals: !4, splitDebugInlining: false, nameTableKind: None)
!3 = !DIFile(filename: "<stdin>", directory: "")
!4 = !{!0}
!5 = !DIFile(filename: "1.cl", directory: "")
!6 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!7 = !{i32 4}
!8 = !{i32 7, !"Dwarf Version", i32 4}
!9 = !{i32 2, !"Debug Info Version", i32 3}
!10 = !{i32 1, i32 2}
!11 = !{}
!12 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2024.0.0 (2024.x.0.YYYYMMDD)"}
!13 = !{ptr @foo}
!14 = distinct !DISubprogram(name: "foo", scope: !5, file: !5, line: 10, type: !15, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !18)
!15 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !16)
!16 = !{null, !17}
!17 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !6, size: 64, dwarfAddressSpace: 1)
!18 = !{!19}
!19 = !DILocalVariable(name: "data", arg: 1, scope: !14, file: !5, line: 10, type: !17)
!20 = !{i32 1}
!21 = !{!"none"}
!22 = !{!"int*"}
!23 = !{!""}
!24 = !{i1 false}
!25 = !{i32 0}
!26 = !{ptr addrspace(1) null}
!27 = !{!28, !28, i64 0}
!28 = !{!"any pointer", !29, i64 0}
!29 = !{!"omnipotent char", !30, i64 0}
!30 = !{!"Simple C/C++ TBAA"}
!31 = !DILocation(line: 10, column: 34, scope: !14)
!32 = !DILocation(line: 11, column: 22, scope: !14)
!33 = !{!29, !29, i64 0}
!34 = !DILocation(line: 11, column: 14, scope: !14)
!35 = !DILocation(line: 11, column: 4, scope: !14)
!36 = !DILocation(line: 11, column: 12, scope: !14)
!37 = !{!38, !38, i64 0}
!38 = !{!"int", !29, i64 0}
!39 = !DILocation(line: 12, column: 2, scope: !14)
!40 = distinct !DISubprogram(name: "sendOne", scope: !5, file: !5, line: 5, type: !41, scopeLine: 5, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition | DISPFlagOptimized, unit: !2, retainedNodes: !43)
!41 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !42)
!42 = !{!6, !6}
!43 = !{!44}
!44 = !DILocalVariable(name: "ch", arg: 1, scope: !40, file: !5, line: 5, type: !6)
!45 = !{target("spirv.Channel") zeroinitializer}
!46 = !DILocation(line: 5, column: 26, scope: !40)
!47 = !DILocation(line: 6, column: 24, scope: !40)
!48 = !DILocation(line: 6, column: 4, scope: !40)
!49 = !DILocation(line: 7, column: 4, scope: !40)

; DEBUGIFY-NOT: WARNING: Missing line
