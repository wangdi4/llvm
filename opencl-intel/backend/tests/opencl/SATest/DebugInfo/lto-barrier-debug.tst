; RUN: SATest -BUILD -tsize=1 -pass-manager-type=lto --config=%s.cfg --dump-llvm-file - | FileCheck %s

; Check that TLS globals are generated and debug info is attached to new alloca.

; CHECK-DAG: @__LocalIds = internal thread_local global [3 x i64] undef, align 16
; CHECK-DAG: @__pWorkDim = internal thread_local global {{.*}} undef, align 8
; CHECK-DAG: @__pWGId = internal thread_local global {{i64\*|ptr}} undef, align 8
; CHECK-DAG: @__BaseGlbId = internal thread_local global [4 x i64] undef, align 16
; CHECK-DAG: @__pSpecialBuf = internal thread_local global {{i8\*|ptr}} undef, align 8
; CHECK-DAG: @__RuntimeHandle = internal thread_local global {{\{\}\*|ptr}} undef, align 8

; CHECK-LABEL: @test

; CHECK: [[GID:%gid.addr.*]] = alloca {{i64\*|ptr}}

; CHECK: call void @llvm.dbg.declare(metadata {{.*}} [[GID]], metadata [[DIL_VAR:![0-9]+]], metadata !DIExpression(DW_OP_deref{{.*}})), !dbg [[DIL:![0-9]+]]

; CHECK-DAG: [[DIL]] = !DILocation(line: 2, column: 10, scope: [[SCOPE:![0-9]+]])
; CHECK-DAG: [[DIL_VAR]] = !DILocalVariable(name: "gid", scope: [[SCOPE]], file: !{{[0-9]+}}, line: 2
