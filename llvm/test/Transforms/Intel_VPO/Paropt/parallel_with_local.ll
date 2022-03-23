; RUN: opt -vpo-paropt -S %s | FileCheck %s
; RUN: opt -passes='vpo-paropt' -S %s | FileCheck %s

; #include <stdio.h>
;
; void foo() {
; #pragma omp parallel
;   {
;     int x = 0;
;     printf ("x = %d\n", x);
;   }
; }


; ModuleID = 'par_with_local.ll'
source_filename = "par_with_local.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

@.str = private unnamed_addr constant [8 x i8] c"x = %d\0A\00", align 1
@"@tid.addr" = external global i32

; Function Attrs: noinline nounwind optnone uwtable
define dso_local void @foo() #0 !dbg !8 {
entry:
  %x = alloca i32, align 4
  br label %DIR.OMP.PARALLEL.1, !dbg !11

; CHECK-NOT: %{{[0-9]+}} = call token @llvm.directive.region.entry() {{.*}}
; CHECK-NOT: call void @llvm.directive.region.exit(token %{{[0-9]+}}) {{.*}}

DIR.OMP.PARALLEL.1:                               ; preds = %entry
  %0 = call token @llvm.directive.region.entry() [ "DIR.OMP.PARALLEL"(), "QUAL.OMP.PRIVATE"(i32* %x) ], !dbg !12
; CHECK: call void {{.*}} @__kmpc_fork_call
  br label %DIR.OMP.PARALLEL.11, !dbg !12

DIR.OMP.PARALLEL.11:                              ; preds = %DIR.OMP.PARALLEL.1

; CHECK: call void @llvm.dbg.declare(metadata i32* %x{{.*}}, metadata [[VAR:![0-9]+]]
  call void @llvm.dbg.declare(metadata i32* %x, metadata !14, metadata !DIExpression()), !dbg !12
  store i32 0, i32* %x, align 4, !dbg !12
  %1 = load i32, i32* %x, align 4, !dbg !16
  %call = call i32 (i8*, ...) @printf(i8* getelementptr inbounds ([8 x i8], [8 x i8]* @.str, i32 0, i32 0), i32 %1), !dbg !17
  br label %DIR.OMP.END.PARALLEL.3, !dbg !11

DIR.OMP.END.PARALLEL.3:                           ; preds = %DIR.OMP.PARALLEL.11
  call void @llvm.directive.region.exit(token %0) [ "DIR.OMP.END.PARALLEL"() ], !dbg !12
  br label %DIR.OMP.END.PARALLEL.2, !dbg !18

DIR.OMP.END.PARALLEL.2:                           ; preds = %DIR.OMP.END.PARALLEL.3
  ret void, !dbg !18
}

; Function Attrs: nounwind
declare token @llvm.directive.region.entry() #1

; Function Attrs: nounwind
declare void @llvm.directive.region.exit(token) #1

; Function Attrs: nounwind readnone speculatable
declare void @llvm.dbg.declare(metadata, metadata, metadata) #2

declare dso_local i32 @printf(i8*, ...) #3

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "may-have-openmp-directive"="true" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind }
attributes #2 = { nounwind readnone speculatable }
attributes #3 = { "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.dbg.intel.emit_class_debug_always = !{!6}
!llvm.ident = !{!7}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 7.0.0", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "par_with_local.c", directory: "/export/iusers/gabaabhi/ics/CQ_ws/ompoC/test_20171102/jira_test_files")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"true"}
!7 = !{!"clang version 7.0.0"}
!8 = distinct !DISubprogram(name: "foo", scope: !1, file: !1, line: 3, type: !9, isLocal: false, isDefinition: true, scopeLine: 3, isOptimized: false, unit: !0, retainedNodes: !2)
!9 = !DISubroutineType(types: !10)
!10 = !{null}
!11 = !DILocation(line: 4, column: 9, scope: !8)
!12 = !DILocation(line: 6, column: 9, scope: !13)
!13 = distinct !DILexicalBlock(scope: !8, file: !1, line: 5, column: 3)
; CHECK: [[SUB:![0-9]+]] = distinct !DISubprogram(name: "foo{{.*}}PARALLEL
; CHECK: [[LB1:![0-9]+]] = distinct !DILexicalBlock(scope: [[SUB]], file: !1, line: 5, column: 3)
; CHECK: [[VAR]] = !DILocalVariable(name: "x", scope: [[LB1]],
!14 = !DILocalVariable(name: "x", scope: !13, file: !1, line: 6, type: !15)
!15 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!16 = !DILocation(line: 7, column: 25, scope: !13)
!17 = !DILocation(line: 7, column: 5, scope: !13)
!18 = !DILocation(line: 9, column: 1, scope: !8)
