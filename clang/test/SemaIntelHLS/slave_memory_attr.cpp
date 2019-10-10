//RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

#define slave_arg __attribute__((local_mem_size(32), slave_memory_argument))
#define not_slave_arg1 __attribute__((local_mem_size(32)))
#define not_slave_arg2 __attribute__((slave_memory_argument))
#define not_slave_arg3

__attribute__((ihc_component))
void foo0(slave_arg __attribute__((memory("MLAB"))) int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo0
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK-NEXT: MemoryAttr{{.*}}MLAB{{$}}
// CHECK: ComponentAttr

__attribute__((ihc_component))
void foo1(slave_arg __attribute__((memory("BLOCK_RAM"))) int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK-NEXT: MemoryAttr{{.*}}BlockRAM{{$}}
// CHECK: ComponentAttr

__attribute__((ihc_component))
void foo2(slave_arg __attribute__((numbanks(4))) int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo2
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: NumBanks{{.*}}
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
// CHECK: ComponentAttr

__attribute__((ihc_component))
void foo3(slave_arg __attribute__((bankwidth(4))) int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo3
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: BankWidthAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
// CHECK: ComponentAttr

__attribute__((ihc_component))
void foo4(slave_arg __attribute__((singlepump)) int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo4
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: SinglePumpAttr
// CHECK: ComponentAttr

__attribute__((ihc_component))
void foo5(slave_arg __attribute__((doublepump)) int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo5
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: DoublePumpAttr
// CHECK: ComponentAttr

__attribute__((ihc_component))
void foo6(slave_arg int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo6
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: ComponentAttr

__attribute__((ihc_component))
void foo7(slave_arg __attribute__((bank_bits(1,2,3))) int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo7
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: BankBitsAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: IntegerLiteral{{.*}}1{{$}}
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: IntegerLiteral{{.*}}3{{$}}
// CHECK: ComponentAttr

__attribute__((ihc_component))
void foo8(slave_arg int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo8
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: ComponentAttr

__attribute__((ihc_component))
void foo9(slave_arg int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo9
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: ComponentAttr

__attribute__((ihc_component))
void foo10(slave_arg __attribute__((internal_max_block_ram_depth(32)))
           int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo10
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: InternalMaxBlockRamDepthAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: IntegerLiteral{{.*}}32{{$}}
// CHECK: ComponentAttr

// Diagnostics

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar0a(
  not_slave_arg1 __attribute__((memory("MLAB")))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar1a(
  not_slave_arg1 __attribute__((memory("BLOCK_RAM")))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar2a(
  not_slave_arg1 __attribute__((numbanks(4)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar3a(
  not_slave_arg1 __attribute__((bankwidth(4)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar4a(
  not_slave_arg1 __attribute__((singlepump))
  int *i) {}

__attribute__((ihc_component))
void bar4d(
  //expected-error@+2{{attributes are not compatible}}
  slave_arg __attribute__((singlepump))
            __attribute__((__doublepump__))
  //expected-note@-2 {{conflicting attribute is here}}
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar5a(
  not_slave_arg1 __attribute__((doublepump))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar7a(
  not_slave_arg1 __attribute__((bank_bits(3,4)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar7b(
  not_slave_arg2 __attribute__((bank_bits(2,3)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar7c(
  not_slave_arg3 __attribute__((bank_bits(1,2)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar10a(
  not_slave_arg1 __attribute__((internal_max_block_ram_depth(32)))
  int *i) {}

__attribute__((ihc_component))
void bar11(
  //expected-error@+1{{'numbanks' attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
  not_slave_arg3 __attribute__((numbanks(4))) int *i) {}

__attribute__((ihc_component))
void baz1(
  //expected-error@+1{{'register' attribute only applies to constant variables, local variables, static variables, and non-static data members}}
  slave_arg __attribute__((register))
  int *i) {}

void baz2(
  slave_arg
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((argument_interface("avalon_mm_slave")))
  __attribute__((internal_max_block_ram_depth(32)))
  //expected-note@-2 {{conflicting attribute is here}}
  int *i0,
  slave_arg
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((internal_max_block_ram_depth(32)))
  __attribute__((argument_interface("avalon_mm_slave")))
  //expected-note@-2 {{conflicting attribute is here}}
  int *ip) {}

