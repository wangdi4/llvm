//RUN: %clang_cc1 -fhls -emit-llvm -o - %s | FileCheck %s
//RUN: %clang_cc1 -fhls -debug-info-kind=limited -emit-llvm -o %t %s

//CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{numbanks:4}{bank_bits:4,5}
//CHECK: [[ANN2A:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{numbanks:4}{bank_bits:5,4}
//CHECK: [[ANN3:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{numreadports:2}{numwriteports:3}
//CHECK: [[ANN4:@.str[\.]*[0-9]*]] = {{.*}}{register:1}
//CHECK: [[ANN5:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}
//CHECK: [[ANN5A:@.str[\.]*[0-9]*]] = {{.*}}{memory:MLAB}
//CHECK: [[ANN5B:@.str[\.]*[0-9]*]] = {{.*}}{memory:BLOCK_RAM}
//CHECK: [[ANN6:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{bankwidth:4}
//CHECK: [[ANN7:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{pump:1}
//CHECK: [[ANN8:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{pump:2}
//CHECK: [[ANN9:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{merge:foo:depth}
//CHECK: [[ANN10:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{merge:bar:width}
//CHECK: [[ANN11:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{optimize_fmax:1}
//CHECK: [[ANN12:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{optimize_ram_usage:1}
//CHECK: [[ANN1:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{pump:1}{bankwidth:4}{numbanks:8}{numreadports:2}{numwriteports:3}{bank_bits:2,3,4}{merge:merge_foo_one:depth}
//CHECK: [[ANN1A:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{numbanks:8}{bank_bits:4,3,2}

__attribute__((ihc_component))
void foo_two() {
  //CHECK: llvm.var.annotation{{.*}}var_two{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5))) var_two;
  //CHECK: llvm.var.annotation{{.*}}var_three{{.*}}[[ANN2]]
  int __attribute__((numbanks(4),bank_bits(4,5))) var_three;
  //CHECK: llvm.var.annotation{{.*}}var_four{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5),numbanks(4))) var_four;
  //CHECK: llvm.var.annotation{{.*}}var_four_A{{.*}}[[ANN2A]]
  int __attribute__((bank_bits(5,4),numbanks(4))) var_four_A;
  //CHECK: llvm.var.annotation{{.*}}var_five{{.*}}[[ANN3]]
  int __attribute__((numports_readonly_writeonly(2,3))) var_five;
  //CHECK: llvm.var.annotation{{.*}}var_six{{.*}}[[ANN3]]
  int __attribute__((numreadports(2),numwriteports(3))) var_six;
  //CHECK: llvm.var.annotation{{.*}}var_seven{{.*}}[[ANN4]]
  int __attribute__((register)) var_seven;
  //CHECK: llvm.var.annotation{{.*}}var_eight{{.*}}[[ANN5]]
  int __attribute__((__memory__)) var_eight;
  //CHECK: llvm.var.annotation{{.*}}var_eightA{{.*}}[[ANN5A]]
  int __attribute__((__memory__("MLAB"))) var_eightA;
  //CHECK: llvm.var.annotation{{.*}}var_eightB{{.*}}[[ANN5B]]
  int __attribute__((__memory__("BLOCK_RAM"))) var_eightB;
  //CHECK: llvm.var.annotation{{.*}}var_nine{{.*}}[[ANN6]]
  int __attribute__((__bankwidth__(4))) var_nine;
  //CHECK: llvm.var.annotation{{.*}}var_ten{{.*}}[[ANN7]]
  int __attribute__((singlepump)) var_ten;
  //CHECK: llvm.var.annotation{{.*}}var_eleven{{.*}}[[ANN8]]
  int __attribute__((doublepump)) var_eleven;
  //CHECK: llvm.var.annotation{{.*}}var_twelve{{.*}}[[ANN9]]
  int __attribute__((merge("foo","depth"))) var_twelve;
  //CHECK: llvm.var.annotation{{.*}}var_thirteen{{.*}}[[ANN10]]
  int __attribute__((merge("bar","width"))) var_thirteen;
  //CHECK: llvm.var.annotation{{.*}}var_forteen{{.*}}[[ANN11]]
  int __attribute__((optimize_fmax)) var_forteen;
  //CHECK: llvm.var.annotation{{.*}}var_fifteen{{.*}}[[ANN12]]
  int __attribute__((optimize_ram_usage)) var_fifteen;
}

template <int bankwidth, int numbanks, int readports, int writeports,
          int bit1, int bit2, int bit3>
__attribute__((ihc_component))
void foo_one()
{
  __attribute__((bankwidth(bankwidth),numbanks(numbanks),
                 numports_readonly_writeonly(readports,writeports),
                 merge("merge_foo_one","depth"), memory, singlepump,
                 __bank_bits__(bit1,bit2,bit3)))
  int var_one;
}

//CHECK: define{{.*}}foo_one{{.*}}!ihc_component
//CHECK: llvm.var.annotation{{.*}}var_one{{.*}}[[ANN1]]

template <int numbanks, int bit>
__attribute__((ihc_component))
void foo_two()
{
  __attribute__((numbanks(numbanks), __bank_bits__(4,bit,2)))
  int var_two;
}

//CHECK: define{{.*}}foo_two{{.*}}!ihc_component
//CHECK: llvm.var.annotation{{.*}}var_two{{.*}}[[ANN1A]]

void call()
{
  foo_one<4,8,2,3,2,3,4>();
  foo_two<8,3>();
}
