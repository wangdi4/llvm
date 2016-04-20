; dwarf.ll
;
; Verify the compiler can emit DWARF debug info in COFF object files.
;
; Source Code
; -----------
; dwarf.cpp:
;    1  extern void c();
;    2  extern void d();
;    3  
;    4  void erase(int * erasedp) {
;    5      int * fixp = erasedp;
;    6  
;    7      try {
;    8          c();
;    9      }
;   10      catch(int e) {
;   11      }
;   12  
;   13      int * p = erasedp;
;   14  
;   15      if (p != erasedp) {
;   16          d();
;   17      }
;   18  
;   19      for (;; p = (int *&)(*fixp)) {
;   20          if (fixp == p) {
;   21              fixp = p;
;   22          }
;   23          else {
;   24             return;
;   25          }
;   26      }
;   27  }
;   28  
;   29  void erase_all() {
;   30      int * p = 0;
;   31      while (p != 0) erase(p);
;   32  }
;   33  
;
;
; Build Commands
; --------------
; $ clang -S -emit-llvm -g dwarf.cpp
;
;
; Test
; ----
; RUN: llc -filetype=obj -o - %s | llvm-dwarfdump -debug-dump=info - | FileCheck %s
;
; CHECK: DW_TAG_subprogram
; CHECK:   DW_AT_name{{.*}}erase
; CHECK: DW_TAG_subprogram
; CHECK:   DW_AT_name{{.*}}erase_all
;

; ModuleID = 'dwarf.cpp'
target datalayout = "e-m:w-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-windows-msvc18.0.0"

%rtti.TypeDescriptor2 = type { i8**, i8*, [3 x i8] }

$"\01??_R0H@8" = comdat any

@"\01??_7type_info@@6B@" = external constant i8*
@"\01??_R0H@8" = linkonce_odr global %rtti.TypeDescriptor2 { i8** @"\01??_7type_info@@6B@", i8* null, [3 x i8] c".H\00" }, comdat

; Function Attrs: uwtable
define void @"\01?erase@@YAXPEAH@Z"(i32* %erasedp) #0 personality i8* bitcast (i32 (...)* @__CxxFrameHandler3 to i8*) !dbg !7 {
entry:
  %erasedp.addr = alloca i32*, align 8
  %fixp = alloca i32*, align 8
  %e = alloca i32, align 4
  %p = alloca i32*, align 8
  store i32* %erasedp, i32** %erasedp.addr, align 8
  call void @llvm.dbg.declare(metadata i32** %erasedp.addr, metadata !17, metadata !18), !dbg !19
  call void @llvm.dbg.declare(metadata i32** %fixp, metadata !20, metadata !18), !dbg !21
  %0 = load i32*, i32** %erasedp.addr, align 8, !dbg !22
  store i32* %0, i32** %fixp, align 8, !dbg !21
  invoke void @"\01?c@@YAXXZ"()
          to label %invoke.cont unwind label %catch.dispatch, !dbg !23

catch.dispatch:                                   ; preds = %entry
  %1 = catchswitch within none [label %catch] unwind to caller, !dbg !25

catch:                                            ; preds = %catch.dispatch
  %2 = catchpad within %1 [%rtti.TypeDescriptor2* @"\01??_R0H@8", i32 0, i32* %e], !dbg !26
  call void @llvm.dbg.declare(metadata i32* %e, metadata !28, metadata !18), !dbg !29
  catchret from %2 to label %catchret.dest, !dbg !30

catchret.dest:                                    ; preds = %catch
  br label %try.cont, !dbg !32

try.cont:                                         ; preds = %catchret.dest, %invoke.cont
  call void @llvm.dbg.declare(metadata i32** %p, metadata !34, metadata !18), !dbg !35
  %3 = load i32*, i32** %erasedp.addr, align 8, !dbg !36
  store i32* %3, i32** %p, align 8, !dbg !35
  %4 = load i32*, i32** %p, align 8, !dbg !37
  %5 = load i32*, i32** %erasedp.addr, align 8, !dbg !39
  %cmp = icmp ne i32* %4, %5, !dbg !40
  br i1 %cmp, label %if.then, label %if.end, !dbg !41

if.then:                                          ; preds = %try.cont
  call void @"\01?d@@YAXXZ"(), !dbg !42
  br label %if.end, !dbg !44

invoke.cont:                                      ; preds = %entry
  br label %try.cont, !dbg !45

if.end:                                           ; preds = %if.then, %try.cont
  br label %for.cond, !dbg !47

for.cond:                                         ; preds = %for.inc, %if.end
  %6 = load i32*, i32** %fixp, align 8, !dbg !48
  %7 = load i32*, i32** %p, align 8, !dbg !53
  %cmp1 = icmp eq i32* %6, %7, !dbg !54
  br i1 %cmp1, label %if.then2, label %if.else, !dbg !55

if.then2:                                         ; preds = %for.cond
  %8 = load i32*, i32** %p, align 8, !dbg !56
  store i32* %8, i32** %fixp, align 8, !dbg !58
  br label %if.end3, !dbg !59

if.else:                                          ; preds = %for.cond
  ret void, !dbg !60

if.end3:                                          ; preds = %if.then2
  br label %for.inc, !dbg !62

for.inc:                                          ; preds = %if.end3
  %9 = load i32*, i32** %fixp, align 8, !dbg !63
  %10 = bitcast i32* %9 to i32**, !dbg !65
  %11 = load i32*, i32** %10, align 4, !dbg !65
  store i32* %11, i32** %p, align 8, !dbg !66
  br label %for.cond, !dbg !67
}

; Function Attrs: nounwind readnone
declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

declare void @"\01?c@@YAXXZ"() #2

declare i32 @__CxxFrameHandler3(...)

declare void @"\01?d@@YAXXZ"() #2

; Function Attrs: uwtable
define void @"\01?erase_all@@YAXXZ"() #0 !dbg !10 {
entry:
  %p = alloca i32*, align 8
  call void @llvm.dbg.declare(metadata i32** %p, metadata !68, metadata !18), !dbg !69
  store i32* null, i32** %p, align 8, !dbg !69
  br label %while.cond, !dbg !70

while.cond:                                       ; preds = %while.body, %entry
  %0 = load i32*, i32** %p, align 8, !dbg !71
  %cmp = icmp ne i32* %0, null, !dbg !73
  br i1 %cmp, label %while.body, label %while.end, !dbg !74

while.body:                                       ; preds = %while.cond
  %1 = load i32*, i32** %p, align 8, !dbg !75
  call void @"\01?erase@@YAXPEAH@Z"(i32* %1), !dbg !77
  br label %while.cond, !dbg !78

while.end:                                        ; preds = %while.cond
  ret void, !dbg !79
}

attributes #0 = { uwtable "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }
attributes #1 = { nounwind readnone }
attributes #2 = { "disable-tail-calls"="false" "less-precise-fpmad"="false" "no-frame-pointer-elim"="false" "no-infs-fp-math"="false" "no-nans-fp-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.dbg.cu = !{!0}
!llvm.module.flags = !{!13, !14, !15}
!llvm.ident = !{!16}

!0 = distinct !DICompileUnit(language: DW_LANG_C_plus_plus, file: !1, producer: "clang version 3.9.0 (branches/xmain_web 6799)", isOptimized: false, runtimeVersion: 0, emissionKind: 1, enums: !2, retainedTypes: !3, subprograms: !6)
!1 = !DIFile(filename: "dwarf.cpp", directory: "C:\5Ciusers\5Cbwyma\5Cxmain_web")
!2 = !{}
!3 = !{!4}
!4 = !DIDerivedType(tag: DW_TAG_pointer_type, baseType: !5, size: 64, align: 64)
!5 = !DIBasicType(name: "int", size: 32, align: 32, encoding: DW_ATE_signed)
!6 = !{!7, !10}
!7 = distinct !DISubprogram(name: "erase", linkageName: "\01?erase@@YAXPEAH@Z", scope: !1, file: !1, line: 4, type: !8, isLocal: false, isDefinition: true, scopeLine: 4, flags: DIFlagPrototyped, isOptimized: false, variables: !2)
!8 = !DISubroutineType(types: !9)
!9 = !{null, !4}
!10 = distinct !DISubprogram(name: "erase_all", linkageName: "\01?erase_all@@YAXXZ", scope: !1, file: !1, line: 29, type: !11, isLocal: false, isDefinition: true, scopeLine: 29, flags: DIFlagPrototyped, isOptimized: false, variables: !2)
!11 = !DISubroutineType(types: !12)
!12 = !{null}
!13 = !{i32 2, !"Dwarf Version", i32 4}
!14 = !{i32 2, !"Debug Info Version", i32 3}
!15 = !{i32 1, !"PIC Level", i32 2}
!16 = !{!"clang version 3.9.0 (branches/xmain_web 6799)"}
!17 = !DILocalVariable(name: "erasedp", arg: 1, scope: !7, file: !1, line: 4, type: !4)
!18 = !DIExpression()
!19 = !DILocation(line: 4, column: 18, scope: !7)
!20 = !DILocalVariable(name: "fixp", scope: !7, file: !1, line: 5, type: !4)
!21 = !DILocation(line: 5, column: 11, scope: !7)
!22 = !DILocation(line: 5, column: 18, scope: !7)
!23 = !DILocation(line: 8, column: 9, scope: !24)
!24 = distinct !DILexicalBlock(scope: !7, file: !1, line: 7, column: 9)
!25 = !DILocation(line: 9, column: 5, scope: !24)
!26 = !DILocation(line: 9, column: 5, scope: !27)
!27 = !DILexicalBlockFile(scope: !24, file: !1, discriminator: 1)
!28 = !DILocalVariable(name: "e", scope: !7, file: !1, line: 10, type: !5)
!29 = !DILocation(line: 10, column: 15, scope: !7)
!30 = !DILocation(line: 11, column: 5, scope: !31)
!31 = distinct !DILexicalBlock(scope: !7, file: !1, line: 10, column: 18)
!32 = !DILocation(line: 11, column: 5, scope: !33)
!33 = !DILexicalBlockFile(scope: !31, file: !1, discriminator: 1)
!34 = !DILocalVariable(name: "p", scope: !7, file: !1, line: 13, type: !4)
!35 = !DILocation(line: 13, column: 11, scope: !7)
!36 = !DILocation(line: 13, column: 15, scope: !7)
!37 = !DILocation(line: 15, column: 9, scope: !38)
!38 = distinct !DILexicalBlock(scope: !7, file: !1, line: 15, column: 9)
!39 = !DILocation(line: 15, column: 14, scope: !38)
!40 = !DILocation(line: 15, column: 11, scope: !38)
!41 = !DILocation(line: 15, column: 9, scope: !7)
!42 = !DILocation(line: 16, column: 9, scope: !43)
!43 = distinct !DILexicalBlock(scope: !38, file: !1, line: 15, column: 23)
!44 = !DILocation(line: 17, column: 5, scope: !43)
!45 = !DILocation(line: 9, column: 5, scope: !46)
!46 = !DILexicalBlockFile(scope: !24, file: !1, discriminator: 2)
!47 = !DILocation(line: 19, column: 5, scope: !7)
!48 = !DILocation(line: 20, column: 13, scope: !49)
!49 = distinct !DILexicalBlock(scope: !50, file: !1, line: 20, column: 13)
!50 = distinct !DILexicalBlock(scope: !51, file: !1, line: 19, column: 34)
!51 = distinct !DILexicalBlock(scope: !52, file: !1, line: 19, column: 5)
!52 = distinct !DILexicalBlock(scope: !7, file: !1, line: 19, column: 5)
!53 = !DILocation(line: 20, column: 21, scope: !49)
!54 = !DILocation(line: 20, column: 18, scope: !49)
!55 = !DILocation(line: 20, column: 13, scope: !50)
!56 = !DILocation(line: 21, column: 20, scope: !57)
!57 = distinct !DILexicalBlock(scope: !49, file: !1, line: 20, column: 24)
!58 = !DILocation(line: 21, column: 18, scope: !57)
!59 = !DILocation(line: 22, column: 9, scope: !57)
!60 = !DILocation(line: 24, column: 12, scope: !61)
!61 = distinct !DILexicalBlock(scope: !49, file: !1, line: 23, column: 14)
!62 = !DILocation(line: 26, column: 5, scope: !50)
!63 = !DILocation(line: 19, column: 27, scope: !64)
!64 = !DILexicalBlockFile(scope: !51, file: !1, discriminator: 1)
!65 = !DILocation(line: 19, column: 17, scope: !64)
!66 = !DILocation(line: 19, column: 15, scope: !64)
!67 = !DILocation(line: 19, column: 5, scope: !64)
!68 = !DILocalVariable(name: "p", scope: !10, file: !1, line: 30, type: !4)
!69 = !DILocation(line: 30, column: 11, scope: !10)
!70 = !DILocation(line: 31, column: 5, scope: !10)
!71 = !DILocation(line: 31, column: 12, scope: !72)
!72 = !DILexicalBlockFile(scope: !10, file: !1, discriminator: 1)
!73 = !DILocation(line: 31, column: 14, scope: !72)
!74 = !DILocation(line: 31, column: 5, scope: !72)
!75 = !DILocation(line: 31, column: 26, scope: !76)
!76 = !DILexicalBlockFile(scope: !10, file: !1, discriminator: 2)
!77 = !DILocation(line: 31, column: 20, scope: !76)
!78 = !DILocation(line: 31, column: 5, scope: !76)
!79 = !DILocation(line: 32, column: 1, scope: !10)
