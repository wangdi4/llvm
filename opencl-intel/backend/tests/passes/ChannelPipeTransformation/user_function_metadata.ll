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

@a = common addrspace(1) global %opencl.channel_t addrspace(1)* null, align 4, !dbg !0


; CHECK-DAG: ![[SENDONE_SCOPE:[0-9]+]] = distinct !DISubprogram(name: "sendOne"
; CHECK-DAG: ![[FOO_SCOPE:[0-9]+]] = distinct !DISubprogram(name: "foo"
; CHECK-DAG: ![[CALL_DI:[0-9]+]] = !DILocation(line: 11, scope: ![[FOO_SCOPE]])
;
; CHECK-DAG: define {{.*}} @foo
; CHECK-DAG: call {{.*}} @sendOne{{.*}}, !dbg ![[CALL_DI]]
;
; CHECK-DAG: define {{.*}} @sendOne{{.*}} !dbg ![[SENDONE_SCOPE]]

; Function Attrs: convergent noinline nounwind
define spir_func i32 @sendOne(%opencl.channel_t addrspace(1)* %ch) #0 !dbg !16 {
entry:
  %ch.addr = alloca %opencl.channel_t addrspace(1)*, align 4
  store %opencl.channel_t addrspace(1)* %ch, %opencl.channel_t addrspace(1)** %ch.addr, align 4, !tbaa !21
  call void @llvm.dbg.declare(metadata %opencl.channel_t addrspace(1)** %ch.addr, metadata !20, metadata !DIExpression()), !dbg !24
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)** %ch.addr, align 4, !dbg !25, !tbaa !21
  call void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)* %0, i32 1) #4, !dbg !25
  ret i32 1, !dbg !26
}

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: convergent
declare void @_Z19write_channel_intel11ocl_channelii(%opencl.channel_t addrspace(1)*, i32) #2

; Function Attrs: convergent nounwind
define spir_kernel void @foo(i32 addrspace(1)* %data) #3 !dbg !27 !kernel_arg_addr_space !33 !kernel_arg_access_qual !34 !kernel_arg_type !35 !kernel_arg_base_type !35 !kernel_arg_type_qual !36 !kernel_arg_host_accessible !37 {
entry:
  %data.addr = alloca i32 addrspace(1)*, align 8
  store i32 addrspace(1)* %data, i32 addrspace(1)** %data.addr, align 8, !tbaa !38
  call void @llvm.dbg.declare(metadata i32 addrspace(1)** %data.addr, metadata !32, metadata !DIExpression()), !dbg !40
  %0 = load %opencl.channel_t addrspace(1)*, %opencl.channel_t addrspace(1)* addrspace(1)* @a, align 4, !dbg !41, !tbaa !21
  %call = call spir_func i32 @sendOne(%opencl.channel_t addrspace(1)* %0) #4, !dbg !41
  %1 = load i32 addrspace(1)*, i32 addrspace(1)** %data.addr, align 8, !dbg !41, !tbaa !38
  %arrayidx = getelementptr inbounds i32, i32 addrspace(1)* %1, i64 0, !dbg !41
  store i32 %call, i32 addrspace(1)* %arrayidx, align 4, !dbg !41, !tbaa !42
  ret void, !dbg !44
}

attributes #0 = { convergent noinline nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone speculatable }
attributes #2 = { convergent "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #3 = { convergent nounwind "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #4 = { convergent }

!llvm.dbg.cu = !{!2}
!opencl.channels = !{!8}
!llvm.module.flags = !{!11, !12, !13}
!opencl.enable.FP_CONTRACT = !{}
!opencl.ocl.version = !{!14}
!opencl.spir.version = !{!14}
!opencl.used.extensions = !{!4}
!opencl.used.optional.core.features = !{!4}
!opencl.compiler.options = !{!4}
!llvm.ident = !{!15}

!0 = !DIGlobalVariableExpression(var: !1, expr: !DIExpression())
!1 = distinct !DIGlobalVariable(name: "a", scope: !2, file: !6, line: 2, type: !7, isLocal: false, isDefinition: true)
!2 = distinct !DICompileUnit(language: DW_LANG_C99, file: !3, producer: "clang version 6.0.0 ", isOptimized: true, runtimeVersion: 0, emissionKind: FullDebug, enums: !4, globals: !5)
!3 = !DIFile(filename: "<stdin>", directory: "/data/xmain/ics-ws/opencl/llvm/projects/opencl")
!4 = !{}
!5 = !{!0}
!6 = !DIFile(filename: "temp.cl", directory: "/data/xmain/ics-ws/opencl/llvm/projects/opencl")
!7 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!8 = !{%opencl.channel_t addrspace(1)* addrspace(1)* @a, !9, !10}
!9 = !{!"packet_size", i32 4}
!10 = !{!"packet_align", i32 4}
!11 = !{i32 2, !"Dwarf Version", i32 4}
!12 = !{i32 2, !"Debug Info Version", i32 3}
!13 = !{i32 1, !"wchar_size", i32 4}
!14 = !{i32 2, i32 0}
!15 = !{!"clang version 6.0.0 "}
!16 = distinct !DISubprogram(name: "sendOne", scope: !6, file: !6, line: 5, type: !17, isLocal: false, isDefinition: true, scopeLine: 5, flags: DIFlagPrototyped, isOptimized: true, unit: !2, variables: !19)
!17 = !DISubroutineType(types: !18)
!18 = !{!7, !7}
!19 = !{!20}
!20 = !DILocalVariable(name: "ch", arg: 1, scope: !16, file: !6, line: 5, type: !7)
!21 = !{!22, !22, i64 0}
!22 = !{!"omnipotent char", !23, i64 0}
!23 = !{!"Simple C/C++ TBAA"}
!24 = !DILocation(line: 5, scope: !16)
!25 = !DILocation(line: 6, scope: !16)
!26 = !DILocation(line: 7, scope: !16)
!27 = distinct !DISubprogram(name: "foo", scope: !6, file: !6, line: 10, type: !28, isLocal: false, isDefinition: true, scopeLine: 10, flags: DIFlagPrototyped, isOptimized: true, unit: !2, variables: !31)
!28 = !DISubroutineType(types: !29)
!29 = !{null, !30}
!30 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !7, size: 64)
!31 = !{!32}
!32 = !DILocalVariable(name: "data", arg: 1, scope: !27, file: !6, line: 10, type: !30)
!33 = !{i32 1}
!34 = !{!"none"}
!35 = !{!"int*"}
!36 = !{!""}
!37 = !{i1 false}
!38 = !{!39, !39, i64 0}
!39 = !{!"any pointer", !22, i64 0}
!40 = !DILocation(line: 10, scope: !27)
!41 = !DILocation(line: 11, scope: !27)
!42 = !{!43, !43, i64 0}
!43 = !{!"int", !22, i64 0}
!44 = !DILocation(line: 12, scope: !27)

