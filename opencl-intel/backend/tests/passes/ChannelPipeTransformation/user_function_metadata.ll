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
; REQUIRES: fpga-emulator
; RUN: %oclopt -runtimelib=%p/../../vectorizer/Full/runtime.bc -channel-pipe-transformation -verify %s -S | FileCheck %s
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024"
target triple = "spir64-unknown-unknown-intelfpga"

%opencl.channel_t = type opaque

@a = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !dbg !0, !packet_size !8, !packet_align !8

; CHECK-DAG: ![[SENDONE_SCOPE:[0-9]+]] = distinct !DISubprogram(name: "sendOne"
; CHECK-DAG: ![[FOO_SCOPE:[0-9]+]] = distinct !DISubprogram(name: "foo"
; CHECK-DAG: ![[CALL_DI:[0-9]+]] = !DILocation(line: 11, scope: ![[FOO_SCOPE]])
;
; CHECK-DAG: define {{.*}} @foo
; CHECK-DAG: call {{.*}} @sendOne{{.*}}, !dbg ![[CALL_DI]]
;
; CHECK-DAG: define {{.*}} @sendOne{{.*}} !dbg ![[SENDONE_SCOPE]]

; Function Attrs: convergent noinline nounwind
define spir_func i32 @sendOne(%opencl.channel_t addrspace(1)* %ch) #0 !dbg !15 {
entry:
  %ch.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  store %opencl.channel_t addrspace(1)* %ch, %opencl.channel_t addrspace(1)** %ch.addr, align 4, !tbaa !20
  call void @llvm.dbg.declare(metadata %opencl.channel_t addrspace(1)** %ch.addr, metadata !19, metadata !DIExpression()), !dbg !23
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %ch.addr, align 4, !dbg !24, !tbaa !20
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %0, i32 1) #4, !dbg !24
  ret i32 1, !dbg !25
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

; Function Attrs: convergent nounwind
define spir_kernel void @foo(i32 addrspace(1)* %data) #3 !dbg !26 !kernel_arg_addr_space !32 !kernel_arg_access_qual !33 !kernel_arg_type !34 !kernel_arg_base_type !34 !kernel_arg_type_qual !35 !kernel_arg_host_accessible !36 !kernel_arg_pipe_depth !37 !kernel_arg_pipe_io !35 !kernel_arg_buffer_location !35 {
entry:
  %data.addr = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %data, i32 addrspace(1)** %data.addr, align 8, !tbaa !38
  call void @llvm.dbg.declare(metadata i32 addrspace(1)** %data.addr, metadata !31, metadata !DIExpression()), !dbg !40
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @a, align 4, !dbg !41, !tbaa !20
  %call = call spir_func i32 @sendOne(%opencl.channel_t addrspace(1)* %0) #4, !dbg !41
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %data.addr, align 8, !dbg !41, !tbaa !38
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %1, i64 0, !dbg !41
  store i32 %call, i32 addrspace(1)* %arrayidx, align 4, !dbg !41, !tbaa !42
  ret void, !dbg !44
}

attributes #0 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "denorms-are-zero"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "uniform-work-group-size"="true" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent }

!llvm.dbg.cu = !{!2}
!llvm.module.flags = !{!9, !10, !11}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!12}
!opencl.spir.version = !{!13}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!14}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !6, line: 2, type: !7, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 7.0.0 ", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "/tmp/<stdin>", directory: "")
!4 = !{}
!5 = !{!0}
!6 = !DIFile(filename: "/tmp/1.cl", directory: "")
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{i32 4}
!9 = !{i32 2, !"Dwarf Version", i32 4}
!10 = !{i32 2, !"Debug Info Version", i32 3}
!11 = !{i32 1, !"wchar_size", i32 4}
!12 = !{i32 1, i32 0}
!13 = !{i32 1, i32 2}
!14 = !{!"clang version 7.0.0 "}
!15 = distinct !DISubprogram(name: "sendOne", scope: !6, file: !6, line: 5, type: !16, isLocal: false, isDefinition: true, scopeLine: 5, flags: DIFlagPrototyped, isOptimized: true, unit: !2, retainedNodes: !18)
!16 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !17)
!17 = !{!7, !7}
!18 = !{!19}
!19 = !DILocalVariable(name: "ch", arg: 1, scope: !15, file: !6, line: 5, type: !7)
!20 = !{!21, !21, i64 0}
!21 = !{!"omnipotent char", !22, i64 0}
!22 = !{!"Simple C/C++ TBAA"}
!23 = !DILocation(line: 5, scope: !15)
!24 = !DILocation(line: 6, scope: !15)
!25 = !DILocation(line: 7, scope: !15)
!26 = distinct !DISubprogram(name: "foo", scope: !6, file: !6, line: 10, type: !27, isLocal: false, isDefinition: true, scopeLine: 10, flags: DIFlagPrototyped, isOptimized: true, unit: !2, retainedNodes: !30)
!27 = !DISubroutineType(cc: DW_CC_LLVM_OpenCLKernel, types: !28)
!28 = !{null, !29}
!29 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!30 = !{!31}
!31 = !DILocalVariable(name: "data", arg: 1, scope: !26, file: !6, line: 10, type: !29)
!32 = !{i32 1}
!33 = !{!"none"}
!34 = !{!"int*"}
!35 = !{!""}
!36 = !{i1 false}
!37 = !{i32 0}
!38 = !{!39, !39, i64 0}
!39 = !{!"any pointer", !21, i64 0}
!40 = !DILocation(line: 10, scope: !26)
!41 = !DILocation(line: 11, scope: !26)
!42 = !{!43, !43, i64 0}
!43 = !{!"int", !21, i64 0}
!44 = !DILocation(line: 12, scope: !26)
