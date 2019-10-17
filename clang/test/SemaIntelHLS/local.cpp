//RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

//CHECK: VarDecl{{.*}}global_const1
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: DoublePumpAttr
__attribute__((__doublepump__)) const int global_const1 = 1;

//CHECK: VarDecl{{.*}}global_const2
//CHECK: MemoryAttr
__attribute__((__memory__)) const int global_const2 = 1;

//CHECK: VarDecl{{.*}}global_const3
//CHECK: RegisterAttr
__attribute__((__register__)) const int global_const3 = 1;

//CHECK: VarDecl{{.*}}global_const4
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: SinglePumpAttr
__attribute__((__singlepump__)) const int global_const4 = 1;

//CHECK: VarDecl{{.*}}global_const5
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: BankWidthAttr
//CHECK: IntegerLiteral{{.*}}4{{$}}
__attribute__((__bankwidth__(4))) const int global_const5 = 1;

//CHECK: VarDecl{{.*}}global_const6
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: NumBanksAttr
//CHECK: IntegerLiteral{{.*}}8{{$}}
__attribute__((__numbanks__(8))) const int global_const6 = 1;

//CHECK: VarDecl{{.*}}global_const10
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: MergeAttr{{.*}}"mrg1" "depth"{{$}}
__attribute__((__merge__("mrg1", "depth"))) const int global_const10 = 1;

//CHECK: VarDecl{{.*}}global_const11
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: MergeAttr{{.*}}"mrg1" "width"{{$}}
__attribute__((__merge__("mrg1", "width"))) const int global_const11 = 1;

//CHECK: VarDecl{{.*}}global_const12
//CHECK: StaticArrayResetAttr
//CHECK: IntegerLiteral{{.*}}0{{$}}
__attribute__((__static_array_reset__(0))) const int global_const12 = 1;

//CHECK: VarDecl{{.*}}global_const13
//CHECK: StaticArrayResetAttr
//CHECK: IntegerLiteral{{.*}}1{{$}}
__attribute__((__static_array_reset__(1))) const int global_const13 = 1;

//CHECK: VarDecl{{.*}}global_const14
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: InternalMaxBlockRamDepthAttr
//CHECK: IntegerLiteral{{.*}}32{{$}}
__attribute__((internal_max_block_ram_depth(32))) const int global_const14 = 1;

//CHECK: VarDecl{{.*}}global_const17
//CHECK: NumBanksAttr{{.*}}Implicit{{$}}
//CHECK: IntegerLiteral{{.*}}16{{$}}
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: BankBitsAttr
//CHECK: IntegerLiteral{{.*}}2{{$}}
//CHECK: IntegerLiteral{{.*}}3{{$}}
//CHECK: IntegerLiteral{{.*}}4{{$}}
//CHECK: IntegerLiteral{{.*}}5{{$}}
__attribute__((__bank_bits__(2, 3, 4, 5))) const int global_const17 = 1;

//CHECK: FunctionDecl{{.*}}foo1
__attribute__((ihc_component))
void foo1()
{
  //CHECK: VarDecl{{.*}}v_one
  //CHECK: IntelFPGAMemoryAttr{{.*}}Implicit
  //CHECK: IntelFPGADoublePumpAttr
  __attribute__((__doublepump__))
  unsigned int v_one[64];

  //CHECK: VarDecl{{.*}}v_two
  //CHECK: IntelFPGAMemoryAttr
  __attribute__((__memory__))
  unsigned int v_two[64];

  //CHECK: VarDecl{{.*}}v_two_A
  //CHECK: IntelFPGAMemoryAttr{{.*}}MLAB{{$}}
  __attribute__((__memory__("MLAB")))
  unsigned int v_two_A[64];

  //CHECK: VarDecl{{.*}}v_two_B
  //CHECK: IntelFPGAMemoryAttr{{.*}}BlockRAM{{$}}
  __attribute__((__memory__("BLOCK_RAM")))
  unsigned int v_two_B[64];

  //CHECK: VarDecl{{.*}}v_two_C
  //CHECK: IntelFPGAMemoryAttr{{.*}}BlockRAM{{$}}
  //CHECK: IntelFPGADoublePumpAttr
  __attribute__((__memory__("BLOCK_RAM")))
  __attribute__((doublepump))
  unsigned int v_two_C[64];

  //CHECK: VarDecl{{.*}}v_three
  //CHECK: IntelFPGARegisterAttr
  __attribute__((__register__))
  unsigned int v_three[64];

  //CHECK: VarDecl{{.*}}v_four
  //CHECK: IntelFPGAMemoryAttr{{.*}}Implicit
  //CHECK: IntelFPGASinglePumpAttr
  __attribute__((__singlepump__))
  unsigned int v_four[64];

  //CHECK: VarDecl{{.*}}v_five
  //CHECK: IntelFPGAMemoryAttr{{.*}}Implicit
  //CHECK: IntelFPGABankWidthAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  __attribute__((__bankwidth__(4)))
  unsigned int v_five[64];

  //CHECK: VarDecl{{.*}}v_five_two
  //CHECK: IntelFPGAMemoryAttr{{.*}}Implicit
  //CHECK: MaxConcurrencyAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  __attribute__((max_concurrency(4)))
  unsigned int v_five_two[64];

  //CHECK: VarDecl{{.*}}v_six
  //CHECK: IntelFPGAMemoryAttr{{.*}}Implicit
  //CHECK: IntelFPGANumBanksAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}
  __attribute__((__numbanks__(8)))
  unsigned int v_six[64];

  //CHECK: VarDecl{{.*}}v_ten
  //CHECK: IntelFPGAMemoryAttr{{.*}}Implicit
  //CHECK: IntelFPGAMergeAttr{{.*}}"mrg1" "depth"{{$}}
  __attribute__((__merge__("mrg1","depth")))
  unsigned int v_ten[64];

  //CHECK: VarDecl{{.*}}v_eleven
  //CHECK: IntelFPGAMemoryAttr{{.*}}Implicit
  //CHECK: IntelFPGAMergeAttr{{.*}}"mrg2" "width"{{$}}
  __attribute__((__merge__("mrg2","width")))
  unsigned int v_eleven[64];

  //CHECK: VarDecl{{.*}}v_twelve
  //CHECK: IntelFPGANumBanksAttr{{.*}}Implicit{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  //CHECK: IntelFPGAMemoryAttr{{.*}}Implicit
  //CHECK: BankBitsAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}3{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}5{{$}}
  __attribute__((__bank_bits__(2,3,4,5)))
  unsigned int v_twelve[64];

  //CHECK: VarDecl{{.*}}v_twelve_A
  //CHECK: IntelFPGANumBanksAttr{{.*}}Implicit{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  //CHECK: IntelFPGAMemoryAttr{{.*}}Implicit
  //CHECK: BankBitsAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}5{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}3{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
  __attribute__((__bank_bits__(5,4,3,2)))
  unsigned int v_twelve_A[64];

  //CHECK: VarDecl{{.*}}v_thirteen
  //CHECK: IntelFPGANumBanksAttr{{.*}}Implicit{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  //CHECK: IntelFPGAMemoryAttr{{.*}}Implicit
  //CHECK: BankBitsAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}3{{$}}
  //CHECK: IntelFPGABankWidthAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  __attribute__((__bank_bits__(2,3), __bankwidth__(16)))
  unsigned int v_thirteen[64];

  //CHECK: VarDecl{{.*}}v_fourteen
  //CHECK: StaticArrayResetAttr
  //CHECK: IntegerLiteral{{.*}}0{{$}}
  __attribute__((__static_array_reset__(0)))
  static unsigned int v_fourteen[64];

  //CHECK: VarDecl{{.*}}v_fifteen
  //CHECK: StaticArrayResetAttr
  //CHECK: IntegerLiteral{{.*}}1{{$}}
  __attribute__((__static_array_reset__(1)))
  static unsigned int v_fifteen[64];

  int __attribute__((__register__)) A;
  int __attribute__((__numbanks__(4), __bankwidth__(16), __singlepump__)) B;
  int __attribute__((__numbanks__(4), __bankwidth__(16), __doublepump__)) C;
  int __attribute__((__numbanks__(4), __bankwidth__(16), __doublepump__)) D;
  int __attribute__((__numbanks__(4), __bankwidth__(16))) E;
  int __attribute__((__bank_bits__(2,3), __bankwidth__(16))) F;

  //CHECK: VarDecl{{.*}}G0
  //CHECK: IntelFPGAMemoryAttr{{.*}}Implicit
  //CHECK: InternalMaxBlockRamDepthAttr
  //CHECK: IntegerLiteral{{.*}}32{{$}}
  int __attribute__((internal_max_block_ram_depth(32))) G0;

  // diagnostics

  //expected-warning@+2{{'internal_max_block_ram_depth' is already applied}}
  __attribute__((internal_max_block_ram_depth(32)))
  __attribute__((internal_max_block_ram_depth(32)))
  int imbrd_one;

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((internal_max_block_ram_depth(32)))
  __attribute__((register))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int imbrd_four[64];

  // **doublepump
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__doublepump__))
  __attribute__((__singlepump__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int dp_one[64];

  //expected-warning@+1{{is already applied}}
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

  //expected-warning@+1{{is already applied}}
  __attribute__((singlepump))  __attribute__((__singlepump__))
  unsigned int sp_two[64];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__singlepump__))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int sp_three[64];

  // **register
  //expected-warning@+1{{is already applied}}
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
  __attribute__((__max_concurrency__(16)))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int reg_six_two[64];

  //expected-note@+1{{conflicting attribute is here}}
  __attribute__((__register__))
  //expected-error@+1{{'__numbanks__' and 'register' attributes are not compatible}}
  __attribute__((__numbanks__(8)))
  unsigned int reg_seven[64];

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

  //expected-warning@+1{{is already applied}}
  __attribute__((memory)) __attribute__((__memory__))
  unsigned int mem_two[64];

  //expected-warning@+1{{attribute 'memory' is already applied}}
  __attribute__((memory)) __attribute__((memory("MLAB")))
      unsigned int mem_three[64];

  //expected-warning@+1{{attribute 'memory' is already applied}}
  __attribute__((memory("BLOCK_RAM"))) __attribute__((memory("MLAB")))
      unsigned int mem_four[64];

  //expected-warning@+1{{is already applied}}
  __attribute__((memory("BLOCK_RAM"))) __attribute__((__memory__))
      unsigned int mem_five[64];

  //expected-error@+1{{requires either no argument or one of: MLAB BLOCK_RAM}}
  __attribute__((memory("")))
  unsigned int mem_eight[64];

  //expected-error@+1{{requires either no argument or one of: MLAB BLOCK_RAM}}
  __attribute__((memory("NLAB")))
  unsigned int mem_nine[64];

  // bankwidth
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__bankwidth__(16)))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int bw_one[64];

  //CHECK: VarDecl{{.*}}bw_two
  //CHECK: IntelFPGABankWidthAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}
  //CHECK: IntelFPGABankWidthAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  //expected-warning@+2{{is already applied}}
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
  //expected-error@+3{{is not an integral constant expression}}
  //expected-note@+2{{not allowed in a constant expression}}
  //expected-note@-3{{declared here}}
  __attribute__((__bankwidth__(i_bankwidth)))
  unsigned int bw_five[64];

  //expected-error@+1{{'__bankwidth__' attribute takes one argument}}
  __attribute__((__bankwidth__(4,8)))
  unsigned int bw_six[64];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((__bankwidth__(0)))
  unsigned int bw_seven[64];

  // max_concurrency
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__max_concurrency__(16)))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int mc_one[64];

  //CHECK: VarDecl{{.*}}mc_two
  //CHECK: IntelFPGAMemoryAttr
  //CHECK: MaxConcurrencyAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}
  //CHECK: MaxConcurrencyAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  //expected-warning@+2{{is already applied}}
  __attribute__((__max_concurrency__(8)))
  __attribute__((__max_concurrency__(16)))
  unsigned int mc_two[64];

  //expected-error@+1{{requires integer constant between 0 and 1048576}}
  __attribute__((__max_concurrency__(-4)))
  unsigned int mc_four[64];

  int i_max_concurrency = 32;
  //expected-error@+3{{is not an integral constant expression}}
  //expected-note@+2{{not allowed in a constant expression}}
  //expected-note@-3{{declared here}}
  __attribute__((__max_concurrency__(i_max_concurrency)))
  unsigned int mc_five[64];

  //expected-error@+1{{'__max_concurrency__' attribute takes one argument}}
  __attribute__((__max_concurrency__(4,8)))
  unsigned int mc_six[64];

  // numbanks
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__numbanks__(16)))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int nb_one[64];

  //CHECK: VarDecl{{.*}}nb_two
  //CHECK: IntelFPGANumBanksAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}
  //CHECK: IntelFPGANumBanksAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  //expected-warning@+2{{is already applied}}
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
  //expected-error@+3{{is not an integral constant expression}}
  //expected-note@+2{{is not allowed in a constant expression}}
  //expected-note@-3{{declared here}}
  __attribute__((__numbanks__(i_numbanks)))
  unsigned int nb_five[64];

  //expected-error@+1{{'__numbanks__' attribute takes one argument}}
  __attribute__((__numbanks__(4,8)))
  unsigned int nb_six[64];

  //expected-error@+1{{requires integer constant between 1 and 1048576}}
  __attribute__((__numbanks__(0)))
  unsigned int nb_seven[64];

  __attribute__((__register__))
  unsigned int nrp_one[4];

  // static_array_reset
  //expected-error@+1{{attribute only applies to constant variables, local static variables, and non-static data members}}
  __attribute__((static_array_reset(0)))
  unsigned int sar_one[8];

  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__static_array_reset__(0)))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  static unsigned int sar_two[4];

  //expected-error@+1{{requires integer constant between 0 and 1 inclusive}}
  __attribute__((__static_array_reset__(-1)))
  static unsigned int sar_three[4];

  //expected-error@+1{{requires integer constant between 0 and 1 inclusive}}
  __attribute__((__static_array_reset__(2)))
  static unsigned int sar_four[4];

  int i_nprowo = 32;
  unsigned int nprowo_six[4];


  // merge
  //expected-error@+2{{attributes are not compatible}}
  __attribute__((__merge__("mrg1","depth")))
  __attribute__((__register__))
  //expected-note@-2 {{conflicting attribute is here}}
  unsigned int mrg_one[4];

  //expected-error@+1{{attribute requires a string}}
  __attribute__((__merge__(3,9.0)))
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
  //CHECK: IntelFPGAMergeAttr{{.*}}"mrg4" "depth"{{$}}
  //CHECK: IntelFPGAMergeAttr{{.*}}"mrg5" "width"{{$}}
  //expected-warning@+2{{is already applied}}
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
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}42{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}43{{$}}
  //CHECK: BankBitsAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}1{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
  //expected-warning@+2{{is already applied}}
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
//CHECK: ComponentAttr
//CHECK: ComponentInterfaceAttr{{.*}}Implicit Streaming

//CHECK: FunctionTemplateDecl{{.*}}tattr
//CHECK: FunctionDecl{{.*}}tattr
//CHECK: TemplateArgument{{.*}}8{{$}}
//CHECK: TemplateArgument{{.*}}4{{$}}
//CHECK: TemplateArgument{{.*}}8{{$}}
//CHECK: TemplateArgument{{.*}}2{{$}}
//CHECK: TemplateArgument{{.*}}3{{$}}
//CHECK: TemplateArgument{{.*}}4{{$}}
//CHECK: VarDecl{{.*}}var1
//CHECK: MemoryAttr{{.*}}Implicit
//CHECK: BankBitsAttr
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: SubstNonTypeTemplateParmExpr
//CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: IntegerLiteral{{.*}}3{{$}}
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: SubstNonTypeTemplateParmExpr
//CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
//CHECK: NumBanksAttr
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: SubstNonTypeTemplateParmExpr
//CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}
//CHECK: BankWidthAttr
//CHECK-NEXT: ConstantExpr
//CHECK-NEXT: SubstNonTypeTemplateParmExpr
//CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
template <int max_concurrency, unsigned bankwidth, unsigned numbanks,
          int bit1, int bit2, int bit3>
void tattr() {

  //expected-error@+1{{'max_concurrency' attribute requires integer constant between 0 and 1048576 inclusive}}
  __attribute__((max_concurrency(max_concurrency)))
    //expected-error@+1{{'bankwidth' attribute argument must be a constant power of two greater than zero}}
  __attribute__((bankwidth(bankwidth)))
  __attribute__((numbanks(numbanks)))
  __attribute__((__bank_bits__(bit1,3,bit3)))
  int var1;
}

void foo2()
{
  tattr</*max_concurrency=*/8, /*bankwidth=*/4, /*numbanks=*/8,
        /*bit1=*/2, /*bit2=*/3, /*bit3=*/4>();

  //expected-note@+1{{in instantiation of function template specialization}}
  tattr</*max_concurrency=*/8, /*bankwidth=*/3, /*numbanks=*/8,
        /*bit1=*/2, /*bit2=*/3, /*bit3=*/4>();

  tattr</*max_concurrency=*/8, /*bankwidth=*/4, /*numbanks=*/8,
        /*bit1=*/2, /*bit2=*/3, /*bit3=*/4>();

  tattr</*max_concurrency=*/8, /*bankwidth=*/4, /*numbanks=*/8,
	/*bit1=*/4, /*bit2=*/3, /*bit3=*/2>();

  //expected-note@+1{{in instantiation of function template specialization}}
  tattr</*max_concurrency=*/-1, /*bankwidth=*/4, /*numbanks=*/8,
        /*bit1=*/2, /*bit2=*/3, /*bit3=*/4>();
}

template <typename T>
T type_temp(T t) {
  T __attribute__((bank_bits(1,2,3))) t1;
  return t+t1;
}

//CHECK: FunctionTemplateDecl{{.*}}type_temp{{$}}
//CHECK: FunctionDecl{{.*}}type_temp 'T (T)'{{$}}
//CHECK: FunctionDecl{{.*}}type_temp 'int (int)'{{$}}
//CHECK: TemplateArgument type 'int'
//CHECK: VarDecl{{.*}}t1 'int':'int'{{$}}
//CHECK: MemoryAttr
//CHECK: NumBanksAttr
//CHECK: IntegerLiteral{{.*}}8{{$}}
//CHECK: BankBitsAttr
//CHECK: IntegerLiteral{{.*}}1{{$}}
//CHECK: IntegerLiteral{{.*}}2{{$}}
//CHECK: IntegerLiteral{{.*}}3{{$}}
void other()
{
  int i = 1;
  type_temp(i);
}

//expected-error@+1{{attribute only applies to constant variables, local variables, static variables, slave memory arguments, and non-static data members}}
__attribute__((__doublepump__)) unsigned int ext_one[64];

//expected-error@+1{{only applies to functions and local non-const variables}}
__attribute__((__max_concurrency__(8))) unsigned int ext_two[64];

//expected-error@+1{{only applies to functions and local non-const variables}}
__attribute__((__max_concurrency__(8))) static unsigned int ext_three[64];

void other2()
{
  ext_three[0] = 1;
  //expected-error@+1{{only applies to functions and local non-const variables}}
  __attribute__((__max_concurrency__(8))) static unsigned int ext_four[64];
  ext_four[0] = 1;

  //expected-error@+1{{only applies to functions and local non-const variables}}
  __attribute__((__max_concurrency__(8))) const int ext_fix[64] = { 0 };
}

//expected-error@+1{{only applies to functions and local non-const variables}}
void other3(__attribute__((__max_concurrency__(8))) int pfoo) {}

//CHECK: CXXRecordDecl{{.*}}foo
struct foo {
  //CHECK: FieldDecl{{.*}}v_one
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: DoublePumpAttr
  __attribute__((__doublepump__)) unsigned int v_one[64];

  //CHECK: FieldDecl{{.*}}v_two
  //CHECK: MemoryAttr
  __attribute__((__memory__)) unsigned int v_two[64];

  //CHECK: FieldDecl{{.*}}v_two_A
  //CHECK: MemoryAttr{{.*}}MLAB{{$}}
  __attribute__((__memory__("MLAB"))) unsigned int v_two_A[64];

  //CHECK: FieldDecl{{.*}}v_two_B
  //CHECK: MemoryAttr{{.*}}BlockRAM{{$}}
  __attribute__((__memory__("BLOCK_RAM"))) unsigned int v_two_B[64];

  //CHECK: FieldDecl{{.*}}v_two_C
  //CHECK: MemoryAttr{{.*}}BlockRAM{{$}}
  //CHECK: DoublePumpAttr
  __attribute__((__memory__("BLOCK_RAM")))
  __attribute__((doublepump)) unsigned int v_two_C[64];

  //CHECK: FieldDecl{{.*}}v_two_D
  //CHECK: MemoryAttr{{.*}}MLAB{{$}}
  //CHECK: DoublePumpAttr
  __attribute__((__memory__("MLAB")))
  __attribute__((doublepump)) unsigned int v_two_D[64];

  //CHECK: FieldDecl{{.*}}v_two_E
  //CHECK: DoublePumpAttr
  //CHECK: MemoryAttr{{.*}}MLAB{{$}}
  __attribute__((doublepump))
  __attribute__((__memory__("MLAB"))) unsigned int v_two_E[64];

  //CHECK: FieldDecl{{.*}}v_three
  //CHECK: RegisterAttr
  __attribute__((__register__)) unsigned int v_three[64];

  //CHECK: FieldDecl{{.*}}v_four
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: SinglePumpAttr
  __attribute__((__singlepump__)) unsigned int v_four[64];

  //CHECK: FieldDecl{{.*}}v_five
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: BankWidthAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  __attribute__((__bankwidth__(4))) unsigned int v_five[64];

  //CHECK: FieldDecl{{.*}}v_six
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: NumBanksAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}8{{$}}
  __attribute__((__numbanks__(8))) unsigned int v_six[64];

  //CHECK: FieldDecl{{.*}}v_ten
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: MergeAttr{{.*}}"mrg1" "depth"{{$}}
  __attribute__((__merge__("mrg1", "depth"))) unsigned int v_ten[64];

  //CHECK: FieldDecl{{.*}}v_eleven
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: MergeAttr{{.*}}"mrg2" "width"{{$}}
  __attribute__((__merge__("mrg2", "width"))) unsigned int v_eleven[64];

  //CHECK: FieldDecl{{.*}}v_twelve
  //CHECK: NumBanksAttr{{.*}}Implicit{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: BankBitsAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}3{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}5{{$}}
  __attribute__((__bank_bits__(2, 3, 4, 5))) unsigned int v_twelve[64];

  //CHECK: FieldDecl{{.*}}v_thirteen
  //CHECK: NumBanksAttr{{.*}}Implicit{{$}}
  //CHECK-NEXT: IntegerLiteral{{.*}}4{{$}}
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: BankBitsAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}2{{$}}
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}3{{$}}
  //CHECK: BankWidthAttr
  //CHECK-NEXT: ConstantExpr
  //CHECK-NEXT: IntegerLiteral{{.*}}16{{$}}
  __attribute__((__bank_bits__(2, 3), __bankwidth__(16))) unsigned int v_thirteen[64];

  //CHECK: FieldDecl{{.*}}v_fourteen
  //CHECK: StaticArrayResetAttr
  //CHECK: IntegerLiteral{{.*}}0{{$}}
  __attribute__((__static_array_reset__(0))) unsigned int v_fourteen[64];

  //CHECK: FieldDecl{{.*}}v_fifteen
  //CHECK: StaticArrayResetAttr
  //CHECK: IntegerLiteral{{.*}}1{{$}}
  __attribute__((__static_array_reset__(1))) unsigned int v_fifteen[64];

  //CHECK: FieldDecl{{.*}}G0
  //CHECK: MemoryAttr{{.*}}Implicit
  //CHECK: InternalMaxBlockRamDepthAttr
  //CHECK: IntegerLiteral{{.*}}32{{$}}
  int __attribute__((internal_max_block_ram_depth(32))) G0;

};

//expected-error@+1{{attribute only applies to constant variables, local variables, static variables, slave memory arguments, and non-static data members}}
__attribute__((__memory__)) int ext_2;

//expected-error@+1{{attribute only applies to constant variables, local variables, static variables, and non-static data members}}
__attribute__((__register__)) int ext_3;

//expected-error@+1{{attribute only applies to constant variables, local variables, static variables, slave memory arguments, and non-static data members}}
__attribute__((__singlepump__)) int ext_4;

//expected-error@+1{{attribute only applies to constant variables, local variables, static variables, slave memory arguments, and non-static data members}}
__attribute__((__bankwidth__(4))) int ext_5;

//expected-error@+1{{attribute only applies to constant variables, local variables, static variables, slave memory arguments, and non-static data members}}
__attribute__((__numbanks__(8))) int ext_6;

//expected-error@+1{{attribute only applies to constant variables, local variables, static variables, and non-static data members}}
__attribute__((__merge__("mrg1", "depth"))) int ext_10;

//expected-error@+1{{attribute only applies to constant variables, local variables, static variables, and non-static data members}}
__attribute__((__merge__("mrg1", "width"))) int ext_11;

//expected-error@+1{{'__static_array_reset__' attribute only applies to constant variables, local static variables, and non-static data members}}
__attribute__((__static_array_reset__(0))) int ext_12;

//expected-error@+1{{'__static_array_reset__' attribute only applies to constant variables, local static variables, and non-static data members}}
__attribute__((__static_array_reset__(1))) int ext_13;

//expected-error@+1{{attribute only applies to constant variables, local variables, static variables, slave memory arguments, and non-static data members}}
__attribute__((internal_max_block_ram_depth(32))) int ext_14;

//expected-error@+1{{attribute only applies to constant variables, local variables, static variables, slave memory arguments, and non-static data members}}
__attribute__((__bank_bits__(2, 3, 4, 5))) int ext_17;
