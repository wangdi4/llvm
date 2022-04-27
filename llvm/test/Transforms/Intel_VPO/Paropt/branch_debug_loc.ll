; RUN: opt -vpo-cfg-restructuring -S %s 2>&1 | FileCheck %s
; RUN: opt -passes='function(vpo-cfg-restructuring)' -S %s 2>&1 | FileCheck %s
;
; Test src:
; void foo() {
;   #pragma omp parallel
;     ;
; }
;
; Check that the branch instruction created during CFGRestructuring gets its debug
; info from the predecessor instruction
; CHECK: call void @llvm.directive.region.exit(token {{.*}}) [ "DIR.OMP.END.PARALLEL"() ], !dbg [[EXITDBG:![0-9]+]]
; CHECK-NEXT:   br label %DIR.OMP.END.PARALLEL{{[^, ]*}}, !dbg [[EXITDBG]]

target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 !dbg !7 {
entry:
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"() ], !dbg !10
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ], !dbg !10
  ret void, !dbg !11
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 9.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2, splitDebugInlining: false, nameTableKind: None)
!1 = !DIFile(filename: "test.c", directory: "/full/path/of/test/directory")
!2 = !{}
!3 = !{i32 7, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 9.0.0"}
!7 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 1, type: !8, scopeLine: 1, spFlags: DISPFlagDefinition, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{null}
!10 = !DILocation(line: 2, column: 3, scope: !7)
!11 = !DILocation(line: 4, column: 1, scope: !7)
