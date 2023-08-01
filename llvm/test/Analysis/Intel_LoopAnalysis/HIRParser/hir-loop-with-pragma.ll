; RUN: opt %s -passes="hir-ssa-deconstruction,print<hir-framework>" -hir-framework-debug=parser -disable-hir-pragma-bailout -disable-output  2>&1 | FileCheck %s

; Check parsing output for the loop
; CHECK:      DO i1 = 0, 99, 1
; CHECK-NEXT:   (%ip)[i1] = i1;
; CHECK-NEXT: END LOOP
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: norecurse nounwind uwtable
define void @foo(ptr nocapture %ip) local_unnamed_addr #0 !dbg !6 {
entry:
  br label %for.body, !dbg !8

for.body:                                         ; preds = %for.body, %entry
  %indvars.iv = phi i64 [ 0, %entry ], [ %indvars.iv.next, %for.body ]
  %arrayidx = getelementptr inbounds i32, ptr %ip, i64 %indvars.iv, !dbg !9
  %0 = trunc i64 %indvars.iv to i32, !dbg !9
  store i32 %0, ptr %arrayidx, align 4, !dbg !9, !tbaa !10
  %indvars.iv.next = add nuw nsw i64 %indvars.iv, 1, !dbg !8
  %exitcond = icmp eq i64 %indvars.iv.next, 100, !dbg !8
  br i1 %exitcond, label %for.end, label %for.body, !dbg !8, !llvm.loop !14

for.end:                                          ; preds = %for.body
  ret void, !dbg !15
}

attributes #0 = { norecurse nounwind uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3}
!llvm.dbg.intel.emit_class_debug_always = !{!4}
!llvm.ident = !{!5}

!0 = distinct !DICompileUnit(language: DW_LANG_C89, file: !1, producer: "clang version 4.0.0 (trunk 20470) (llvm/branches/loopopt 20580)", isOptimized: true, runtimeVersion: 0, emissionKind: NoDebug, enums: !2)
!1 = !DIFile(filename: "t1.c", directory: "/export/iusers/sguggill/work")
!2 = !{}
!3 = !{i32 2, !"Debug Info Version", i32 3}
!4 = !{!"true"}
!5 = !{!"clang version 4.0.0 (trunk 20470) (llvm/branches/loopopt 20580)"}
!6 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !7, isLocal: false, isDefinition: true, scopeLine: 2, flags: DIFlagPrototyped, isOptimized: true, unit: !0, retainedNodes: !2)
!7 = !DISubroutineType(types: !2)
!8 = !DILocation(line: 5, scope: !6)
!9 = !DILocation(line: 6, scope: !6)
!10 = !{!11, !11, i64 0}
!11 = !{!"int", !12, i64 0}
!12 = !{!"omnipotent char", !13, i64 0}
!13 = !{!"Simple C/C++ TBAA"}
!14 = distinct !{!14, !8}
!15 = !DILocation(line: 7, scope: !6)
