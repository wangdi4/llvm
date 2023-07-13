; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK %s
; RUN: opt < %s -whole-program-assume -intel-libirc-allowed -disable-output \
; RUN:      -debug-only=dtrans-soatoaosop-deps \
; RUN:      -passes='require<dtrans-safetyanalyzer>,require<soatoaosop-approx>' \
; RUN:      2>&1 | FileCheck --check-prefix=CHECK-WF %s
; REQUIRES: asserts

target triple = "x86_64-unknown-linux-gnu"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"

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

%struct.Arr = type <{ ptr, i32, [4 x i8], ptr, i32, [4 x i8] }>
%struct.Mem = type { ptr }

define void @_ZN3ArrIPiE7reallocEi(ptr "intel_dtrans_func_index"="1" %this, i32 %inc) !intel.dtrans.func.type !17 {
entry:
  call void @llvm.dbg.value(metadata ptr %this, metadata !19, metadata !DIExpression()), !dbg !25
  %size = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 4
  %tmp = load i32, ptr %size, align 8
  %add = add nsw i32 %tmp, %inc
  %capacity = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 1
  %tmp2 = load i32, ptr %capacity, align 8
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
  %mem = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 0
  %tmp3 = load ptr, ptr %mem, align 8
  %size2 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 4
  %tmp4 = load i32, ptr %size2, align 8
  %add3 = add nsw i32 %tmp4, %inc
  %conv = sext i32 %add3 to i64
  %mul = mul i64 %conv, 8
  %call = call ptr @_ZN10MemManager8allocateEl(ptr %tmp3, i64 %mul)
  %tmp8 = bitcast ptr %call to ptr
  %size5 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 4
  %tmp9 = load i32, ptr %size5, align 8
  %add6 = add nsw i32 %tmp9, %inc
  %capacilty7 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 1
  store i32 %add6, ptr %capacilty7, align 8
  br label %for.cond

for.cond:                                         ; preds = %for.inc, %if.end
  %i.0 = phi i32 [ 0, %if.end ], [ %inc12, %for.inc ]
  %size8 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 4
  %tmp12 = load i32, ptr %size8, align 8
  %cmp9 = icmp slt i32 %i.0, %tmp12
  br i1 %cmp9, label %for.body, label %for.end

for.body:                                         ; preds = %for.cond
  %base = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
  %tmp13 = load ptr, ptr %base, align 8
  %idxprom = sext i32 %i.0 to i64
  %arrayidx = getelementptr inbounds ptr, ptr %tmp13, i64 %idxprom
; Load from base pointer
; CHECK:      Load(Func(Load(GEP(Arg 0)
; CHECK-NEXT:                    3)))
; CHECK-NEXT: %elem_load = load ptr, ptr %arrayidx, align 8
  %elem_load = load ptr, ptr %arrayidx, align 8
  %idxprom10 = sext i32 %i.0 to i64
  %arrayidx11 = getelementptr inbounds ptr, ptr %tmp8, i64 %idxprom10
; Copy some element from array to newly allocated memory.
; CHECK:  %arrayidx11 = getelementptr inbounds ptr, ptr %tmp8, i64 %idxprom10
; CHECK-NEXT: Store(Load(Func(Load(GEP(Arg 0)
; CHECK-NEXT:                          3))))
; CHECK-NEXT:      (Func(Alloc size(Func(Arg 1)
; CHECK-NEXT:                           (Load(GEP(Arg 0)
; CHECK-NEXT:                                     4)))
; CHECK-NEXT:                      (Func(Load(GEP(Arg 0)
; CHECK-NEXT:                                     0)))))
; CHECK-NEXT:  store ptr %elem_load, ptr %arrayidx11, align 8
  store ptr %elem_load, ptr %arrayidx11, align 8
  br label %for.inc

for.inc:                                          ; preds = %for.body
  %inc12 = add nsw i32 %i.0, 1
  br label %for.cond

for.end:                                          ; preds = %for.cond
  %mem13 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 0
  %tmp19 = load ptr, ptr %mem13, align 8
  %base14 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
  %tmp20 = load ptr, ptr %base14, align 8
  %tmp21 = bitcast ptr %tmp20 to ptr
; Deallocate base pointer.
; CHECK:      Free ptr(Func(Load(GEP(Arg 0)
; CHECK-NEXT:                        3)))
; CHECK-NEXT:         (Func(Load(GEP(Arg 0)
; CHECK-NEXT:                        0)))
; CHECK-NEXT: call void @_ZN10MemManager10deallocateEPv(ptr %tmp19, ptr %tmp21)
  call void @_ZN10MemManager10deallocateEPv(ptr %tmp19, ptr %tmp21)
  %base17 = getelementptr inbounds %struct.Arr, ptr %this, i32 0, i32 3
  store ptr %tmp8, ptr %base17, align 8
  br label %return

return:                                           ; preds = %for.end, %if.then
  ret void
}

define dso_local noalias nonnull "intel_dtrans_func_index"="1" ptr @_ZN10MemManager8allocateEl(ptr nocapture readnone "intel_dtrans_func_index"="2" %this, i64 %size) align 2 !intel.dtrans.func.type !26 {
entry:
  %call = call ptr @malloc(i64 %size)
  ret ptr %call
}

define dso_local void @_ZN10MemManager10deallocateEPv(ptr nocapture readnone "intel_dtrans_func_index"="1" %this, ptr "intel_dtrans_func_index"="2" %p) align 2 !intel.dtrans.func.type !28 {
entry:
  tail call void @_ZdlPv(ptr %p)
  ret void
}

declare !intel.dtrans.func.type !29 dso_local void @_ZdlPv(ptr "intel_dtrans_func_index"="1")

; Function Attrs: allockind("alloc,uninitialized") allocsize(0)
declare !intel.dtrans.func.type !30 "intel_dtrans_func_index"="1" ptr @malloc(i64) #0

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare void @llvm.dbg.value(metadata, metadata, metadata) #1

attributes #0 = { allockind("alloc,uninitialized") allocsize(0) "alloc-family"="malloc" }
attributes #1 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }

; XCHECK: Deps computed: 23, Queries: 55

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}
!intel.dtrans.types = !{!8, !12}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, nameTableKind: None)
!1 = !DIFile(filename: "test", directory: ".")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!""}
!8 = !{!"S", %struct.Mem zeroinitializer, i32 1, !9}
!9 = !{!10, i32 2}
!10 = !{!"F", i1 true, i32 0, !11}
!11 = !{i32 0, i32 0}
!12 = !{!"S", %struct.Arr zeroinitializer, i32 6, !13, !11, !14, !16, !11, !14}
!13 = !{%struct.Mem zeroinitializer, i32 1}
!14 = !{!"A", i32 4, !15}
!15 = !{i8 0, i32 0}
!16 = !{i32 0, i32 2}
!17 = distinct !{!18}
!18 = !{%struct.Arr zeroinitializer, i32 1}
!19 = !DILocalVariable(name: "na", arg: 1, scope: !20, file: !1, line: 1, type: !24)
!20 = distinct !DISubprogram(name: "na", linkageName: "na", scope: !1, file: !1, line: 1, type: !21, scopeLine: 1, flags: DIFlagPrototyped, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!21 = !DISubroutineType(types: !22)
!22 = !{!23, !24}
!23 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!24 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: null, size: 64)
!25 = !DILocation(line: 1, column: 1, scope: !20)
!26 = distinct !{!27, !13}
!27 = !{i8 0, i32 1}
!28 = distinct !{!13, !27}
!29 = distinct !{!27}
!30 = distinct !{!27}
