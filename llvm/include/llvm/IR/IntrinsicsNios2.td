//===- IntrinsicsNios2.td - Defines Nios2 intrinsics -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines all of the Nios2-specific intrinsics.
//
//===----------------------------------------------------------------------===//

let TargetPrefix = "nios2" in {

  //===--------------------------------------------------------------------===//
  // Other intrinsics
  //===--------------------------------------------------------------------===//
  //int __builtin_ldbio(volatile const void *);
  def int_nios2_ldbio : GCCBuiltin<"__builtin_ldbio">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty], [IntrReadMem]>;
  //int __builtin_ldbuio(volatile const void *);
  def int_nios2_ldbuio : GCCBuiltin<"__builtin_ldbuio">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty], [IntrReadMem]>;
  //int __builtin_ldhio(volatile const void *);
  def int_nios2_ldhio : GCCBuiltin<"__builtin_ldhio">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty], [IntrReadMem]>;
  //int __builtin_ldhuio(volatile const void *);
  def int_nios2_ldhuio : GCCBuiltin<"__builtin_ldhuio">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty], [IntrReadMem]>;
  //int __builtin_ldwio(volatile const void *);
  def int_nios2_ldwio : GCCBuiltin<"__builtin_ldwio">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty], [IntrReadMem]>;
  //int __builtin_ldwuio(int);
  def int_nios2_ldwuio : GCCBuiltin<"__builtin_ldwuio">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty], [IntrReadMem]>;
  
  // int __builtin_rdctl(int);
  def int_nios2_rdctl : GCCBuiltin<"__builtin_rdctl">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty], []>;
  // void __builtin_wrctl(int, int);
  def int_nios2_wrctl : GCCBuiltin<"__builtin_wrctl">,
    Intrinsic<[], [llvm_i32_ty, llvm_i32_ty], []>;
  
  // int __builtin_rdprs(int, int);
  def int_nios2_rdprs : GCCBuiltin<"__builtin_rdprs">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty, llvm_i32_ty], []>;
  // int __builtin_wrpie(int);
  def int_nios2_wrpie : GCCBuiltin<"__builtin_wrpie">,
    Intrinsic<[llvm_i32_ty], [llvm_i32_ty], []>;
  
  //void __builtin_stbio(volatile void *, int);
  def int_nios2_stbio : GCCBuiltin<"__builtin_stbio">,
    Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty], [IntrWriteMem]>;
  //void __builtin_sthio(volatile void *, int);
  def int_nios2_sthio : GCCBuiltin<"__builtin_sthio">,
    Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty], [IntrWriteMem]>;
  //void __builtin_stwio(volatile void *, int);
  def int_nios2_stwio : GCCBuiltin<"__builtin_stwio">,
    Intrinsic<[], [llvm_ptr_ty, llvm_i32_ty], [IntrWriteMem]>;
  
  //void __builtin_sync(void);
  def int_nios2_sync : GCCBuiltin<"__builtin_sync">,
    Intrinsic<[], [], []>;
  
  // void __builtin_flushd(volatile void *);
  def int_nios2_flushd : GCCBuiltin<"__builtin_flushd">,
    Intrinsic<[], [llvm_ptr_ty], []>;
  // void __builtin_flushda(volatile void *);
  def int_nios2_flushda : GCCBuiltin<"__builtin_flushda">,
    Intrinsic<[], [llvm_ptr_ty], []>;
  
  // void __builtin_eni(int);
  def int_nios2_eni : GCCBuiltin<"__builtin_eni">,
    Intrinsic<[], [llvm_i32_ty], []>;
  
  // int __builtin_ldex(volatile const void *);
  def int_nios2_ldex : GCCBuiltin<"__builtin_ldex">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty], []>;
  // int __builtin_stex(volatile void *, int);
  def int_nios2_stex : GCCBuiltin<"__builtin_stex">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty, llvm_i32_ty], []>;
  // int __builtin_ldsex(volatile const void *);
  def int_nios2_ldsex : GCCBuiltin<"__builtin_ldsex">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty], []>;
  // int __builtin_stsex(volatile void *, int);
  def int_nios2_stsex : GCCBuiltin<"__builtin_stsex">,
    Intrinsic<[llvm_i32_ty], [llvm_ptr_ty, llvm_i32_ty], []>;
}
