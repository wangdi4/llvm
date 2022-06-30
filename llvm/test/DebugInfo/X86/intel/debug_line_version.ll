; RUN: llc -filetype=obj -o - %s | llvm-dwarfdump -debug-line - | FileCheck -check-prefix=CHECK-4 %s
; RUN: llc -filetype=obj -o - %s -debug-line-version=4 | llvm-dwarfdump -debug-line - | FileCheck -check-prefix=CHECK-4 %s
; RUN: llc -filetype=obj -o - %s -debug-line-version=3 | llvm-dwarfdump -debug-line - | FileCheck -check-prefix=CHECK-3 %s
; RUN: llc -filetype=obj -o - %s -debug-line-version=2 | llvm-dwarfdump -debug-line - | FileCheck -check-prefix=CHECK-2 %s
; RUN: sed -e "s#\"Dwarf Version\", i32 4#\"Dwarf Version\", i32 5#g" %s | llc -filetype=obj -o - -debug-line-version=2 | llvm-dwarfdump -debug-line - | FileCheck -check-prefix=CHECK-5 %s
;
; Verify the proper .debug_line version is emitted.
;
; Source Code
; -----------
; test.c:
;     1	int main() {
;     2	  return 0;
;     3	}
;
; Build Commands
; --------------
; $ clang -S -emit-llvm -g test.c
;
; Test
; ----
; CHECK-2: version: 2
; CHECK-3: version: 3
; CHECK-4: version: 4
; CHECK-5: version: 5
;

; ModuleID = 'test.c'
source_filename = "test.c"
target datalayout = "e-m:e-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 !dbg !7 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  ret i32 0, !dbg !11
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="true" "no-frame-pointer-elim-non-leaf" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!3, !4, !5}
!llvm.ident = !{!6}

!0 = distinct !DICompileUnit(language: DW_LANG_C99, file: !1, producer: "clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang a4756e9629cf0ff32f8b7e7f4eadb905f64fe2aa) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 442442c771c8b98228cab1d4d90a936094cdb8a6)", isOptimized: false, runtimeVersion: 0, emissionKind: FullDebug, enums: !2)
!1 = !DIFile(filename: "test.c", directory: "/path/to")
!2 = !{}
!3 = !{i32 2, !"Dwarf Version", i32 4}
!4 = !{i32 2, !"Debug Info Version", i32 3}
!5 = !{i32 1, !"wchar_size", i32 4}
!6 = !{!"clang version 7.0.0 (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-clang a4756e9629cf0ff32f8b7e7f4eadb905f64fe2aa) (ssh://git-amr-2.devtools.intel.com:29418/dpd_icl-llvm 442442c771c8b98228cab1d4d90a936094cdb8a6)"}
!7 = distinct !DISubprogram(name: "main", scope: !1, file: !1, line: 1, type: !8, isLocal: false, isDefinition: true, scopeLine: 1, isOptimized: false, unit: !0, retainedNodes: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{!10}
!10 = !DIBasicType(name: "int", size: 32, encoding: DW_ATE_signed)
!11 = !DILocation(line: 2, column: 3, scope: !7)
