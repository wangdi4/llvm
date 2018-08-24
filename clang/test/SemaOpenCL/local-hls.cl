// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s
// RUN: %clang_cc1 -x cl -triple x86_64-unknown-unknown-intelfpga -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

//CHECK: FunctionDecl{{.*}}foo1
void foo1()
{
  //CHECK: VarDecl{{.*}}v_one
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: DoublePumpAttr
  __attribute__((__doublepump__))
  unsigned int v_one[64];

  //CHECK: VarDecl{{.*}}v_two
  //CHECK: MemoryAttr
  __attribute__((__memory__))
  unsigned int v_two[64];

  //CHECK: VarDecl{{.*}}v_three
  //CHECK: RegisterAttr
  __attribute__((__register__))
  unsigned int v_three[64];

  //CHECK: VarDecl{{.*}}v_four
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: SinglePumpAttr
  __attribute__((__singlepump__))
  unsigned int v_four[64];

  //CHECK: VarDecl{{.*}}v_five
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: BankWidthAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  __attribute__((__bankwidth__(4)))
  unsigned int v_five[64];

  //CHECK: VarDecl{{.*}}v_six
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: NumBanksAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}
  __attribute__((__numbanks__(8)))
  unsigned int v_six[64];

  //CHECK: VarDecl{{.*}}v_seven
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: NumReadPortsAttr
  //CHECK: IntegerLiteral{{.*}}2{{$}}
  __attribute__((__numreadports__(2)))
  unsigned int v_seven[64];

  //CHECK: VarDecl{{.*}}v_eight
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: NumWritePortsAttr
  //CHECK: IntegerLiteral{{.*}}4{{$}}
  __attribute__((__numwriteports__(4)))
  unsigned int v_eight[64];

  //CHECK: VarDecl{{.*}}v_nine
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: NumReadPortsAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  //CHECK: NumWritePortsAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  __attribute__((__numports_readonly_writeonly__(4,16)))
  unsigned int v_nine[64];

  //CHECK: VarDecl{{.*}}v_ten
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: MergeAttr{{.*}}"mrg1" "depth"{{$}}
  __attribute__((__merge__("mrg1","depth")))
  unsigned int v_ten[64];

  //CHECK: VarDecl{{.*}}v_eleven
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: MergeAttr{{.*}}"mrg2" "width"{{$}}
  __attribute__((__merge__("mrg2","width")))
  unsigned int v_eleven[64];

  //CHECK: VarDecl{{.*}}v_twelve
  //CHECK: NumBanksAttr{{.*}}Implicit{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: BankBitsAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}3{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}5{{$}}
  __attribute__((__bank_bits__(2,3,4,5)))
  unsigned int v_twelve[64];

  //CHECK: VarDecl{{.*}}v_thirteen
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: BankWidthAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  //CHECK: NumBanksAttr{{.*}}Implicit{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  //CHECK: BankBitsAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}3{{$}}
  __attribute__((__bank_bits__(2,3), __bankwidth__(16)))
  unsigned int v_thirteen[64];

  int __attribute__((__register__)) A;
  int __attribute__((__numbanks__(4), __bankwidth__(16), __singlepump__,
                     __numreadports__(1), __numwriteports__(2))) B;
  int __attribute__((__numbanks__(4), __bankwidth__(16), __doublepump__,
                     __numreadports__(1), __numwriteports__(2))) C;
  int __attribute__((__numbanks__(4), __bankwidth__(16), __doublepump__)) D;
  int __attribute__((__numbanks__(4), __bankwidth__(16))) E;
  int __attribute__((__bank_bits__(2,3), __bankwidth__(16))) F;

  // diagnostics

  // **doublepump
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__doublepump__))
  __attribute__((__singlepump__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int dp_one[64];

  //expected-warning@+1{{attribute 'doublepump' is already applied}}
  __attribute__((doublepump))  __attribute__((__doublepump__))
  unsigned int dp_two[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__doublepump__))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int dp_three[64];

  // **singlepump
  //expected-error@+1{{attributes are not compatible}}
  __attribute__((__singlepump__,__doublepump__))
  //expected-note@-1 {{conflicting attribute is here}}
  unsigned int sp_one[64];

  //expected-warning@+1{{attribute 'singlepump' is already applied}}
  __attribute__((singlepump))  __attribute__((__singlepump__))
  unsigned int sp_two[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__singlepump__))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int sp_three[64];

  // **register
  //expected-warning@+1{{attribute 'register' is already applied}}
  __attribute__((register)) __attribute__((__register__))
  unsigned int reg_one[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__register__))
  __attribute__((__singlepump__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int reg_two[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__register__))
  __attribute__((__doublepump__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int reg_three[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__register__))
  __attribute__((__memory__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int reg_four[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__register__))
  __attribute__((__bank_bits__(4,5)))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int reg_five[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__register__))
  __attribute__((__bankwidth__(16)))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int reg_six[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__register__))
  __attribute__((__numbanks__(8)))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int reg_seven[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__register__))
  __attribute__((__numreadports__(8)))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int reg_eight[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__register__))
  __attribute__((__numwriteports__(8)))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int reg_nine[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__register__))
  __attribute__((__numports_readonly_writeonly__(4,8)))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int reg_ten[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__register__))
  __attribute__((__merge__("mrg1","depth")))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int reg_eleven[64];

  // **memory
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__memory__))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int mem_one[64];

  //expected-warning@+1{{attribute 'memory' is already applied}}
  __attribute__((memory)) __attribute__((__memory__))
  unsigned int mem_two[64];

  // bankwidth
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__bankwidth__(16)))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int bw_one[64];

  //CHECK: VarDecl{{.*}}bw_two
  //CHECK: BankWidthAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}
  //CHECK: BankWidthAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  //expected-warning@+2{{attribute 'bankwidth' is already applied}}
  __attribute__((__bankwidth__(8)))
  __attribute__((__bankwidth__(16)))
  unsigned int bw_two[64];

  //expected-error@+1{{must be a constant power of two greater than zero}}
  __attribute__((__bankwidth__(3)))
  unsigned int bw_three[64];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((__bankwidth__(-4)))
  unsigned int bw_four[64];

  int i_bankwidth = 32;
  //expected-error@+1{{is not an integer constant expression}}
  __attribute__((__bankwidth__(i_bankwidth)))
  unsigned int bw_five[64];

  //expected-error@+1{{'__bankwidth__' attribute takes one argument}}
  __attribute__((__bankwidth__(4,8)))
  unsigned int bw_six[64];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((__bankwidth__(0)))
  unsigned int bw_seven[64];

  // numbanks
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__numbanks__(16)))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int nb_one[64];

  //CHECK: VarDecl{{.*}}nb_two
  //CHECK: NumBanksAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}
  //CHECK: NumBanksAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  //expected-warning@+2{{attribute 'numbanks' is already applied}}
  __attribute__((__numbanks__(8)))
  __attribute__((__numbanks__(16)))
  unsigned int nb_two[64];

  //expected-error@+1{{must be a constant power of two greater than zero}}
  __attribute__((__numbanks__(15)))
  unsigned int nb_three[64];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((__numbanks__(-4)))
  unsigned int nb_four[64];

  int i_numbanks = 32;
  //expected-error@+1{{is not an integer constant expression}}
  __attribute__((__numbanks__(i_numbanks)))
  unsigned int nb_five[64];

  //expected-error@+1{{'__numbanks__' attribute takes one argument}}
  __attribute__((__numbanks__(4,8)))
  unsigned int nb_six[64];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((__numbanks__(0)))
  unsigned int nb_seven[64];

  // numreadports
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__numreadports__(4)))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int nrp_one[4];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((__numreadports__(-4)))
  unsigned int nrp_two[4];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((__numreadports__(0)))
  unsigned int nrp_three[4];

  // numwriteports
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__numwriteports__(4)))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int nwp_one[4];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((__numwriteports__(-4)))
  unsigned int nwp_two[4];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((__numwriteports__(0)))
  unsigned int nwp_three[4];

  // numports_readonly_writeonly
  //expected-error@+5{{attributes are not compatible}}
  //expected-note@+3 {{conflicting attribute is here}}
  //expected-error@+3{{attributes are not compatible}}
  //expected-note@+1 {{conflicting attribute is here}}
  __attribute__((__numports_readonly_writeonly__(4,4)))
  __attribute__((__register__))
  unsigned int nprowo_one[4];

  //expected-error@+1{{attribute requires exactly 2 arguments}}
  __attribute__((__numports_readonly_writeonly__(4)))
  unsigned int nprowo_two[4];

  //expected-error@+1{{attribute requires exactly 2 arguments}}
  __attribute__((__numports_readonly_writeonly__(4,4,4)))
  unsigned int nprowo_three[4];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((__numports_readonly_writeonly__(-4,8)))
  unsigned int nprowo_four[4];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((__numports_readonly_writeonly__(4,0)))
  unsigned int nprowo_five[4];

  int i_nprowo = 32;
  //expected-error@+1{{expression is not an integer constant expression}}
  __attribute__((__numports_readonly_writeonly__(4,i_nprowo)))
  unsigned int nprowo_six[4];

  //CHECK: VarDecl{{.*}}nprowo_seven
  //CHECK: NumReadPortsAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}9{{$}}
  //CHECK-NEXT: NumWritePortsAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  //CHECK: NumReadPortsAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
  //CHECK-NEXT: NumWritePortsAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  //expected-warning@+3{{'numreadports' is already applied}}
  //expected-warning@+2{{'numwriteports' is already applied}}
  __attribute__((__numports_readonly_writeonly__(9,16)))
  __attribute__((__numports_readonly_writeonly__(2,4)))
  unsigned int nprowo_seven[4];

  // merge
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__merge__("mrg1","depth")))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int mrg_one[4];

  //expected-error@+1{{attribute requires a string}}
  __attribute__((__merge__(3,9.0f)))
  unsigned int mrg_two[4];

  //expected-error@+1{{attribute requires exactly 2 arguments}}
  __attribute__((__merge__("mrg2")))
  unsigned int mrg_three[4];

  //expected-error@+1{{attribute requires exactly 2 arguments}}
  __attribute__((__merge__("mrg3","depth","oops")))
  unsigned int mrg_four[4];

  //expected-error@+1{{merge direction must be 'depth' or 'width'}}
  __attribute__((__merge__("mrg4","depths")))
  unsigned int mrg_five[4];

  //Last one is applied and others ignored.
  //CHECK: VarDecl{{.*}}mrg_six
  //CHECK: MergeAttr{{.*}}"mrg4" "depth"{{$}}
  //CHECK: MergeAttr{{.*}}"mrg5" "width"{{$}}
  //expected-warning@+2{{attribute 'merge' is already applied}}
  __attribute__((__merge__("mrg4","depth")))
  __attribute__((__merge__("mrg5","width")))
  unsigned int mrg_six[4];

  // bank_bits
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__bank_bits__(2,3)))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int bb_one[4];

  //CHECK: VarDecl{{.*}}bb_two
  //CHECK: BankBitsAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}42{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}43{{$}}
  //CHECK: BankBitsAttr
  //CHECK-NEXT: IntegerLiteral{{.*}}1{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
  //expected-warning@+2{{attribute 'bank_bits' is already applied}}
  __attribute__((__bank_bits__(42,43)))
  __attribute__((__bank_bits__(1,2)))
  unsigned int bb_two[4];

  //expected-error@+1{{bank_bits must be equal to ceil(log2(numbanks))}}
  __attribute__((numbanks(8), bank_bits(3,4)))
  unsigned int bb_three[4];

  //expected-error@+1{{bank_bits must be equal to ceil(log2(numbanks))}}
  __attribute__((bank_bits(3,4), numbanks(8)))
  unsigned int bb_four[4];

  //expected-error@+1{{bank_bits must be consecutive}}
  __attribute__((bank_bits(3,3,4),bankwidth(4)))
  unsigned int bb_five[4];

  //expected-error@+1{{bank_bits must be consecutive}}
  __attribute__((bank_bits(1,3,4),bankwidth(4)))
  unsigned int bb_six[4];

  //expected-error@+1{{attribute takes at least 1 argument}}
  __attribute__((bank_bits))
  unsigned int bb_seven[4];

  //expected-error@+1{{attribute takes at least 1 argument}}
  __attribute__((bank_bits()))
  unsigned int bb_eight[4];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((bank_bits(0)))
  unsigned int bb_nine[4];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((bank_bits(-1)))
  unsigned int bb_ten[4];
}

//CHECK: FunctionDecl{{.*}}foo2
kernel void foo2(
  //expected-error-re@+2{{local or static variables{{$}}}}
  __local __attribute__((local_mem_size(1042)))
          __attribute__((singlepump)) int *a0,
  //expected-error-re@+1{{local or static variables{{$}}}}
  __local __attribute__((doublepump)) int *a1)
{
}
