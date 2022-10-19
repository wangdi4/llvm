; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-TY %s
; RUN: opt < %s -dtransop-allow-typed-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-OP %s
; RUN: opt < %s -opaque-pointers -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

%struct.Arr = type <{ %struct.Mem*, i32, [4 x i8], i32**, i32, [4 x i8] }>
%struct.Mem = type { i32 (...)** }

; This test checks various approximations for side effects in realloc-like function
; in SOA-to-AOS.
;   void realloc(int inc) {
;     if (size + inc <= capacity)
;       return;
;
;     S *new_base = (S *)mem->allocate((size + inc) * sizeof(S));
;     capacity = size + inc;
;     for (int i = 0; i < size; ++i) {
;       new_base[i] = base[i];
;     }
;     mem->deallocate(base);
;     base = new_base;
;   }
; Check that approximations work as expected.
; CHECK-WF-NOT: ; {{.*}}Unknown{{.*}}Dep
; There should be no unknown GEP
; CHECK-WF-NOT: ; Func(GEP
define void @_ZN3ArrIPiE7reallocEi(%struct.Arr* "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !28 {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr* %this, metadata !13, metadata !DIExpression()), !dbg !14
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp = load i32, i32* %size, align 8
  %add = add nsw i32 %tmp, %inc
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  %tmp2 = load i32, i32* %capacity, align 8
  %cmp = icmp sle i32 %add, %tmp2
; Some function  of %inc and 1st and 4th integer fields.
; CHECK-TY: %cmp = icmp sle i32 %add, %tmp2
; CHECK-TY-NEXT: Func(Arg 1)
; CHECK-TY-NEXT:     (Load(GEP(Arg 0)
; CHECK-TY-NEXT:               4))
; CHECK-TY-NEXT:     (Load(GEP(Arg 0)
; CHECK-TY-NEXT:               1))
; CHECK-TY-NEXT: br i1 %cmp, label %if.then, label %if.end
; CHECK-OP: %cmp = icmp sle i32 %add, %tmp2
; CHECK-OP-NEXT: Func(Arg 1)
; CHECK-OP-NEXT:     (Load(GEP(Arg 0)
; CHECK-OP-NEXT:               4))
; CHECK-OP-NEXT:     (Load(GEP(Arg 0)
; CHECK-OP-NEXT:               1))
; CHECK-OP-NEXT: br i1 %cmp, label %if.then, label %if.end
  br i1 %cmp, label %if.then, label %if.end

if.then:                                          ; preds = %entry
  br label %return

if.end:                                           ; preds = %entry
  %mem = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  %tmp3 = load %struct.Mem*, %struct.Mem** %mem, align 8
  %size2 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp4 = load i32, i32* %size2, align 8
  %add3 = add nsw i32 %tmp4, %inc
  %conv = sext i32 %add3 to i64
  %mul = mul i64 %conv, 8
  %call = call i8* @_ZN10MemManager8allocateEl(%struct.Mem* %tmp3, i64 %mul)
  %tmp8 = bitcast i8* %call to i32**
  %size5 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp9 = load i32, i32* %size5, align 8
  %add6 = add nsw i32 %tmp9, %inc
  %capacilty7 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  store i32 %add6, i32* %capacilty7, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc12, %for.inc ]
  %size8 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp12 = load i32, i32* %size8, align 8
  %cmp9 = icmp slt i32 %i.0, %tmp12
  br i1 %cmp9, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %base = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp13 = load i32**, i32*** %base, align 8
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds i32*, i32** %tmp13, i64 %idxprom
; Load from base pointer
; CHECK-TY:      Load(Func(Load(GEP(Arg 0)
; CHECK-TY-NEXT:                    3)))
; CHECK-TY-NEXT: %elem_load = load i32*, i32** %arrayidx, align 8
; CHECK-OP:      Load(Func(Load(GEP(Arg 0)
; CHECK-OP-NEXT:                    3)))
; CHECK-OP-NEXT: %elem_load = load ptr, ptr %arrayidx, align 8
  %elem_load = load i32*, i32** %arrayidx, align 8
  %idxprom10 = sext i32 %i.0 to i64
  %arrayidx11 = getelementptr inbounds i32*, i32** %tmp8, i64 %idxprom10
; Copy some element from array to newly allocated memory.
; CHECK-TY:  %arrayidx11 = getelementptr inbounds i32*, i32** %tmp8, i64 %idxprom10
; CHECK-TY-NEXT: Store(Load(Func(Load(GEP(Arg 0)
; CHECK-TY-NEXT:                          3))))
; CHECK-TY-NEXT:      (Func(Alloc size(Func(Arg 1)
; CHECK-TY-NEXT:                           (Load(GEP(Arg 0)
; CHECK-TY-NEXT:                                     4)))
; CHECK-TY-NEXT:                      (Func(Load(GEP(Arg 0)
; CHECK-TY-NEXT:                                     0)))))
; CHECK-TY-NEXT:  store i32* %elem_load, i32** %arrayidx11, align 8
; CHECK-OP:  %arrayidx11 = getelementptr inbounds ptr, ptr %tmp8, i64 %idxprom10
; CHECK-OP-NEXT: Store(Load(Func(Load(GEP(Arg 0)
; CHECK-OP-NEXT:                          3))))
; CHECK-OP-NEXT:      (Func(Alloc size(Func(Arg 1)
; CHECK-OP-NEXT:                           (Load(GEP(Arg 0)
; CHECK-OP-NEXT:                                     4)))
; CHECK-OP-NEXT:                      (Func(Load(GEP(Arg 0)
; CHECK-OP-NEXT:                                     0)))))
; CHECK-OP-NEXT:  store ptr %elem_load, ptr %arrayidx11, align 8
  store i32* %elem_load, i32** %arrayidx11, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc12 = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %mem13 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 0
  %tmp19 = load %struct.Mem*, %struct.Mem** %mem13, align 8
  %base14 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  %tmp20 = load i32**, i32*** %base14, align 8
  %tmp21 = bitcast i32** %tmp20 to i8*
; Deallocate base pointer.
; CHECK-TY:      Free ptr(Func(Load(GEP(Arg 0)
; CHECK-TY-NEXT:                        3)))
; CHECK-TY-NEXT:         (Func(Load(GEP(Arg 0)
; CHECK-TY-NEXT:                        0)))
; CHECK-TY-NEXT: call void @_ZN10MemManager10deallocateEPv(%struct.Mem* %tmp19, i8* %tmp21)
; CHECK-OP:      Free ptr(Func(Load(GEP(Arg 0)
; CHECK-OP-NEXT:                        3)))
; CHECK-OP-NEXT:         (Func(Load(GEP(Arg 0)
; CHECK-OP-NEXT:                        0)))
; CHECK-OP-NEXT: call void @_ZN10MemManager10deallocateEPv(ptr %tmp19, ptr %tmp21)
  call void @_ZN10MemManager10deallocateEPv(%struct.Mem* %tmp19, i8* %tmp21)
  %base17 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  store i32** %tmp8, i32*** %base17, align 8
  br label %return

return:                                           ; preds = %for.end, %if.then
  ret void
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1"  i8* @_ZN10MemManager8allocateEl(%struct.Mem* "intel_dtrans_func_index"="2" nocapture readnone %this, i64 %size) align 2 !intel.dtrans.func.type !32 {
entry:
  %call = call i8* @malloc(i64 %size)
  ret i8* %call
}

define dso_local void @_ZN10MemManager10deallocateEPv(%struct.Mem* "intel_dtrans_func_index"="1" nocapture readnone %this, i8* "intel_dtrans_func_index"="2" %p) align 2 !intel.dtrans.func.type !33 {
entry:
  tail call void @_ZdlPv(i8* %p)
  ret void
}

declare !intel.dtrans.func.type !34 dso_local void @_ZdlPv(i8* "intel_dtrans_func_index"="1")
declare !intel.dtrans.func.type !35 "intel_dtrans_func_index"="1" i8* @malloc(i64) #0

; XCHECK: Deps computed: 23, Queries: 55

declare void @llvm.dbg.value(metadata, metadata, metadata)

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test", directory: ".")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!""}
!8 = distinct !DISubprogram(name: "na", linkageName: "na", scope: !1, file: !1, line: 1, type: !9, isLocal: false, isDefinition: true, scopeLine: 1, flags: DIFlagPrototyped, isOptimized: false, unit: !0, retainedNodes: !2)
; int(void*) type.
!9 = !DISubroutineType(types: !10)
!10 = !{!11, !12}
!11 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!12 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!13 = !DILocalVariable(name: "na", arg: 1, scope: !8, file: !1, line: 1, type: !12)
!14 = !DILocation(line: 1, column: 1, scope: !8)
!15 = !DILocation(line: 1, column: 1, scope: !8)
!16 = !DILocation(line: 1, column: 1, scope: !8)

!intel.dtrans.types = !{!18, !22}

!17 = !{%struct.Mem zeroinitializer, i32 1}
!18 = !{!"S", %struct.Mem zeroinitializer, i32 1, !19}
!19 = !{!20, i32 2}
!20 = !{!"F", i1 true, i32 0, !21}
!21 = !{i32 0, i32 0}
!22 = !{!"S", %struct.Arr zeroinitializer, i32 6, !17, !23, !24, !26, !27, !24}
!23 = !{i32 0, i32 0}
!24 = !{!"A", i32 4, !25}
!25 = !{i8 0, i32 0}
!26 = !{i32 0, i32 2}
!27 = !{i32 0, i32 0}
!28 = distinct !{!29}
!29 = !{%struct.Arr zeroinitializer, i32 1}
!30 = !{i8 0, i32 1}
!31 = !{%struct.Mem zeroinitializer, i32 1}
!32 = distinct !{!30, !31}
!33 = distinct !{!31, !30}
!34 = distinct !{!30}
!35 = distinct !{!30}
