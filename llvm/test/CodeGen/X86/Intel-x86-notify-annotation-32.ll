;RUN: llc < %s -mtriple=i386-unknown-linux-gnu  | FileCheck %s -check-prefix=X86
;RUN: llc < %s -mtriple=i386-pc-win32  | FileCheck %s -check-prefix=X86Win

@main.lock = internal global i32 7, align 4
@main.lock2 = internal global i32 9, align 4
@.str = private unnamed_addr constant [18 x i8] c"__itt_sync_create\00", align 1
@.str.1 = private unnamed_addr constant [19 x i8] c"__itt_sync_destroy\00", align 1

; Function Attrs: noinline nounwind optnone
define dso_local i32 @main() #0 {
entry:
  %retval = alloca i32, align 4
  store i32 0, i32* %retval, align 4
  call void @llvm.notify.zc(i8* getelementptr inbounds ([18 x i8], [18 x i8]* @.str, i32 0, i32 0), i8* bitcast (i32* @main.lock to i8*))
  fence syncscope("singlethread") seq_cst
  call void @llvm.notify.nzc(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str.1, i32 0, i32 0), i8* bitcast (i32* @main.lock2 to i8*))
  call void @llvm.notify.zc(i8* getelementptr inbounds ([19 x i8], [19 x i8]* @.str.1, i32 0, i32 0), i8* bitcast (i32* @main.lock to i8*))
  ret i32 0
}

; Function Attrs: inaccessiblememonly noduplicate nounwind willreturn
declare void @llvm.notify.zc(i8*, i8*) #1

; Function Attrs: inaccessiblememonly noduplicate nounwind willreturn
declare void @llvm.notify.nzc(i8*, i8*) #1

;X86-LABEL: main:
;X86:              movl    $main.lock, %eax
;X86-NEXT: .Lnotify_zc0:
;X86-NEXT:         #__notify_zc_intrinsic(__itt_sync_create, dwarf::DW_OP_reg0)
;X86-NEXT: .Lnotify_zc_probe0:
;X86-NEXT:         #MEMBARRIER
;X86-NEXT:         movl    $main.lock2, %ecx
;X86-NEXT: .Lnotify_nzc0:
;X86-NEXT:         #__notify_intrinsic(__itt_sync_destroy, dwarf::DW_OP_reg1)
;X86-NEXT:         nop
;X86-NEXT:         nop
;X86-NEXT:         nop
;X86-NEXT:         nop
;X86-NEXT:         nop
;X86-NEXT:         nop
;X86-NEXT: .Lnotify_nzc_probe0:
;X86-NEXT: .Lnotify_zc1:
;X86-NEXT:         #__notify_zc_intrinsic(__itt_sync_destroy, dwarf::DW_OP_reg0)
;X86-NEXT: .Lnotify_zc_probe1:
;X86-NEXT:         xorl    %eax, %eax

;X86:      .Lfunc_end0:
;X86:      main.lock:
;X86-NEXT:         .long   7                       # 0x7
;X86-NEXT:         .size   main.lock, 4
;X86:      main.lock2:
;X86-NEXT:         .long   9                       # 0x9
;X86-NEXT:         .size   main.lock2, 4
;X86:      .L.str:
;X86-NEXT:         .asciz  "__itt_sync_create"
;X86-NEXT:         .size   .L.str, 18
;X86:      .L.str.1:
;X86-NEXT:         .asciz  "__itt_sync_destroy"
;X86-NEXT:         .size   .L.str.1, 19

;X86:              .section        .itt_notify_tab,"a",@progbits
;X86-NEXT: itt_notify_tab:
;X86-NEXT:         .ascii  ".itt_notify_tab"
;X86-NEXT:         .byte   0
;X86-NEXT:         .short  257
;X86-NEXT:         .short  3
;X86-NEXT:         .long   .Ltmp1-itt_notify_tab
;X86-NEXT:         .long   .Ltmp2-.Ltmp1
;X86-NEXT:         .long   .Ltmp2-itt_notify_tab
;X86-NEXT:         .long   .Ltmp3-.Ltmp2
;X86-NEXT:         .p2align        2
;X86-NEXT: .Ltmp0:
;X86-NEXT:         .quad   .Lnotify_zc0
;X86-NEXT:         .long   .Lnotify_zc_probe0-.Lnotify_zc0
;X86-NEXT:         .long   0
;X86-NEXT:         .long   0
;X86-NEXT:         .quad   .Lnotify_nzc0
;X86-NEXT:         .long   .Lnotify_nzc_probe0-.Lnotify_nzc0
;X86-NEXT:         .long   18
;X86-NEXT:         .long   2
;X86-NEXT:         .quad   .Lnotify_zc1
;X86-NEXT:         .long   .Lnotify_zc_probe1-.Lnotify_zc1
;X86-NEXT:         .long   37
;X86-NEXT:         .long   4
;X86-NEXT: .Ltmp1:
;X86-NEXT:         .ascii  "__itt_sync_create"
;X86-NEXT:         .byte   0
;X86-NEXT:         .ascii  "__itt_sync_destroy"
;X86-NEXT:         .byte   0
;X86-NEXT:         .ascii  "__itt_sync_destroy"
;X86-NEXT:         .byte   0
;X86-NEXT: .Ltmp2:
;X86-NEXT:         .short  20481
;X86-NEXT:         .short  20737
;X86-NEXT:         .short  20481
;X86-NEXT: .Ltmp3:
;X86-NEXT: .Lsec_end0:


;X86Win-LABEL: _main:                                  # @main
;X86Win:              movl    $_main.lock, %eax
;X86Win-NEXT: Lnotify_zc0:
;X86Win-NEXT:         #__notify_zc_intrinsic(__itt_sync_create, dwarf::DW_OP_reg0)
;X86Win-NEXT: Lnotify_zc_probe0:
;X86Win-NEXT:         #MEMBARRIER
;X86Win-NEXT:         movl    $_main.lock2, %ecx
;X86Win-NEXT: Lnotify_nzc0:
;X86Win-NEXT:         #__notify_intrinsic(__itt_sync_destroy, dwarf::DW_OP_reg1)
;X86Win-NEXT:         nop
;X86Win-NEXT:         nop
;X86Win-NEXT:         nop
;X86Win-NEXT:         nop
;X86Win-NEXT:         nop
;X86Win-NEXT:         nop
;X86Win-NEXT: Lnotify_nzc_probe0:
;X86Win-NEXT: Lnotify_zc1:
;X86Win-NEXT:         #__notify_zc_intrinsic(__itt_sync_destroy, dwarf::DW_OP_reg0)
;X86Win-NEXT: Lnotify_zc_probe1:
;X86Win-NEXT:         xorl    %eax, %eax
;X86Win-NEXT:         popl    %ecx
;X86Win-NEXT:         retl

;X86Win:      L_.str:                                 # @.str
;X86Win-NEXT:         .asciz  "__itt_sync_create"
;X86Win:      L_.str.1:                               # @.str.1
;X86Win-NEXT:         .asciz  "__itt_sync_destroy"

;X86Win:              .section        .itt_not,"dr"
;X86Win-NEXT: itt_notify_tab:
;X86Win-NEXT:         .ascii  ".itt_notify_tab"
;X86Win-NEXT:         .byte   0
;X86Win-NEXT:         .short  257
;X86Win-NEXT:         .short  3
;X86Win-NEXT:         .long   Ltmp1-itt_notify_tab
;X86Win-NEXT:         .long   Ltmp2-Ltmp1
;X86Win-NEXT:         .long   Ltmp2-itt_notify_tab
;X86Win-NEXT:         .long   Ltmp3-Ltmp2
;X86Win-NEXT:         .p2align        2
;X86Win-NEXT: Ltmp0:
;X86Win-NEXT:         .quad   Lnotify_zc0
;X86Win-NEXT:         .long   Lnotify_zc_probe0-Lnotify_zc0
;X86Win-NEXT:         .long   0
;X86Win-NEXT:         .long   0
;X86Win-NEXT:         .quad   Lnotify_nzc0
;X86Win-NEXT:         .long   Lnotify_nzc_probe0-Lnotify_nzc0
;X86Win-NEXT:         .long   18
;X86Win-NEXT:         .long   2
;X86Win-NEXT:         .quad   Lnotify_zc1
;X86Win-NEXT:         .long   Lnotify_zc_probe1-Lnotify_zc1
;X86Win-NEXT:         .long   37
;X86Win-NEXT:         .long   4
;X86Win-NEXT: Ltmp1:
;X86Win-NEXT:         .ascii  "__itt_sync_create"
;X86Win-NEXT:         .byte   0
;X86Win-NEXT:         .ascii  "__itt_sync_destroy"
;X86Win-NEXT:         .byte   0
;X86Win-NEXT:         .ascii  "__itt_sync_destroy"
;X86Win-NEXT:         .byte   0
;X86Win-NEXT: Ltmp2:
;X86Win-NEXT:         .short  20481
;X86Win-NEXT:         .short  20737
;X86Win-NEXT:         .short  20481
;X86Win-NEXT: Ltmp3:
;X86Win-NEXT: Lsec_end0:
