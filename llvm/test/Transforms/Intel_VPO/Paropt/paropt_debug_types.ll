; RUN: opt -S -switch-to-offload -vpo-paropt %s | FileCheck %s
; RUN: opt -S -switch-to-offload -passes='vpo-paropt' %s | FileCheck %s
;
; Verify type information from different scopes is emitting into the offload
; device code correctly.
;
; // Global declaration used inside parallel region.
; struct S0 { ... };
; int main() {
; #pragma omp target
;   {
;     struct S0 s0;
;   }
; }
;
; // Local declaration outside of parallel region.
; int main() {
;   struct S1 { ... };
; #pragma omp target
;   {
;     struct S1 s1;
;   }
; }
;
; // Local declaration inside lexical block outside of parallel region.
; int main() {
;   { // Lexical block
;     struct S2 { ... };
; #pragma omp target
;     {
;       struct S2 s2;
;     }
;   }
; }
;
; // Local declaration inside of parallel region.
; int main() {
; #pragma omp target
;   {
;     struct S3 { ... };
;     {
;       struct S3 s3;
;     }
;   }
; }
;
; // Local declaration inside lexical block inside of parallel region.
; int main() {
; #pragma omp target
;   {
;     {
;       struct S4 { ... };
;       struct S4 s4;
;     }
;   }
; }
;
; // Local declaration with nested anonymous struct outside parallel region.
; int main() {
;   struct S5 {
;     struct { ... } m;
;   };
; #pragma omp target
;   {
;     struct S5 s5;
;   }
; }
;
; // Local declaration with self-referencing type, outside parallel region.
; int main() {
;   struct S6 {
;     struct S6 *m;
;   };
; #pragma omp target
;   {
;     struct S6 s6;
;   }
; }
;
; // Function call with lambda object operator argument is inlined into offload
; // region and the inlined routine retains the function arguments.
; //    DISubprogram("inlined").retainedNodes = !{ !"f" }
; template<typename F> void inlined(..., F f) { ... }
; int main() {
; #pragma omp for
;   {
;     inlined(..., [=](int i) { ... });
;   }
; }
;
; CHECK: %struct.S6 = type { %struct.S6* }
; CHECK: %struct.S5 = type { %struct.anon }
; CHECK: %struct.S4 = type { double }
; CHECK: %struct.S3 = type { float }
; CHECK: %struct.S2 = type { i64 }
; CHECK: %struct.S1 = type { i16 }
; CHECK: %struct.S0 = type { i32 }
;
; CHECK: define {{.*}} @__omp_offloading_{{.*}}main_l8()
; CHECK-SAME: !dbg [[KERNEL:![0-9]+]]
; CHECK-SAME: {
;
; CHECK-LABEL: newFuncRoot:
; CHECK: [[V7:%[^ ]+]] = alloca i32
; CHECK: [[V6:%[^ ]+]] = alloca %struct.S6
; CHECK: [[V5:%[^ ]+]] = alloca %struct.S5
; CHECK: [[V4:%[^ ]+]] = alloca %struct.S4
; CHECK: [[V3:%[^ ]+]] = alloca %struct.S3
; CHECK: [[V2:%[^ ]+]] = alloca %struct.S2
; CHECK: [[V1:%[^ ]+]] = alloca %struct.S1
; CHECK: [[V0:%[^ ]+]] = alloca %struct.S0
;
; CHECK-LABEL: DIR.OMP.TARGET{{.*}}:
; CHECK: call void @llvm.dbg.declare(metadata %struct.S6* [[V6]],
; CHECK-SAME: metadata [[S6:![0-9]+]],
; CHECK-SAME: metadata !DIExpression())
; CHECK: call void @llvm.dbg.declare(metadata %struct.S5* [[V5]],
; CHECK-SAME: metadata [[S5:![0-9]+]],
; CHECK-SAME: metadata !DIExpression())
; CHECK: call void @llvm.dbg.declare(metadata %struct.S4* [[V4]],
; CHECK-SAME: metadata [[S4:![0-9]+]],
; CHECK-SAME: metadata !DIExpression())
; CHECK: call void @llvm.dbg.declare(metadata %struct.S3* [[V3]],
; CHECK-SAME: metadata [[S3:![0-9]+]],
; CHECK-SAME: metadata !DIExpression())
; CHECK: call void @llvm.dbg.declare(metadata %struct.S2* [[V2]],
; CHECK-SAME: metadata [[S2:![0-9]+]],
; CHECK-SAME: metadata !DIExpression())
; CHECK: call void @llvm.dbg.declare(metadata %struct.S1* [[V1]],
; CHECK-SAME: metadata [[S1:![0-9]+]],
; CHECK-SAME: metadata !DIExpression())
; CHECK: call void @llvm.dbg.declare(metadata %struct.S0* [[V0]],
; CHECK-SAME: metadata [[S0:![0-9]+]],
; CHECK-SAME: metadata !DIExpression())
;
; CHECK: [[T7:%[^ ]+]] = load i32, i32* [[V7]]
; CHECK: call void @llvm.dbg.value(metadata i32 [[T7]],
; CHECK-SAME: metadata [[S7:![0-9]+]]
; CHECK-SAME: metadata !DIExpression())
;
; CHECK: }
;
; CHECK: [[SP:![0-9]+]] ={{.*}}!DISubprogram(name: "main.DIR.OMP.TARGET.{{.*}}"
; CHECK: [[LB1:![0-9]+]] = distinct !DILexicalBlock(scope: [[LB2:![0-9]+]]{{.*}}, line: 8, column: 1)
; CHECK: [[LB2:![0-9]+]] = distinct !DILexicalBlock(scope: [[SP]]{{.*}}, line: 5, column: 3)
;
; CHECK: [[S6]] = !DILocalVariable(name: "s6"
; CHECK-SAME: scope: [[LB1]]
; CHECK-SAME: type: [[T6:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[T6]] = distinct !DICompositeType(tag: DW_TAG_structure_type
; CHECK-SAME: name: "S6"
; CHECK-SAME: scope: [[SP]]
; CHECK-SAME: elements: [[E6:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[E6]] = !{[[M6:![0-9]+]]}
; CHECK: [[M6]] = !DIDerivedType(tag: DW_TAG_member
; CHECK-SAME: name: "m"
; CHECK-SAME: scope: [[T6]]
; CHECK-SAME: baseType: [[P6:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[P6]] = !DIDerivedType(tag: DW_TAG_pointer_type
; CHECK-SAME: baseType: [[T6]]
; CHECK-SAME: )
;
; CHECK: [[S5]] = !DILocalVariable(name: "s5"
; CHECK-SAME: scope: [[LB1]]
; CHECK-SAME: type: [[T5:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[T5]] = distinct !DICompositeType(tag: DW_TAG_structure_type
; CHECK-SAME: name: "S5"
; CHECK-SAME: scope: [[SP]]
; CHECK-SAME: elements: [[E5:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[E5]] = !{[[M5:![0-9]+]]}
; CHECK: [[M5]] = !DIDerivedType(tag: DW_TAG_member
; CHECK-SAME: name: "m"
; CHECK-SAME: scope: [[T5]]
; CHECK-SAME: baseType: [[NT5:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[NT5]] = distinct !DICompositeType(tag: DW_TAG_structure_type
; CHECK-SAME: scope: [[T5]]
; CHECK-SAME: elements: [[NE5:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[NE5]] = !{[[NN5:![0-9]+]]}
; CHECK: [[NN5]] = !DIDerivedType(tag: DW_TAG_member, name: "n"
; CHECK-SAME: scope: [[NT5]]
; CHECK-SAME: )
;
; CHECK: [[S4]] = !DILocalVariable(name: "s4"
; CHECK-SAME: scope: [[LB1]]
; CHECK-SAME: type: [[T4:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[T4]] = distinct !DICompositeType(tag: DW_TAG_structure_type
; CHECK-SAME: name: "S4"
; ==> FIXME: struct S4 is expected to have a scope. Broken in CFE.
; CHECK-NOT:  scope:
; CHECK-SAME: )
;
; CHECK: [[S3]] = !DILocalVariable(name: "s3"
; CHECK-SAME: scope: [[LB1]]
; CHECK-SAME: type: [[T3:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[T3]] = distinct !DICompositeType(tag: DW_TAG_structure_type
; CHECK-SAME: name: "S3"
; ==> FIXME: struct S3 is expected to have a scope. Broken in CFE.
; CHECK-NOT:  scope:
; CHECK-SAME: )
;
; CHECK: [[S2]] = !DILocalVariable(name: "s2"
; CHECK-SAME: scope: [[LB1]]
; CHECK-SAME: type: [[T2:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[T2]] = distinct !DICompositeType(tag: DW_TAG_structure_type
; CHECK-SAME: name: "S2"
; CHECK-SAME: scope: [[SP]]
; CHECK-SAME: )
;
; CHECK: [[S1]] = !DILocalVariable(name: "s1"
; CHECK-SAME: scope: [[LB1]]
; CHECK-SAME: type: [[T1:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[T1]] = distinct !DICompositeType(tag: DW_TAG_structure_type
; CHECK-SAME: name: "S1"
; CHECK-SAME: scope: [[SP]]
; CHECK-SAME: )
;
; CHECK: [[S0]] = !DILocalVariable(name: "s0"
; CHECK-SAME: scope: [[LB1]]
; CHECK-SAME: type: [[T0:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[T0]] = distinct !DICompositeType(tag: DW_TAG_structure_type
; CHECK-SAME: name: "S0"
; CHECK-NOT:  scope:
; CHECK-SAME: )
;
; CHECK: [[S7]] = !DILocalVariable(name: "f"
; CHECK-SAME: arg: 3
; CHECK-SAME: scope: [[FORALL:![0-9]+]]
; CHECK-SAME: type: [[T7:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[FORALL]] = distinct !DISubprogram(name: "forall<...>"
; CHECK-SAME: retainedNodes: [[R7:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[R7]] = !{[[S7]]}
; CHECK: [[T7]] = distinct !DICompositeType(tag: DW_TAG_class_type
; CHECK-SAME: elements: [[E7:![0-9]+]]
; CHECK-SAME: )
; CHECK: [[E7]] = !{[[T7M1:![0-9]+]], [[T7M2:![0-9]+]]}
; CHECK: [[T7M1]] = !DIDerivedType(tag: DW_TAG_member
; CHECK-SAME: name: "t"
; CHECK-SAME: scope: [[T7]]
; CHECK-SAME: )
; CHECK: [[T7M2]] = !DISubprogram(name: "operator()"
; CHECK-SAME: scope: [[T7]]
; CHECK-SAME: )
;
; -----------------------------------------------------------------------------
; ModuleID = '<stdin>'
source_filename = "test.cpp"
target datalayout = "e-i64:64-v16:16-v24:32-v32:32-v48:64-v96:128-v192:256-v256:256-v512:512-v1024:1024-n8:16:32:64"
target triple = "spir64"
target device_triples = "spir64"

%struct.S0 = type { i32 }
%struct.S1 = type { i16 }
%struct.S2 = type { i64 }
%struct.S3 = type { float }
%struct.S4 = type { double }
%struct.S5 = type { %struct.anon }
%struct.anon = type { i32 }
%struct.S6 = type { %struct.S6* }

@"@tid.addr" = external global i32

; Function Attrs: convergent mustprogress noinline norecurse nounwind optnone
define hidden i32 @main(i32 %argc, i8 addrspace(4)* addrspace(4)* %argv) #0 !dbg !10 {
entry:
  %retval = alloca i32, align 4
  %argc.addr = alloca i32, align 4
  %argv.addr = alloca i8 addrspace(4)* addrspace(4)*, align 8
  %i = alloca i32, align 4
  %s0 = alloca %struct.S0, align 4
  %s1 = alloca %struct.S1, align 2
  %s2 = alloca %struct.S2, align 8
  %s3 = alloca %struct.S3, align 4
  %s4 = alloca %struct.S4, align 8
  %s5 = alloca %struct.S5, align 8
  %s6 = alloca %struct.S6, align 8
  %s7 = alloca i32, align 4
  %retval.ascast = addrspacecast i32* %retval to i32 addrspace(4)*
  %argc.addr.ascast = addrspacecast i32* %argc.addr to i32 addrspace(4)*
  %argv.addr.ascast = addrspacecast i8 addrspace(4)* addrspace(4)** %argv.addr to i8 addrspace(4)* addrspace(4)* addrspace(4)*
  %i.ascast = addrspacecast i32* %i to i32 addrspace(4)*
  %s0.ascast = addrspacecast %struct.S0* %s0 to %struct.S0 addrspace(4)*
  %s1.ascast = addrspacecast %struct.S1* %s1 to %struct.S1 addrspace(4)*
  %s2.ascast = addrspacecast %struct.S2* %s2 to %struct.S2 addrspace(4)*
  %s3.ascast = addrspacecast %struct.S3* %s3 to %struct.S3 addrspace(4)*
  %s4.ascast = addrspacecast %struct.S4* %s4 to %struct.S4 addrspace(4)*
  %s5.ascast = addrspacecast %struct.S5* %s5 to %struct.S5 addrspace(4)*
  %s6.ascast = addrspacecast %struct.S6* %s6 to %struct.S6 addrspace(4)*
  %s7.ascast = addrspacecast i32* %s7 to i32 addrspace(4)*
  store i32 0, i32 addrspace(4)* %retval.ascast, align 4
  store i32 %argc, i32 addrspace(4)* %argc.addr.ascast, align 4
  call void @llvm.dbg.declare(metadata i32* %argc.addr, metadata !19, metadata !DIExpression()), !dbg !20
  store i8 addrspace(4)* addrspace(4)* %argv, i8 addrspace(4)* addrspace(4)* addrspace(4)* %argv.addr.ascast, align 8
  call void @llvm.dbg.declare(metadata i8 addrspace(4)* addrspace(4)** %argv.addr, metadata !21, metadata !DIExpression()), !dbg !22
  br label %DIR.OMP.TARGET.1, !dbg !22

DIR.OMP.TARGET.1:                                 ; preds = %entry
  br label %DIR.OMP.TARGET.2, !dbg !23

DIR.OMP.TARGET.2:                                 ; preds = %DIR.OMP.TARGET.1
  br label %DIR.OMP.TARGET.15, !dbg !23

DIR.OMP.TARGET.15:                                ; preds = %DIR.OMP.TARGET.2
  br label %DIR.OMP.TARGET.15.split, !dbg !23

DIR.OMP.TARGET.15.split:                          ; preds = %DIR.OMP.TARGET.15
  %end.dir.temp = alloca i1, align 1, !dbg !23
  br label %DIR.OMP.TARGET.112, !dbg !23

DIR.OMP.TARGET.112:                               ; preds = %DIR.OMP.TARGET.15.split
  br label %DIR.OMP.TARGET.213, !dbg !23

DIR.OMP.TARGET.213:                               ; preds = %DIR.OMP.TARGET.112
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE:WILOCAL"(i32 addrspace(4)* %i.ascast), "QUAL.OMP.PRIVATE:WILOCAL"(%struct.S0 addrspace(4)* %s0.ascast), "QUAL.OMP.PRIVATE:WILOCAL"(%struct.S1 addrspace(4)* %s1.ascast), "QUAL.OMP.PRIVATE:WILOCAL"(%struct.S2 addrspace(4)* %s2.ascast), "QUAL.OMP.PRIVATE:WILOCAL"(%struct.S3 addrspace(4)* %s3.ascast), "QUAL.OMP.PRIVATE:WILOCAL"(%struct.S4 addrspace(4)* %s4.ascast), "QUAL.OMP.PRIVATE:WILOCAL"(%struct.S5 addrspace(4)* %s5.ascast), "QUAL.OMP.PRIVATE:WILOCAL"(%struct.S6 addrspace(4)* %s6.ascast), "QUAL.OMP.PRIVATE:WILOCAL"(i32 addrspace(4)* %s7.ascast), "QUAL.OMP.JUMP.TO.END.IF"(i1* %end.dir.temp) ], !dbg !23
  br label %DIR.OMP.TARGET.314, !dbg !23

DIR.OMP.TARGET.314:                               ; preds = %DIR.OMP.TARGET.213
  %temp.load = load volatile i1, i1* %end.dir.temp, align 1, !dbg !23
  %cmp = icmp ne i1 %temp.load, false, !dbg !23
  br i1 %cmp, label %DIR.OMP.END.TARGET.4.split, label %1, !dbg !23

1:                                                ; preds = %DIR.OMP.TARGET.314
  br label %DIR.OMP.TARGET.3, !dbg !23

DIR.OMP.TARGET.3:                                 ; preds = %1
  call void @llvm.dbg.declare(metadata i32* %i, metadata !26, metadata !DIExpression()), !dbg !28
  call void @llvm.dbg.declare(metadata %struct.S0* %s0, metadata !29, metadata !DIExpression()), !dbg !35
  call void @llvm.dbg.declare(metadata %struct.S1* %s1, metadata !36, metadata !DIExpression()), !dbg !41
  call void @llvm.dbg.declare(metadata %struct.S2* %s2, metadata !42, metadata !DIExpression()), !dbg !47
  call void @llvm.dbg.declare(metadata %struct.S3* %s3, metadata !48, metadata !DIExpression()), !dbg !53
  call void @llvm.dbg.declare(metadata %struct.S4* %s4, metadata !54, metadata !DIExpression()), !dbg !59
  call void @llvm.dbg.declare(metadata %struct.S5* %s5, metadata !60, metadata !DIExpression()), !dbg !67
  call void @llvm.dbg.declare(metadata %struct.S6* %s6, metadata !68, metadata !DIExpression()), !dbg !88
  %s7.local = load i32, i32 addrspace(4)* %s7.ascast, align 4
  call void @llvm.dbg.value(metadata i32 %s7.local, metadata !73, metadata !DIExpression()), !dbg !89
  %m = getelementptr inbounds %struct.S0, %struct.S0 addrspace(4)* %s0.ascast, i32 0, i32 0, !dbg !90
  store i32 1234, i32 addrspace(4)* %m, align 4, !dbg !88
  %m1 = getelementptr inbounds %struct.S1, %struct.S1 addrspace(4)* %s1.ascast, i32 0, i32 0, !dbg !85
  store i16 42, i16 addrspace(4)* %m1, align 2, !dbg !86
  %m2 = getelementptr inbounds %struct.S2, %struct.S2 addrspace(4)* %s2.ascast, i32 0, i32 0, !dbg !87
  store i64 12345678, i64 addrspace(4)* %m2, align 8, !dbg !88
  %m3 = getelementptr inbounds %struct.S3, %struct.S3 addrspace(4)* %s3.ascast, i32 0, i32 0, !dbg !90
  store float 0x4010CCCCC0000000, float addrspace(4)* %m3, align 4, !dbg !85
  %m4 = getelementptr inbounds %struct.S4, %struct.S4 addrspace(4)* %s4.ascast, i32 0, i32 0, !dbg !86
  store double 0x400921FB54411744, double addrspace(4)* %m4, align 8, !dbg !87
  %m5 = getelementptr inbounds %struct.S5, %struct.S5 addrspace(4)* %s5.ascast, i32 0, i32 0
  %n5 = getelementptr inbounds %struct.anon, %struct.anon addrspace(4)* %m5, i32 0, i32 0
  store i32 42, i32 addrspace(4)* %n5, align 4
  %m6 = getelementptr inbounds %struct.S6, %struct.S6 addrspace(4)* %s6.ascast, i32 0, i32 0
  store %struct.S6* null, %struct.S6* addrspace(4)* %m6, align 8
  store i32 %s7.local, i32 addrspace(4)* %s7.ascast, align 4
  store i32 0, i32 addrspace(4)* %i.ascast, align 4, !dbg !88
  br label %DIR.OMP.END.TARGET.4, !dbg !88

DIR.OMP.END.TARGET.4:                             ; preds = %DIR.OMP.TARGET.3
  br label %DIR.OMP.END.TARGET.4.split, !dbg !23

DIR.OMP.END.TARGET.4.split:                       ; preds = %DIR.OMP.END.TARGET.4, %DIR.OMP.TARGET.314
  br label %DIR.OMP.END.TARGET.415, !dbg !23

DIR.OMP.END.TARGET.415:                           ; preds = %DIR.OMP.END.TARGET.4.split
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.TARGET"() ], !dbg !23
  br label %DIR.OMP.END.TARGET.5, !dbg !23

DIR.OMP.END.TARGET.5:                             ; preds = %DIR.OMP.END.TARGET.415
  ret i32 0, !dbg !90
}

; Function Attrs: nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

; Function Attrs: mustprogress nofree nosync nounwind readnone speculatable willreturn
declare void @llvm.dbg.value(metadata %0, metadata %1, metadata %2) #1

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #2

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #2

attributes #0 = { convergent mustprogress noinline norecurse nounwind optnone "approx-func-fp-math"="true" "contains-openmp-target"="true" "frame-pointer"="all" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "unsafe-fp-math"="true" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
attributes #2 = { nounwind }

!llvm.dbg.cu = !{!0}
!omp_offload.info = !{!2}
!llvm.module.flags = !{!3, !4, !5, !6, !7, !8, !9}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus_14, file: !1, producer: "clang", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug)
!1 = !DIFile(filename: "test.cpp", directory: "/path/to")
!2 = !{i32 0, i32 2055, i32 68816954, !"_Z4main", i32 8, i32 0, i32 0}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{i32 7, !"openmp", i32 50}
!7 = !{i32 7, !"openmp-device", i32 50}
!8 = !{i32 7, !"PIC Level", i32 2}
!9 = !{i32 7, !"frame-pointer", i32 2}
!10 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 3, type: !11, scopeLine: 3, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !18)
!11 = !DISubroutineType(types: !12)
!12 = !{!13, !13, !14}
!13 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!14 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !15, size: 64)
!15 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !16, size: 64)
!16 = !DIDerivedType(tag: DW_TAG_const_type, baseType: !17)
!17 = !DIBasicType(name: "char", size: 8, encoding: DW_ATE_signed_char)
!18 = !{}
!19 = !DILocalVariable(name: "argc", arg: 1, scope: !10, file: !1, line: 3, type: !13)
!20 = !DILocation(line: 3, column: 14, scope: !10)
!21 = !DILocalVariable(name: "argv", arg: 2, scope: !10, file: !1, line: 3, type: !14)
!22 = !DILocation(line: 3, column: 32, scope: !10)
!23 = !DILocation(line: 8, column: 1, scope: !24)
!24 = distinct !DILexicalBlock(scope: !25, file: !1, line: 8, column: 1)
!25 = distinct !DILexicalBlock(scope: !10, file: !1, line: 5, column: 3)
!26 = !DILocalVariable(name: "i", scope: !27, file: !1, line: 11, type: !13)
!27 = distinct !DILexicalBlock(scope: !24, file: !1, line: 9, column: 5)
!28 = !DILocation(line: 11, column: 11, scope: !27)
!29 = !DILocalVariable(name: "s0", scope: !30, file: !1, line: 15, type: !31)
!30 = distinct !DILexicalBlock(scope: !27, file: !1, line: 12, column: 7)
!31 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "S0", file: !1, line: 1, size: 32, flags: DIFlagTypePassByValue, elements: !32, identifier: "_ZTS2S0")
!32 = !{!33}
!33 = !DIDerivedType(tag: DW_TAG_member, name: "m", scope: !31, file: !1, line: 1, baseType: !34, size: 32)
!34 = !DIBasicType(name: "unsigned int", size: 32, encoding: DW_ATE_unsigned)
!35 = !DILocation(line: 15, column: 19, scope: !30)
!36 = !DILocalVariable(name: "s1", scope: !30, file: !1, line: 16, type: !37)
!37 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "S1", scope: !10, file: !1, line: 4, size: 16, flags: DIFlagTypePassByValue, elements: !38)
!38 = !{!39}
!39 = !DIDerivedType(tag: DW_TAG_member, name: "m", scope: !37, file: !1, line: 4, baseType: !40, size: 16)
!40 = !DIBasicType(name: "short", size: 16, encoding: DW_ATE_signed)
!41 = !DILocation(line: 16, column: 19, scope: !30)
!42 = !DILocalVariable(name: "s2", scope: !30, file: !1, line: 17, type: !43)
!43 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "S2", scope: !10, file: !1, line: 6, size: 64, flags: DIFlagTypePassByValue, elements: !44)
!44 = !{!45}
!45 = !DIDerivedType(tag: DW_TAG_member, name: "m", scope: !43, file: !1, line: 6, baseType: !46, size: 64)
!46 = !DIBasicType(name: "long", size: 64, encoding: DW_ATE_signed)
!47 = !DILocation(line: 17, column: 19, scope: !30)
!48 = !DILocalVariable(name: "s3", scope: !30, file: !1, line: 18, type: !49)
!49 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "S3", file: !1, line: 10, size: 32, flags: DIFlagTypePassByValue, elements: !50)
!50 = !{!51}
!51 = !DIDerivedType(tag: DW_TAG_member, name: "m", scope: !49, file: !1, line: 10, baseType: !52, size: 32)
!52 = !DIBasicType(name: "float", size: 32, encoding: DW_ATE_float)
!53 = !DILocation(line: 18, column: 19, scope: !30)
!54 = !DILocalVariable(name: "s4", scope: !30, file: !1, line: 19, type: !55)
!55 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "S4", file: !1, line: 13, size: 64, flags: DIFlagTypePassByValue, elements: !56)
!56 = !{!57}
!57 = !DIDerivedType(tag: DW_TAG_member, name: "m", scope: !55, file: !1, line: 13, baseType: !58, size: 64)
!58 = !DIBasicType(name: "double", size: 64, encoding: DW_ATE_float)
!59 = !DILocation(line: 19, column: 19, scope: !30)
!60 = !DILocalVariable(name: "s5", scope: !30, file: !1, line: 13, type: !61)
!61 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "S5", scope: !10, file: !1, line: 14, size: 32, flags: DIFlagTypePassByValue, elements: !62)
!62 = !{!63}
!63 = !DIDerivedType(tag: DW_TAG_member, name: "m", scope: !61, file: !1, line: 17, baseType: !64, size: 32)
!64 = distinct !DICompositeType(tag: DW_TAG_structure_type, scope: !61, file: !1, line: 15, size: 32, flags: DIFlagTypePassByValue, elements: !65)
!65 = !{!66}
!66 = !DIDerivedType(tag: DW_TAG_member, name: "n", scope: !64, file: !1, line: 16, baseType: !13, size: 32)
!67 = !DILocation(line: 20, column: 12, scope: !30)
!68 = !DILocalVariable(name: "s6", scope: !30, file: !1, line: 14, type: !69)
!69 = distinct !DICompositeType(tag: DW_TAG_structure_type, name: "S6", scope: !10, file: !1, line: 18, size: 64, flags: DIFlagTypePassByValue, elements: !70)
!70 = !{!71}
!71 = !DIDerivedType(tag: DW_TAG_member, name: "m", scope: !69, file: !1, line: 19, baseType: !72, size: 64)
!72 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !69, size: 64)

!73 = !DILocalVariable(name: "f", arg: 3, scope: !91, file: !1, line: 17, type: !74)
!74 = distinct !DICompositeType(tag: DW_TAG_class_type, name: "_ZTSZ8timestepiEUliE_", file: !1, line: 34, size: 32, flags: DIFlagTypePassByValue | DIFlagNonTrivial, elements: !75)
!75 = !{!76, !77}
!76 = !DIDerivedType(tag: DW_TAG_member, name: "t", scope: !74, file: !1, line: 35, baseType: !13, size: 32)
!77 = !DISubprogram(name: "operator()", linkageName: "_ZZ8timestepiENKUliE_clEi", scope: !74, file: !1, line: 34, type: !92, scopeLine: 34, flags: DIFlagPublic | DIFlagPrototyped, spFlags: DISPFlagLocalToUnit | DISPFlagOptimized)
!78 = !DILocation(line: 21, column: 14, scope: !30)
!79 = !DILocation(line: 21, column: 12, scope: !30)
!80 = !DILocation(line: 22, column: 12, scope: !30)
!81 = !DILocation(line: 22, column: 14, scope: !30)
!82 = !DILocation(line: 23, column: 12, scope: !30)
!83 = !DILocation(line: 23, column: 14, scope: !30)
!84 = !DILocation(line: 24, column: 12, scope: !30)
!85 = !DILocation(line: 24, column: 14, scope: !30)
!86 = !DILocation(line: 25, column: 12, scope: !30)
!87 = !DILocation(line: 25, column: 14, scope: !30)
!88 = distinct !DILocation(line: 26, column: 11, scope: !30)
!89 = !DILocation(line: 27, column: 15, scope: !91, inlinedAt: !88)
!90 = !DILocation(line: 31, column: 3, scope: !10)
!91 = distinct !DISubprogram(name: "forall<...>", linkageName: "_Z6forallIiZ8timestepiEUliE_EvT_S1_T0_", scope: !1, file: !1, line: 21, type: !92, scopeLine: 21, flags: DIFlagPrototyped | DIFlagAllCallsDescribed, spFlags: DISPFlagLocalToUnit | DISPFlagDefinition | DISPFlagOptimized, unit: !0, retainedNodes: !94)
!92 = !DISubroutineType(types: !93)
!93 = !{null}
!94 = !{!73}
