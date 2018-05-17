// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -x cl -triple x86_64-unknown-unknown-intelfpga -emit-llvm %s -o - | FileCheck %s

//CHECK: [[ANN2:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{numbanks:4}{bank_bits:4,5}
//CHECK: [[ANN3:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{numreadports:2}{numwriteports:3}
//CHECK: [[ANN4:@.str[\.]*[0-9]*]] = {{.*}}{register:1}
//CHECK: [[ANN5:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}
//CHECK: [[ANN6:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{bankwidth:4}
//CHECK: [[ANN7:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{pump:1}
//CHECK: [[ANN8:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{pump:2}
//CHECK: [[ANN9:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{merge:foo:depth}
//CHECK: [[ANN10:@.str[\.]*[0-9]*]] = {{.*}}{memory:DEFAULT}{merge:bar:width}

//__attribute__((ihc_component))
void foo_two() {
  //CHECK: %[[VAR_TWO:[0-9]+]] = bitcast{{.*}}var_two
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_TWO]],{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5))) var_two;
  //CHECK: %[[VAR_THREE:[0-9]+]] = bitcast{{.*}}var_three
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_THREE]],{{.*}}[[ANN2]]
  int __attribute__((numbanks(4),bank_bits(4,5))) var_three;
  //CHECK: %[[VAR_FOUR:[0-9]+]] = bitcast{{.*}}var_four
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_FOUR]],{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5),numbanks(4))) var_four;
  //CHECK: %[[VAR_FIVE:[0-9]+]] = bitcast{{.*}}var_five
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_FIVE]],{{.*}}[[ANN3]]
  int __attribute__((numports_readonly_writeonly(2,3))) var_five;
  //CHECK: %[[VAR_SIX:[0-9]+]] = bitcast{{.*}}var_six
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_SIX]],{{.*}}[[ANN3]]
  int __attribute__((numreadports(2),numwriteports(3))) var_six;
  //CHECK: %[[VAR_SEVEN:[0-9]+]] = bitcast{{.*}}var_seven
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_SEVEN]],{{.*}}[[ANN4]]
  int __attribute__((register)) var_seven;
  //CHECK: %[[VAR_EIGHT:[0-9]+]] = bitcast{{.*}}var_eight
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_EIGHT]],{{.*}}[[ANN5]]
  int __attribute__((__memory__)) var_eight;
  //CHECK: %[[VAR_NINE:[0-9]+]] = bitcast{{.*}}var_nine
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_NINE]],{{.*}}[[ANN6]]
  int __attribute__((__bankwidth__(4))) var_nine;
  //CHECK: %[[VAR_TEN:[0-9]+]] = bitcast{{.*}}var_ten
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_TEN]],{{.*}}[[ANN7]]
  int __attribute__((singlepump)) var_ten;
  //CHECK: %[[VAR_ELEVEN:[0-9]+]] = bitcast{{.*}}var_eleven
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_ELEVEN]],{{.*}}[[ANN8]]
  int __attribute__((doublepump)) var_eleven;
  //CHECK: %[[VAR_TWELVE:[0-9]+]] = bitcast{{.*}}var_twelve
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_TWELVE]],{{.*}}[[ANN9]]
  int __attribute__((merge("foo","depth"))) var_twelve;
  //CHECK: %[[VAR_THIRTEEN:[0-9]+]] = bitcast{{.*}}var_thirteen
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_THIRTEEN]],{{.*}}[[ANN10]]
  int __attribute__((merge("bar","width"))) var_thirteen;
}

