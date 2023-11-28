// RUN: llvm-mc -triple x86_64-linux-gnu -filetype=obj %s | llvm-readobj -r - | FileCheck %s

// CHECK:      Relocations [
// CHECK-NEXT:   Section ({{[0-9]+}}) .rela.text {
ccmpbb	{of}	$foo, %bl               // CHECK-NEXT:     R_X86_64_8
ccmpbb	{of}	$foo, 123(%r8,%rax,4)   // CHECK-NEXT:     R_X86_64_8
ccmpbw	{of}	$foo, %bx               // CHECK-NEXT:     R_X86_64_16
ccmpbw	{of}	$foo, 123(%r8,%rax,4)   // CHECK-NEXT:     R_X86_64_16
ccmpbl	{of}	$foo, %ebx              // CHECK-NEXT:     R_X86_64_32
ccmpbl	{of}	$foo, 123(%r8,%rax,4)   // CHECK-NEXT:     R_X86_64_32
ccmpbq	{of}	$foo, %rbx              // CHECK-NEXT:     R_X86_64_32
ccmpbq	{of}	$foo, 123(%r8,%rax,4)   // CHECK-NEXT:     R_X86_64_32
// CHECK-NEXT:   }
// CHECK-NEXT: ]
