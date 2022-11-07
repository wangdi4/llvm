; RUN: SATest -BUILD -tsize=1 -pass-manager-type=lto --config=%s.cfg --dump-llvm-file - | FileCheck %s

; Check that TLS globals are generated and debug info is attached to new alloca.

; CHECK-DAG: @LocalIds = linkonce_odr thread_local global [3 x i64] undef, align 16
; CHECK-DAG: @pLocalMemBase = linkonce_odr thread_local global i8 addrspace(3)* undef, align 8
; CHECK-DAG: @pWorkDim = linkonce_odr thread_local global { i64, [3 x i64], [3 x i64], [2 x [3 x i64]], [3 x i64], {}*, {}* }* undef, align 8
; CHECK-DAG: @pWGId = linkonce_odr thread_local global i64* undef, align 8
; CHECK-DAG: @BaseGlbId = linkonce_odr thread_local global [4 x i64] undef, align 16
; CHECK-DAG: @pSpecialBuf = linkonce_odr thread_local global i8* undef, align 8
; CHECK-DAG: @RuntimeHandle = linkonce_odr thread_local global {}* undef, align 8

; CHECK-LABEL: @test

; CHECK: [[GID:%gid.addr.*]] = alloca i64*

; CHECK: call void @llvm.dbg.declare(metadata i64** [[GID]], metadata [[DIL_VAR:![0-9]+]], metadata !DIExpression(DW_OP_deref)), !dbg [[DIL:![0-9]+]]

; CHECK-DAG: [[DIL]] = !DILocation(line: 2, column: 10, scope: [[SCOPE:![0-9]+]])
; CHECK-DAG: [[DIL_VAR]] = !DILocalVariable(name: "gid", scope: [[SCOPE]], file: !{{[0-9]+}}, line: 2
