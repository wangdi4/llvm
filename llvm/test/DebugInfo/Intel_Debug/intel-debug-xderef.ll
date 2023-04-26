; RUN: llc -filetype=null -mtriple x86_64-unknown-linux-gnu \
; RUN:     --print-after x86-isel < %s 2>&1 \
; RUN:   | FileCheck --check-prefixes=X86-ISEL %s
; RUN: opt -S -passes=intel-debug -mtriple spir64 %s \
; RUN:   | FileCheck --check-prefixes=CHECK,UNMOD %s
; RUN: opt -S -passes=intel-debug -mtriple x86_64-unknown-linux-gnu %s \
; RUN:   | FileCheck --check-prefixes=CHECK,MODIF %s
; RUN: opt -S -passes=intel-debug -mtriple x86_64-unknown-linux-gnu %s \
; RUN:     -enable-intel-debug-remove-xderef=false \
; RUN:   | FileCheck --check-prefixes=CHECK,UNMOD %s
;
; - - test.cpp - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; #define ADDRESS_SPACE_PRIVATE   __attribute__((opencl_private))
; #define ADDRESS_SPACE_GLOBAL    __attribute__((opencl_global))
; #define ADDRESS_SPACE_CONSTANT  __attribute__((opencl_constant))
; #define ADDRESS_SPACE_LOCAL     __attribute__((opencl_local))
; #define ADDRESS_SPACE_GENERIC  /* default */
; 
; void local_ptr(ADDRESS_SPACE_LOCAL int *Arg) {}
; void local_ref(ADDRESS_SPACE_LOCAL int &Arg) {}
; 
; __attribute__((sycl_device)) void test() {
;   ADDRESS_SPACE_PRIVATE  int *ASPRV;
;   ADDRESS_SPACE_GLOBAL   int *ASGLB;
;   ADDRESS_SPACE_CONSTANT int *ASCON;
;   ADDRESS_SPACE_LOCAL    int *ASLOC;
;   ADDRESS_SPACE_GENERIC  int *ASGEN;
;   local_ptr(ASLOC);
;   local_ref(*ASLOC);
; }
; - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
;
; The test below was based on the following IR:
; $ clang-17 -cc1 -triple spir64 -fsycl-is-device -emit-llvm \
;            -debug-info-kind=limited -dwarf-version=5 test.cpp -o -
;
; -----------------------------------------------------------------------------
;
; During X86 instruction selection, the address space casts are removed.
;
; X86-ISEL:      # Machine code for function _Z4testv
; X86-ISEL:      %0:gr64 = MOV64rm %stack.3.ASLOC
; X86-ISEL-NEXT: ADJCALLSTACKDOWN64
; X86-ISEL-NEXT: $rdi = COPY %0:gr64
; X86-ISEL-NEXT: CALL64pcrel32 @_Z9local_ptrPU3AS3i
; X86-ISEL:      %1:gr64 = MOV64rm %stack.3.ASLOC
; X86-ISEL-NEXT: ADJCALLSTACKDOWN64
; X86-ISEL-NEXT: $rdi = COPY %1:gr64
; X86-ISEL-NEXT: CALL64pcrel32 @_Z9local_refRU3AS3i
; X86-ISEL:      # End machine code for function _Z4testv
;
; The intel-debug pass will remove address space casts from the location
; expression metadata for X86 targets in the same way they are removed from
; the IR during X86 instruction selection. Both the MODIFied expressions
; and the UNMODified expressions are checked below.
;
; CHECK:      define {{.*}} void @_Z4testv()
; CHECK-SAME: {
; CHECK:        call void @llvm.dbg.declare(metadata ptr %ASPRV
; UNMOD-SAME:    !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)
; MODIF-SAME:    !DIExpression()
; CHECK:        call void @llvm.dbg.declare(metadata ptr %ASGLB
; UNMOD-SAME:    !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)
; MODIF-SAME:    !DIExpression()
; CHECK:        call void @llvm.dbg.declare(metadata ptr %ASCON
; UNMOD-SAME:    !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)
; MODIF-SAME:    !DIExpression()
; CHECK:        call void @llvm.dbg.declare(metadata ptr %ASLOC
; UNMOD-SAME:    !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)
; MODIF-SAME:    !DIExpression()
; CHECK:        call void @llvm.dbg.declare(metadata ptr %ASGEN
; UNMOD-SAME:    !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)
; MODIF-SAME:    !DIExpression()
; CHECK:        call void @llvm.dbg.value(metadata !{{[0-9]+}}
; UNMOD-SAME:    !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)
; MODIF-SAME:    !DIExpression()
; CHECK:      }
;
; CHECK:      define {{.*}} void @_Z9local_ptrPU3AS3i
; CHECK-SAME: ptr addrspace(3) noundef %Arg
; CHECK-SAME: {
; CHECK:        %Arg.addr = alloca ptr addrspace(3)
; CHECK:        store ptr addrspace(3) %Arg, ptr addrspace(4) %Arg.addr.ascast
; CHECK:        call void @llvm.dbg.declare(metadata ptr %Arg.addr
; UNMOD-SAME:    !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)
; MODIF-SAME:    !DIExpression()
; CHECK:      }
;
; CHECK:      define {{.*}} void @_Z9local_refRU3AS3i
; CHECK-SAME: ptr addrspace(3) noundef align 4 dereferenceable(4) %Arg
; CHECK-SAME: {
; CHECK:        %Arg.addr = alloca ptr addrspace(3)
; CHECK:        store ptr addrspace(3) %Arg, ptr addrspace(4) %Arg.addr.ascast
; CHECK:        call void @llvm.dbg.declare(metadata ptr %Arg.addr
; UNMOD-SAME:    !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)
; MODIF-SAME:    !DIExpression()
; CHECK:      }
;
;
; The address space identifiers on pointers/references are not removed.
; Future improvement? For now they seem to be safely ignored by GDB.
;
; CHECK:      !DIGlobalVariableExpression(var: !{{[0-9]+}}
; UNMOD-SAME: expr: !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)
; MODIF-SAME: expr: !DIExpression()
; CHECK-SAME: )
; CHECK:      [[INT:![0-9]+]] = !DIBasicType(name: "int"
; CHECK:      [[PTR0:![0-9]+]] = !DIDerivedType(tag: DW_TAG_pointer_type,
; CHECK-SAME:    dwarfAddressSpace: 0
; CHECK:      [[PTR1:![0-9]+]] = !DIDerivedType(tag: DW_TAG_pointer_type,
; CHECK-SAME:   baseType: [[INT]]
; CHECK-SAME:   dwarfAddressSpace: 1
; CHECK:      [[PTR2:![0-9]+]] = !DIDerivedType(tag: DW_TAG_pointer_type,
; CHECK-SAME:   baseType: [[INT]]
; CHECK-SAME:   dwarfAddressSpace: 2
; CHECK:      [[PTR3:![0-9]+]] = !DIDerivedType(tag: DW_TAG_pointer_type,
; CHECK-SAME:   baseType: [[INT]]
; CHECK-SAME:   dwarfAddressSpace: 3
; CHECK:      [[PTR4:![0-9]+]] = !DIDerivedType(tag: DW_TAG_pointer_type,
; CHECK-SAME:   baseType: [[INT]]
; CHECK-SAME:   dwarfAddressSpace: 4
; CHECK:      [[SP1:![0-9]+]] = distinct !DISubprogram(name: "local_ptr"
; CHECK:      !DILocalVariable(name: "Arg"
; CHECK-SAME:   arg: 1
; CHECK-SAME:   scope: [[SP1]]
; CHECK-SAME:   type: [[PTR3]]
; CHECK:      [[SP2:![0-9]+]] = distinct !DISubprogram(name: "local_ref"
; CHECK:      [[REF3:![0-9]+]] = !DIDerivedType(tag: DW_TAG_reference_type
; CHECK-SAME:   baseType: [[INT]]
; CHECK-SAME:   dwarfAddressSpace: 3
; CHECK:      !DILocalVariable(name: "Arg"
; CHECK-SAME:   arg: 1
; CHECK-SAME:   scope: [[SP2]]
; CHECK-SAME:   type: [[REF3]]
;
; -----------------------------------------------------------------------------
; ModuleID = 'test.cpp'
source_filename = "test.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define dso_local spir_func void @_Z4testv() #0 !dbg !9 !srcloc !13 {
entry:
  %ASPRV = alloca ptr, align 8
  %ASGLB = alloca ptr addrspace(1), align 8
  %ASCON = alloca ptr addrspace(2), align 8
  %ASLOC = alloca ptr addrspace(3), align 8
  %ASGEN = alloca ptr addrspace(4), align 8
  %ASPRV.ascast = addrspacecast ptr %ASPRV to ptr addrspace(4)
  %ASGLB.ascast = addrspacecast ptr %ASGLB to ptr addrspace(4)
  %ASCON.ascast = addrspacecast ptr %ASCON to ptr addrspace(4)
  %ASLOC.ascast = addrspacecast ptr %ASLOC to ptr addrspace(4)
  %ASGEN.ascast = addrspacecast ptr %ASGEN to ptr addrspace(4)
  call void @llvm.dbg.declare(metadata ptr %ASPRV, metadata !14, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !17
  call void @llvm.dbg.declare(metadata ptr %ASGLB, metadata !18, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !20
  call void @llvm.dbg.declare(metadata ptr %ASCON, metadata !21, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !23
  call void @llvm.dbg.declare(metadata ptr %ASLOC, metadata !24, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !26
  call void @llvm.dbg.declare(metadata ptr %ASGEN, metadata !27, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !29
  %0 = load ptr addrspace(3), ptr addrspace(4) %ASLOC.ascast, align 8, !dbg !30
  call spir_func void @_Z9local_ptrPU3AS3i(ptr addrspace(3) noundef %0) #3, !dbg !31
  %1 = load ptr addrspace(3), ptr addrspace(4) %ASLOC.ascast, align 8, !dbg !32
  call spir_func void @_Z9local_refRU3AS3i(ptr addrspace(3) noundef align 4 dereferenceable(4) %1) #3, !dbg !33
  call void @llvm.dbg.value(metadata !7, metadata !27, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !29
  ret void, !dbg !34
}

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

@global = addrspace(1) constant i32 50, align 4, !dbg !50

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define dso_local spir_func void @_Z9local_ptrPU3AS3i(ptr addrspace(3) noundef %Arg) #2 !dbg !35 !srcloc !38 {
entry:
  %Arg.addr = alloca ptr addrspace(3), align 8
  %Arg.addr.ascast = addrspacecast ptr %Arg.addr to ptr addrspace(4)
  store ptr addrspace(3) %Arg, ptr addrspace(4) %Arg.addr.ascast, align 8
  call void @llvm.dbg.declare(metadata ptr %Arg.addr, metadata !39, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !40
  ret void, !dbg !41
}

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define dso_local spir_func void @_Z9local_refRU3AS3i(ptr addrspace(3) noundef align 4 dereferenceable(4) %Arg) #2 !dbg !42 !srcloc !46 {
entry:
  %Arg.addr = alloca ptr addrspace(3), align 8
  %Arg.addr.ascast = addrspacecast ptr %Arg.addr to ptr addrspace(4)
  store ptr addrspace(3) %Arg, ptr addrspace(4) %Arg.addr.ascast, align 8
  call void @llvm.dbg.declare(metadata ptr %Arg.addr, metadata !47, metadata !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef)), !dbg !48
  ret void, !dbg !49
}

declare dso_local spir_func i32 @_Z18__spirv_ocl_printfPU3AS2Kcz(ptr addrspace(2), ...)

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "no-trapping-math"="true" "stack-protector-buffer-size"="8" "sycl-module-id"="test.cpp" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #2 = { convergent mustprogress noinline norecurse nounwind optnone "no-trapping-math"="true" "stack-protector-buffer-size"="8" }
attributes #3 = { convergent }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!2, !3, !4}
!opencl.spir.version = !{!5}
!spirv.Source = !{!6}
!opencl.compiler.options = !{!7}
!llvm.ident = !{!8}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, splitDebugInlining: false, nameTableKind: None, globals: !52)
!1 = !DIFile(filename: "<stdin>", directory: "/path/to")
!2 = !{i32 7, !"Dwarf Version", i32 5}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{i32 1, !"wchar_size", i32 4}
!5 = !{i32 1, i32 2}
!6 = !{i32 4, i32 100000}
!7 = !{}
!8 = !{!"Intel(R) oneAPI DPC++/C++ Compiler 2023.1.0 (2023.x.0.YYYYMMDD)"}
!9 = distinct !DISubprogram(name: "test", linkageName: "_Z4testv", scope: !10, file: !10, line: 10, type: !11, scopeLine: 10, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !7)
!10 = !DIFile(filename: "test.cpp", directory: "/path/to")
!11 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !12)
!12 = !{null}
!13 = !{i32 453}
!14 = !DILocalVariable(name: "ASPRV", scope: !9, file: !10, line: 11, type: !15)
!15 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !16, size: 64, dwarfAddressSpace: 0)
!16 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!17 = !DILocation(line: 11, column: 31, scope: !9)
!18 = !DILocalVariable(name: "ASGLB", scope: !9, file: !10, line: 12, type: !19)
!19 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !16, size: 64, dwarfAddressSpace: 1)
!20 = !DILocation(line: 12, column: 31, scope: !9)
!21 = !DILocalVariable(name: "ASCON", scope: !9, file: !10, line: 13, type: !22)
!22 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !16, size: 64, dwarfAddressSpace: 2)
!23 = !DILocation(line: 13, column: 31, scope: !9)
!24 = !DILocalVariable(name: "ASLOC", scope: !9, file: !10, line: 14, type: !25)
!25 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !16, size: 64, dwarfAddressSpace: 3)
!26 = !DILocation(line: 14, column: 31, scope: !9)
!27 = !DILocalVariable(name: "ASGEN", scope: !9, file: !10, line: 15, type: !28)
!28 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !16, size: 64, dwarfAddressSpace: 4)
!29 = !DILocation(line: 15, column: 31, scope: !9)
!30 = !DILocation(line: 16, column: 13, scope: !9)
!31 = !DILocation(line: 16, column: 3, scope: !9)
!32 = !DILocation(line: 17, column: 14, scope: !9)
!33 = !DILocation(line: 17, column: 3, scope: !9)
!34 = !DILocation(line: 18, column: 1, scope: !9)
!35 = distinct !DISubprogram(name: "local_ptr", linkageName: "_Z9local_ptrPU3AS3i", scope: !10, file: !10, line: 7, type: !36, scopeLine: 7, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !7)
!36 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !37)
!37 = !{null, !25}
!38 = !{i32 307}
!39 = !DILocalVariable(name: "Arg", arg: 1, scope: !35, file: !10, line: 7, type: !25)
!40 = !DILocation(line: 7, column: 51, scope: !35)
!41 = !DILocation(line: 7, column: 57, scope: !35)
!42 = distinct !DISubprogram(name: "local_ref", linkageName: "_Z9local_refRU3AS3i", scope: !10, file: !10, line: 8, type: !43, scopeLine: 8, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !7)
!43 = !DISubroutineType(cc: DW_CC_LLVM_SpirFunction, types: !44)
!44 = !{null, !45}
!45 = !DIDerivedType(tag: DW_TAG_reference_type, baseType: !16, size: 64, dwarfAddressSpace: 3)
!46 = !{i32 365}
!47 = !DILocalVariable(name: "Arg", arg: 1, scope: !42, file: !10, line: 8, type: !45)
!48 = !DILocation(line: 8, column: 51, scope: !42)
!49 = !DILocation(line: 8, column: 57, scope: !42)
!50 = !DIGlobalVariableExpression(var: !51, expr: !DIExpression(DW_OP_constu, 4, DW_OP_swap, DW_OP_xderef))
!51 = distinct !DIGlobalVariable(name: "global", linkageName: "global", scope: !0, file: !1, line: 12, type: !16, isLocal: true, isDefinition: true)
!52 = !{!50}
