;RUN: llc < %s -mtriple=x86_64-unknown-linux-gnu | FileCheck %s -check-prefix=X64
;RUN: llc < %s -mtriple=x86_64-windows  | FileCheck %s -check-prefix=X64Win
;RUN: llc < %s -mtriple=x86_64-unknown-linux-gnu -opaque-pointers | FileCheck %s -check-prefix=X64
;RUN: llc < %s -mtriple=x86_64-windows -opaque-pointers | FileCheck %s -check-prefix=X64Win

@foo.lock2 = internal global i32 2, align 4
@foo.lock3 = internal global i32 3, align 4
@.str = private unnamed_addr constant [18 x i8] c"__itt_sync_create\00", align 1
@.str.1 = private unnamed_addr constant [20 x i8] c"__itt_sync_acquired\00", align 1
@.str.2 = private unnamed_addr constant [19 x i8] c"__itt_sync_destroy\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @foo(i32 %zc, i8* %lock) {
entry:
  %zc.addr = alloca i32, align 4
  %lock.addr = alloca i8*, align 8
  store i32 %zc, i32* %zc.addr, align 4
  store i8* %lock, i8** %lock.addr, align 8
  %0 = load i32, i32* %zc.addr, align 4
  %tobool = icmp ne i32 %0, 0
  br i1 %tobool, label %if.then, label %if.else

if.then:                                          ; preds = %entry
  fence syncscope("singlethread") seq_cst
  %1 = load i8*, i8** %lock.addr, align 8
  call void @llvm.notify.nzc(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i64 0, i64 0), i8* %1)
  br label %if.end

if.else:                                          ; preds = %entry
  fence syncscope("singlethread") seq_cst
  %2 = load i8*, i8** %lock.addr, align 8
  %add.ptr = getelementptr i8, i8* %2, i64 4
  call void @llvm.notify.nzc(i8* getelementptr inbounds ([20 x i8], [20 x i8]* @.str.1, i64 0, i64 0), i8* %add.ptr)
  br label %if.end

if.end:                                           ; preds = %if.else, %if.then
  %3 = load i8*, i8** %lock.addr, align 8
  %4 = load i32, i32* %zc.addr, align 4
  %idx.ext = sext i32 %4 to i64
  %add.ptr1 = getelementptr i8, i8* %3, i64 %idx.ext
  call void @llvm.notify.zc(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i64 0, i64 0), i8* %add.ptr1)
  %5 = load i32, i32* @foo.lock2, align 4
  %6 = load i32, i32* @foo.lock3, align 4
  %add = add nsw i32 %5, %6
  store i32 %add, i32* @foo.lock2, align 4
  fence syncscope("singlethread") seq_cst
  call void @llvm.notify.nzc(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str.2, i64 0, i64 0), i8* bitcast (i32* @foo.lock2 to i8*))
  call void @llvm.notify.zc(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str.2, i64 0, i64 0), i8* bitcast (i32* @foo.lock3 to i8*))
  %7 = load i8*, i8** %lock.addr, align 8
  call void @bar(i8* %7, i8* bitcast (i32* @foo.lock2 to i8*), i8* bitcast (i32* @foo.lock3 to i8*))
  ret i32 0
}

; Function Attrs: inaccessiblememonly noduplicate nounwind willreturn
declare void @llvm.notify.nzc(i8*, i8*)

; Function Attrs: inaccessiblememonly noduplicate nounwind willreturn
declare void @llvm.notify.zc(i8*, i8*)

declare dso_local void @bar(i8*, i8*, i8*)

;X64-LABLE: foo:

;X64:      # %bb.1:                                # %if.then
;X64-NEXT:         #MEMBARRIER
;X64-NEXT:         movq    8(%rsp), %rax
;X64-NEXT: .Lnotify_nzc1:
;X64-NEXT:         #__notify_intrinsic(__itt_sync_create, dwarf::DW_OP_reg0)
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT: .Lnotify_nzc_probe1:
;X64-NEXT:         jmp     .LBB0_3
;X64-NEXT: .LBB0_2:                                # %if.else
;X64-NEXT:         #MEMBARRIER
;X64-NEXT:         movq    8(%rsp), %rax
;X64-NEXT:         addq    $4, %rax
;X64-NEXT: .Lnotify_nzc0:
;X64-NEXT:         #__notify_intrinsic(__itt_sync_acquired, dwarf::DW_OP_reg0)
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT: .Lnotify_nzc_probe0:
;X64-NEXT: .LBB0_3:                                # %if.end
;X64-NEXT:         movslq  20(%rsp), %rax
;X64-NEXT:         addq    8(%rsp), %rax
;X64-NEXT: .Lnotify_zc0:
;X64-NEXT:         #__notify_zc_intrinsic(__itt_sync_create, dwarf::DW_OP_reg0)
;X64-NEXT: .Lnotify_zc_probe0:
;X64-NEXT:         movl    $foo.lock2, %eax
;X64-NEXT:         movl    $foo.lock3, %ecx
;X64-NEXT:         movl    foo.lock3(%rip), %edx
;X64-NEXT:         addl    %edx, foo.lock2(%rip)
;X64-NEXT:         #MEMBARRIER
;X64-NEXT: .Lnotify_nzc2:
;X64-NEXT:         #__notify_intrinsic(__itt_sync_destroy, dwarf::DW_OP_reg0)
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT:         nop
;X64-NEXT: .Lnotify_nzc_probe2:
;X64-NEXT: .Lnotify_zc1:
;X64-NEXT:         #__notify_zc_intrinsic(__itt_sync_destroy, dwarf::DW_OP_reg2)
;X64-NEXT: .Lnotify_zc_probe1:

;X64:              callq   bar
;X64:      .Lfunc_end0:

;X64:      .L.str:
;X64-NEXT:         .asciz  "__itt_sync_create"
;X64-NEXT:         .size   .L.str, 18

;X64:      .L.str.1:
;X64-NEXT:         .asciz  "__itt_sync_acquired"
;X64-NEXT:         .size   .L.str.1, 20

;X64:      .L.str.2:
;X64-NEXT:         .asciz  "__itt_sync_destroy"
;X64-NEXT:         .size   .L.str.2, 19

;X64:              .section        .itt_notify_tab,"a",@progbits
;X64-NEXT: itt_notify_tab:
;X64-NEXT:         .ascii  ".itt_notify_tab"
;X64-NEXT:         .byte   0
;X64-NEXT:         .short  257
;X64-NEXT:         .short  5
;X64-NEXT:         .long   .Lnotify_strings0-itt_notify_tab
;X64-NEXT:         .long   .Lnotify_exprs0-.Lnotify_strings0
;X64-NEXT:         .long   .Lnotify_exprs0-itt_notify_tab
;X64-NEXT:         .long   .Litt_notify_tab_end0-.Lnotify_exprs0
;X64-NEXT:         .p2align        2
;X64-NEXT: .Lnotify_entries0:
;X64-NEXT:         .quad   .Lnotify_nzc0
;X64-NEXT:         .long   .Lnotify_nzc_probe0-.Lnotify_nzc0
;X64-NEXT:         .long   0
;X64-NEXT:         .long   0
;X64-NEXT:         .quad   .Lnotify_nzc1
;X64-NEXT:         .long   .Lnotify_nzc_probe1-.Lnotify_nzc1
;X64-NEXT:         .long   20
;X64-NEXT:         .long   2
;X64-NEXT:         .quad   .Lnotify_zc0
;X64-NEXT:         .long   .Lnotify_zc_probe0-.Lnotify_zc0
;X64-NEXT:         .long   38
;X64-NEXT:         .long   4
;X64-NEXT:         .quad   .Lnotify_nzc2
;X64-NEXT:         .long   .Lnotify_nzc_probe2-.Lnotify_nzc2
;X64-NEXT:         .long   56
;X64-NEXT:         .long   6
;X64-NEXT:         .quad   .Lnotify_zc1
;X64-NEXT:         .long   .Lnotify_zc_probe1-.Lnotify_zc1
;X64-NEXT:         .long   75
;X64-NEXT:         .long   8
;X64-NEXT: .Lnotify_strings0:
;X64-NEXT:         .ascii  "__itt_sync_acquired"
;X64-NEXT:         .byte   0
;X64-NEXT:         .ascii  "__itt_sync_create"
;X64-NEXT:         .byte   0
;X64-NEXT:         .ascii  "__itt_sync_create"
;X64-NEXT:         .byte   0
;X64-NEXT:         .ascii  "__itt_sync_destroy"
;X64-NEXT:         .byte   0
;X64-NEXT:         .ascii  "__itt_sync_destroy"
;X64-NEXT:         .byte   0
;X64-NEXT: .Lnotify_exprs0:
;X64-NEXT:         .short  20481
;X64-NEXT:         .short  20481
;X64-NEXT:         .short  20481
;X64-NEXT:         .short  20481
;X64-NEXT:         .short  20993
;X64-NEXT: .Litt_notify_tab_end0:
;X64-NEXT: .Lsec_end0:


;X64Win-LABEL: foo:
;X64Win:      # %bb.1:
;X64Win-NEXT:         #MEMBARRIER
;X64Win-NEXT:         movq    40(%rsp), %rax
;X64Win-NEXT: .Lnotify_nzc1:
;X64Win-NEXT:         #__notify_intrinsic(__itt_sync_create, dwarf::DW_OP_reg0)
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT: .Lnotify_nzc_probe1:
;X64Win-NEXT:         jmp     .LBB0_3
;X64Win-NEXT: .LBB0_2:
;X64Win-NEXT:         #MEMBARRIER
;X64Win-NEXT:         movq    40(%rsp), %rax
;X64Win-NEXT:         addq    $4, %rax
;X64Win-NEXT: .Lnotify_nzc0:
;X64Win-NEXT:         #__notify_intrinsic(__itt_sync_acquired, dwarf::DW_OP_reg0)
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT: .Lnotify_nzc_probe0:
;X64Win-NEXT: .LBB0_3:
;X64Win-NEXT:         movslq  52(%rsp), %rax
;X64Win-NEXT:         addq    40(%rsp), %rax
;X64Win-NEXT: .Lnotify_zc0:
;X64Win-NEXT:         #__notify_zc_intrinsic(__itt_sync_create, dwarf::DW_OP_reg0)
;X64Win-NEXT: .Lnotify_zc_probe0:
;X64Win-NEXT:         leaq    foo.lock2(%rip), %rdx
;X64Win-NEXT:         leaq    foo.lock3(%rip), %r8
;X64Win-NEXT:         movl    foo.lock3(%rip), %eax
;X64Win-NEXT:         addl    %eax, foo.lock2(%rip)
;X64Win-NEXT:         #MEMBARRIER
;X64Win-NEXT: .Lnotify_nzc2:
;X64Win-NEXT:         #__notify_intrinsic(__itt_sync_destroy, dwarf::DW_OP_reg1)
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT:         nop
;X64Win-NEXT: .Lnotify_nzc_probe2:
;X64Win-NEXT: .Lnotify_zc1:
;X64Win-NEXT:         #__notify_zc_intrinsic(__itt_sync_destroy, dwarf::DW_OP_reg8)
;X64Win-NEXT: .Lnotify_zc_probe1:
;X64Win:              movq    40(%rsp), %rcx

;X64Win:      .L.str:                                 # @.str
;X64Win-NEXT:         .asciz  "__itt_sync_create"
;X64Win:      .L.str.1:                               # @.str.1
;X64Win-NEXT:         .asciz  "__itt_sync_acquired"
;X64Win:      .L.str.2:                               # @.str.2
;X64Win-NEXT:         .asciz  "__itt_sync_destroy"

;X64Win:              .section        .itt_not,"dr"
;X64Win-NEXT: itt_notify_tab:
;X64Win-NEXT:         .ascii  ".itt_notify_tab"
;X64Win-NEXT:         .byte   0
;X64Win-NEXT:         .short  257
;X64Win-NEXT:         .short  5
;X64Win-NEXT:         .long   .Lnotify_strings0-itt_notify_tab
;X64Win-NEXT:         .long   .Lnotify_exprs0-.Lnotify_strings0
;X64Win-NEXT:         .long   .Lnotify_exprs0-itt_notify_tab
;X64Win-NEXT:         .long   .Litt_notify_tab_end0-.Lnotify_exprs0
;X64Win-NEXT:         .p2align        2
;X64Win-NEXT: .Lnotify_entries0:
;X64Win-NEXT:         .quad   .Lnotify_nzc0
;X64Win-NEXT:         .long   .Lnotify_nzc_probe0-.Lnotify_nzc0
;X64Win-NEXT:         .long   0
;X64Win-NEXT:         .long   0
;X64Win-NEXT:         .quad   .Lnotify_nzc1
;X64Win-NEXT:         .long   .Lnotify_nzc_probe1-.Lnotify_nzc1
;X64Win-NEXT:         .long   20
;X64Win-NEXT:         .long   2
;X64Win-NEXT:         .quad   .Lnotify_zc0
;X64Win-NEXT:         .long   .Lnotify_zc_probe0-.Lnotify_zc0
;X64Win-NEXT:         .long   38
;X64Win-NEXT:         .long   4
;X64Win-NEXT:         .quad   .Lnotify_nzc2
;X64Win-NEXT:         .long   .Lnotify_nzc_probe2-.Lnotify_nzc2
;X64Win-NEXT:         .long   56
;X64Win-NEXT:         .long   6
;X64Win-NEXT:         .quad   .Lnotify_zc1
;X64Win-NEXT:         .long   .Lnotify_zc_probe1-.Lnotify_zc1
;X64Win-NEXT:         .long   75
;X64Win-NEXT:         .long   8
;X64Win-NEXT: .Lnotify_strings0:
;X64Win-NEXT:         .ascii  "__itt_sync_acquired"
;X64Win-NEXT:         .byte   0
;X64Win-NEXT:         .ascii  "__itt_sync_create"
;X64Win-NEXT:         .byte   0
;X64Win-NEXT:         .ascii  "__itt_sync_create"
;X64Win-NEXT:         .byte   0
;X64Win-NEXT:         .ascii  "__itt_sync_destroy"
;X64Win-NEXT:         .byte   0
;X64Win-NEXT:         .ascii  "__itt_sync_destroy"
;X64Win-NEXT:         .byte   0
;X64Win-NEXT: .Lnotify_exprs0:
;X64Win-NEXT:         .short  20481
;X64Win-NEXT:         .short  20481
;X64Win-NEXT:         .short  20481
;X64Win-NEXT:         .short  20737
;X64Win-NEXT:         .short  22529
;X64Win-NEXT: .Litt_notify_tab_end0:
;X64Win-NEXT: .Lsec_end0:
