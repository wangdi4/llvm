// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -x cl -triple x86_64-unknown-unknown-intelfpga -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s

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
  //CHECK: %[[VAR_TWO1:var_two[0-9]+]] = bitcast{{.*}}var_two
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_TWO1]],{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5))) var_two;
  //CHECK: %[[VAR_THREE:[0-9]+]] = bitcast{{.*}}var_three
  //CHECK: %[[VAR_THREE1:var_three[0-9]+]] = bitcast{{.*}}var_three
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_THREE1]],{{.*}}[[ANN2]]
  int __attribute__((numbanks(4),bank_bits(4,5))) var_three;
  //CHECK: %[[VAR_FOUR:[0-9]+]] = bitcast{{.*}}var_four
  //CHECK: %[[VAR_FOUR1:var_four[0-9]+]] = bitcast{{.*}}var_four
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_FOUR1]],{{.*}}[[ANN2]]
  int __attribute__((bank_bits(4,5),numbanks(4))) var_four;
  //CHECK: %[[VAR_FIVE:[0-9]+]] = bitcast{{.*}}var_five
  //CHECK: %[[VAR_FIVE1:var_five[0-9]+]] = bitcast{{.*}}var_five
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_FIVE1]],{{.*}}[[ANN3]]
  int __attribute__((numports_readonly_writeonly(2,3))) var_five;
  //CHECK: %[[VAR_SIX:[0-9]+]] = bitcast{{.*}}var_six
  //CHECK: %[[VAR_SIX1:var_six[0-9]+]] = bitcast{{.*}}var_six
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_SIX1]],{{.*}}[[ANN3]]
  int __attribute__((numreadports(2),numwriteports(3))) var_six;
  //CHECK: %[[VAR_SEVEN:[0-9]+]] = bitcast{{.*}}var_seven
  //CHECK: %[[VAR_SEVEN1:var_seven[0-9]+]] = bitcast{{.*}}var_seven
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_SEVEN1]],{{.*}}[[ANN4]]
  int __attribute__((register)) var_seven;
  //CHECK: %[[VAR_EIGHT:[0-9]+]] = bitcast{{.*}}var_eight
  //CHECK: %[[VAR_EIGHT1:var_eight[0-9]+]] = bitcast{{.*}}var_eight
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_EIGHT1]],{{.*}}[[ANN5]]
  int __attribute__((__memory__)) var_eight;
  //CHECK: %[[VAR_NINE:[0-9]+]] = bitcast{{.*}}var_nine
  //CHECK: %[[VAR_NINE1:var_nine[0-9]+]] = bitcast{{.*}}var_nine
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_NINE1]],{{.*}}[[ANN6]]
  int __attribute__((__bankwidth__(4))) var_nine;
  //CHECK: %[[VAR_TEN:[0-9]+]] = bitcast{{.*}}var_ten
  //CHECK: %[[VAR_TEN1:var_ten[0-9]+]] = bitcast{{.*}}var_ten
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_TEN1]],{{.*}}[[ANN7]]
  int __attribute__((singlepump)) var_ten;
  //CHECK: %[[VAR_ELEVEN:[0-9]+]] = bitcast{{.*}}var_eleven
  //CHECK: %[[VAR_ELEVEN1:var_eleven[0-9]+]] = bitcast{{.*}}var_eleven
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_ELEVEN1]],{{.*}}[[ANN8]]
  int __attribute__((doublepump)) var_eleven;
  //CHECK: %[[VAR_TWELVE:[0-9]+]] = bitcast{{.*}}var_twelve
  //CHECK: %[[VAR_TWELVE1:var_twelve[0-9]+]] = bitcast{{.*}}var_twelve
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_TWELVE1]],{{.*}}[[ANN9]]
  int __attribute__((merge("foo","depth"))) var_twelve;
  //CHECK: %[[VAR_THIRTEEN:[0-9]+]] = bitcast{{.*}}var_thirteen
  //CHECK: %[[VAR_THIRTEEN1:var_thirteen[0-9]+]] = bitcast{{.*}}var_thirteen
  //CHECK: llvm.var.annotation{{.*}}%[[VAR_THIRTEEN1]],{{.*}}[[ANN10]]
  int __attribute__((merge("bar","width"))) var_thirteen;
}

