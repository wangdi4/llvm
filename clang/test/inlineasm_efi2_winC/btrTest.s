// RUN: llvm-mc -x86-asm-syntax=intel -output-asm-variant=1 < %s | FileCheck %s

		btr	ax,4
//CHECK:	btr	ax, 4
		btr	ax,dx
//CHECK:	btr	ax, dx
		btr	eax,4
//CHECK:	btr	eax, 4
		btr	eax,ecx
//CHECK:	btr	eax, ecx
		btr	rax,4
//CHECK:	btr	rax, 4
		btr	rax,rcx
//CHECK:	btr	rax, rcx
		btr	QWORD PTR test_mem,4
//CHECK:	btr	qword ptr [test_mem], 4
		btr	DWORD PTR test_mem,4
//CHECK:	btr	dword ptr [test_mem], 4
		btr	WORD PTR test_mem,4
//CHECK:	btr	word ptr [test_mem], 4
		btr	WORD PTR test_mem,ax
//CHECK:	btr	dword ptr [test_mem], ax
		btr	DWORD PTR test_mem,eax
//CHECK:	btr	dword ptr [test_mem], eax
		btr	QWORD PTR test_mem,rax
//CHECK:	btr	dword ptr [test_mem], rax
		btr	XMMWORD PTR test_mem,4
//CHECK:	btr	dword ptr [test_mem], 4
		btr	TBYTE PTR test_mem,4
//CHECK:	btr	dword ptr [test_mem], 4
		btr	BYTE PTR test_mem,4
//CHECK:	btr	dword ptr [test_mem], 4
		btr	XMMWORD PTR test_mem,ax
//CHECK:	btr	dword ptr [test_mem], ax
		btr	QWORD PTR test_mem,ax
//CHECK:	btr	dword ptr [test_mem], ax
		btr	TBYTE PTR test_mem,ax
//CHECK:	btr	dword ptr [test_mem], ax
		btr	DWORD PTR test_mem,ax
//CHECK:	btr	dword ptr [test_mem], ax
		btr	BYTE PTR test_mem,ax
//CHECK:	btr	dword ptr [test_mem], ax
		btr	XMMWORD PTR test_mem,eax
//CHECK:	btr	dword ptr [test_mem], eax
		btr	QWORD PTR test_mem,eax
//CHECK:	btr	dword ptr [test_mem], eax
		btr	TBYTE PTR test_mem,eax
//CHECK:	btr	dword ptr [test_mem], eax
		btr	WORD PTR test_mem,eax
//CHECK:	btr	dword ptr [test_mem], eax
		btr	BYTE PTR test_mem,eax
//CHECK:	btr	dword ptr [test_mem], eax
		btr	XMMWORD PTR test_mem,rax
//CHECK:	btr	dword ptr [test_mem], rax
		btr	TBYTE PTR test_mem,rax
//CHECK:	btr	dword ptr [test_mem], rax
		btr	DWORD PTR test_mem,rax
//CHECK:	btr	dword ptr [test_mem], rax
		btr	WORD PTR test_mem,rax
//CHECK:	btr	dword ptr [test_mem], rax
		btr	BYTE PTR test_mem,rax
//CHECK:	btr	dword ptr [test_mem], rax
