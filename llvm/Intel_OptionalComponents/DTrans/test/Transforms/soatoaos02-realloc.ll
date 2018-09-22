; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:      -debug-only=dtrans-soatoaos-deps \
; RUN:      -passes='require<dtransanalysis>,function(require<soatoaos-approx>)' \
; RUN:      -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN:      2>&1 | FileCheck %s
; RUN: opt < %s -whole-program-assume -disable-output \
; RUN:      -debug-only=dtrans-soatoaos-deps \
; RUN:      -passes='require<dtransanalysis>,function(require<soatoaos-approx>)' \
; RUN:      -dtrans-malloc-functions=struct.Mem,0 -dtrans-free-functions=struct.Mem,1 \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; REQUIRES: asserts
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
define void @_ZN3ArrIPiE7reallocEi(%struct.Arr* %this, i32 %inc) {
entry:
  call void @llvm.dbg.value(metadata %struct.Arr* %this, metadata !13, metadata !DIExpression()), !dbg !14
  %size = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 4
  %tmp = load i32, i32* %size, align 8
  %add = add nsw i32 %tmp, %inc
  %capacity = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 1
  %tmp2 = load i32, i32* %capacity, align 8
  %cmp = icmp sle i32 %add, %tmp2
; Some function  of %inc and 1st and 4th integer fields.
; CHECK: %cmp = icmp sle i32 %add, %tmp2
; CHECK-NEXT: Func(Arg 1)
; CHECK-NEXT:     (Load(GEP(Arg 0)
; CHECK-NEXT:               4))
; CHECK-NEXT:     (Load(GEP(Arg 0)
; CHECK-NEXT:               1))
; CHECK-NEXT: br i1 %cmp, label %if.then, label %if.end
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
  %conv4 = trunc i64 %mul to i32
  %tmp6 = bitcast %struct.Mem* %tmp3 to i8* (%struct.Mem*, i32)***
  %vtable = load i8* (%struct.Mem*, i32)**, i8* (%struct.Mem*, i32)*** %tmp6, align 8
  %vfn = getelementptr inbounds i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vtable, i64 0
  %tmp7 = load i8* (%struct.Mem*, i32)*, i8* (%struct.Mem*, i32)** %vfn, align 8
  %call = call i8* %tmp7(%struct.Mem* %tmp3, i32 %conv4)
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
; CHECK:      Load(Func(Load(GEP(Arg 0)
; CHECK-NEXT:                    3)))
; CHECK-NEXT: %elem_load = load i32*, i32** %arrayidx, align 8
  %elem_load = load i32*, i32** %arrayidx, align 8
  %idxprom10 = sext i32 %i.0 to i64
  %arrayidx11 = getelementptr inbounds i32*, i32** %tmp8, i64 %idxprom10
; Copy some element from array to newly allocated memory.
; CHECK:  %arrayidx11 = getelementptr inbounds i32*, i32** %tmp8, i64 %idxprom10
; CHECK-NEXT: Store(Load(Func(Load(GEP(Arg 0)
; CHECK-NEXT:                          3))))
; CHECK-NEXT:      (Func(Alloc size(Func(Arg 1)
; CHECK-NEXT:                           (Load(GEP(Arg 0)
; CHECK-NEXT:                                     4)))
; CHECK-NEXT:                      (Func(Load(GEP(Arg 0)
; CHECK-NEXT:                                     0))
; CHECK-NEXT:                           (Load(Func(Load(Load(GEP(Arg 0)
; CHECK-NEXT:                                                   0))))))))
; CHECK-NEXT:  store i32* %elem_load, i32** %arrayidx11, align 8
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
  %tmp22 = bitcast %struct.Mem* %tmp19 to void (%struct.Mem*, i8*)***
  %vtable15 = load void (%struct.Mem*, i8*)**, void (%struct.Mem*, i8*)*** %tmp22, align 8
  %vfn16 = getelementptr inbounds void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vtable15, i64 1
  %tmp23 = load void (%struct.Mem*, i8*)*, void (%struct.Mem*, i8*)** %vfn16, align 8
; Deallocate base pointer.
; CHECK:      Free ptr(Func(Load(GEP(Arg 0)
; CHECK-NEXT:                        3)))
; CHECK-NEXT:         (Func(Load(GEP(Arg 0)
; CHECK-NEXT:                        0))
; CHECK-NEXT:              (Load(Func(Load(Load(GEP(Arg 0)
; CHECK-NEXT:                                       0))))))
; CHECK-NEXT: call void %tmp23(%struct.Mem* %tmp19, i8* %tmp21)
  call void %tmp23(%struct.Mem* %tmp19, i8* %tmp21)
  %base17 = getelementptr inbounds %struct.Arr, %struct.Arr* %this, i32 0, i32 3
  store i32** %tmp8, i32*** %base17, align 8
  br label %return

return:                                           ; preds = %for.end, %if.then
  ret void
}

; XCHECK: Deps computed: 26, Queries: 58

declare void @llvm.dbg.value(metadata, metadata, metadata)

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
