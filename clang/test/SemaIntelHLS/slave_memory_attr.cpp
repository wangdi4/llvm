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
void foo6(slave_arg __attribute__((numports_readonly_writeonly(4,4))) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
          int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo6
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: NumReadPortsAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
// CHECK-NEXT: NumWritePortsAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
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
void foo8(slave_arg __attribute__((numreadports(2))) int *i_par1) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
{}
// CHECK: FunctionDecl{{.*}}foo8
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: NumReadPortsAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
// CHECK: ComponentAttr

__attribute__((ihc_component))
void foo9(slave_arg __attribute__((numwriteports(4))) int *i_par1) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
{}
// CHECK: FunctionDecl{{.*}}foo9
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: NumWritePortsAttr
// CHECK-NEXT: ConstantExpr
// CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
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

__attribute__((ihc_component))
void foo11(slave_arg __attribute__((optimize_fmax))
           int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo11
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: OptimizeFMaxAttr
// CHECK: ComponentAttr

__attribute__((ihc_component))
void foo12(slave_arg __attribute__((optimize_ram_usage))
           int *i_par1)
{}
// CHECK: FunctionDecl{{.*}}foo12
// CHECK: ParmVarDecl{{.*}}i_par1
// CHECK-NEXT: OpenCLLocalMemSizeAttr
// CHECK-NEXT: SlaveMemoryArgumentAttr
// CHECK: OptimizeRamUsageAttr
// CHECK: ComponentAttr


// Diagnostics

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar0a(
  not_slave_arg1 __attribute__((memory("MLAB")))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar0b(
  not_slave_arg2 __attribute__((memory("MLAB")))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar0c(
  not_slave_arg3 __attribute__((memory("MLAB")))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar1a(
  not_slave_arg1 __attribute__((memory("BLOCK_RAM")))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar1b(
  not_slave_arg2 __attribute__((memory("BLOCK_RAM")))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar1c(
  not_slave_arg3 __attribute__((memory("BLOCK_RAM")))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar2a(
  not_slave_arg1 __attribute__((numbanks(4)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar2b(
  not_slave_arg2 __attribute__((numbanks(4)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar2c(
  not_slave_arg3 __attribute__((numbanks(4)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar3a(
  not_slave_arg1 __attribute__((bankwidth(4)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar3b(
  not_slave_arg2 __attribute__((bankwidth(4)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar3c(
  not_slave_arg3 __attribute__((bankwidth(4)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar4a(
  not_slave_arg1 __attribute__((singlepump))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar4b(
  not_slave_arg2 __attribute__((singlepump))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar4c(
  not_slave_arg3 __attribute__((singlepump))
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
void bar5b(
  not_slave_arg2 __attribute__((doublepump))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar5c(
  not_slave_arg3 __attribute__((doublepump))
  int *i) {}

// expected-error@+4{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar6a(
  not_slave_arg1 __attribute__((numports_readonly_writeonly(4,4))) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
  int *i) {}

// expected-error@+4{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar6b(
  not_slave_arg2 __attribute__((numports_readonly_writeonly(4,4))) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
  int *i) {}

// expected-error@+4{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar6c(
  not_slave_arg3 __attribute__((numports_readonly_writeonly(4,4))) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
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
void bar8a(
  not_slave_arg1 __attribute__((numreadports(4))) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar8b(
  not_slave_arg2 __attribute__((numreadports(4))) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar8c(
  not_slave_arg3 __attribute__((numreadports(4))) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar9a(
  not_slave_arg1 __attribute__((numwriteports(2))) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar9b(
  not_slave_arg2 __attribute__((numwriteports(2))) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar9c(
  not_slave_arg3 __attribute__((numwriteports(2))) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar10a(
  not_slave_arg1 __attribute__((internal_max_block_ram_depth(32)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar10b(
  not_slave_arg2 __attribute__((internal_max_block_ram_depth(32)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar10c(
  not_slave_arg3 __attribute__((internal_max_block_ram_depth(32)))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar11a(
  not_slave_arg1 __attribute__((optimize_fmax))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar11b(
  not_slave_arg2 __attribute__((optimize_fmax))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar11c(
  not_slave_arg3 __attribute__((optimize_fmax))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar12a(
  not_slave_arg1 __attribute__((optimize_ram_usage))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar12b(
  not_slave_arg2 __attribute__((optimize_ram_usage))
  int *i) {}

// expected-error@+3{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar12c(
  not_slave_arg3 __attribute__((optimize_ram_usage))
  int *i) {}

// expected-error@+5{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
// expected-error@+5{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
// expected-error@+5{{attribute only applies to slave memory arguments, non-static field members, constant variables, local variables and static variables}}
__attribute__((ihc_component))
void bar11(
  not_slave_arg3 __attribute__((numbanks(4)))
                 __attribute__((numreadports(4))) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
                 __attribute__((numwriteports(4))) //expected-warning{{attributes numreadports/numwriteports/numports_readonly_writeonly are deprecated, use max_replicates attribute instead}}
  int *i) {}

// expected-error@+3{{only applies to constant variables, local variables, static variables, and non-static data members}}
__attribute__((ihc_component))
void baz1(
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

void baz3(
  slave_arg
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((argument_interface("avalon_mm_slave")))
  __attribute__((optimize_fmax))
  //expected-note@-2 {{conflicting attribute is here}}
  int *i0,
  slave_arg
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((optimize_fmax))
  __attribute__((argument_interface("avalon_mm_slave")))
  //expected-note@-2 {{conflicting attribute is here}}
  int *ip) {}

void baz4(
  slave_arg
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((argument_interface("avalon_mm_slave")))
  __attribute__((optimize_ram_usage))
  //expected-note@-2 {{conflicting attribute is here}}
  int *i0,
  slave_arg
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((optimize_ram_usage))
  __attribute__((argument_interface("avalon_mm_slave")))
  //expected-note@-2 {{conflicting attribute is here}}
  int *ip) {}
