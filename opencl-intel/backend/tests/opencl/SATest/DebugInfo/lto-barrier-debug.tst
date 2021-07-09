; RUN: SATest -BUILD -tsize=1 -pass-manager-type=lto-legacy --config=%s.cfg --dump-llvm-file - | FileCheck %s

; Check that debug info is attached to new allloca.

; CHECK-LABEL: @test

; CHECK: [[GID:%gid.addr.*]] = alloca i64*

; CHECK: call void @llvm.dbg.declare(metadata i64** [[GID]], metadata [[DIL_VAR:![0-9]+]], metadata !DIExpression(DW_OP_deref)), !dbg [[DIL:![0-9]+]]

; CHECK: [[DIL]] = !DILocation(line: 2, column: 10, scope: [[SCOPE:![0-9]+]])
; CHECK: [[DIL_VAR]] = !DILocalVariable(name: "gid", scope: [[SCOPE]], file: !{{[0-9]+}}, line: 2
