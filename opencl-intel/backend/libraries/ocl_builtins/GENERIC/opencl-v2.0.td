let Types = [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
              v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64],
    Attrs = [CONSTFUNC, OVERLOADABLE] in {
let NeedForwardDecl = 1 in {
def ctz       : OclBuiltin<"ctz",       (outs gentype:$ret), (ins gentype:$x), [], [], 1, 0>;
}
}

// native relaxed math built-ins OpenCL 2.0 rev 08 Table 7.2, single-precision only
let Types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32],
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = 1 in {
def cos_rm    : OclBuiltin<"cos_rm",    (outs gentype:$ret), (ins gentype:$x), [], [], 1, 0>;
//def divide_rm : OclBuiltin<"divide_rm", (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 1, 0>;
def exp_rm    : OclBuiltin<"exp_rm",    (outs gentype:$ret), (ins gentype:$x), [], [], 1, 0>;
def exp2_rm   : OclBuiltin<"exp2_rm",   (outs gentype:$ret), (ins gentype:$x), [], [], 1, 0>;
def exp10_rm  : OclBuiltin<"exp10_rm",  (outs gentype:$ret), (ins gentype:$x), [], [], 1, 0>;
def log_rm    : OclBuiltin<"log_rm",    (outs gentype:$ret), (ins gentype:$x), [], [], 1, 0>;
def log2_rm   : OclBuiltin<"log2_rm",   (outs gentype:$ret), (ins gentype:$x), [], [], 1, 0>;
def pow_rm   : OclBuiltin<"pow_rm",   (outs gentype:$ret), (ins gentype:$x, gentype:$y), [], [], 1, 0>;
def sin_rm    : OclBuiltin<"sin_rm",    (outs gentype:$ret), (ins gentype:$x), [], [], 1, 0>;
def tan_rm    : OclBuiltin<"tan_rm",    (outs gentype:$ret), (ins gentype:$x), [], [], 1, 0>;
def sincos_pas_rm    : OclBuiltin<"sincos_rm",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], 1, 0>, ExceptAttr<[CONSTFUNC]>, PRIVATE;
def sincos_las_rm    : OclBuiltin<"sincos_rm",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], 1, 0>, ExceptAttr<[CONSTFUNC]>, LOCAL;
def sincos_gas_rm    : OclBuiltin<"sincos_rm",    (outs gentype:$ret), (ins gentype:$x, pgentype:$cosval), [], [], 1, 0>, ExceptAttr<[CONSTFUNC]>, GLOBAL;
}

// Implementation of atomic builtins relies on "generic address space" optimization passes
// There are two of them: Static and Dynamic
// 1. Static pass is to replace calls to __generic builtin with the source pointer's address space
// 2. Dynamic pass is to replace all calls which static couldn't.
//    For CPU it will replace them to __global address space builtins (the most strict for atomics)
// None of __generic address space builtins must survive the passes.  

let Types = [v1i32, v1u32, v1f32],
    Attrs = [OVERLOADABLE], NeedForwardDecl = 1 in {
def atomic_init_g : OclBuiltin<"atomic_init",  (outs),
    (ins patomgentype:$object, gentype:$value), [], [], 1, 0>, VOLATILE_GLOBAL;

def atomic_store_g : OclBuiltin<"atomic_store",  (outs),
    (ins patomgentype:$object, gentype:$desired), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_store_explicit_g : OclBuiltin<"atomic_store_explicit",  (outs),
    (ins patomgentype:$object, gentype:$desired, memory_order:$order), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_store_explicit_scope_g : OclBuiltin<"atomic_store_explicit",  (outs),
    (ins patomgentype:$object, gentype:$desired, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_GLOBAL;

def atomic_load_g : OclBuiltin<"atomic_load",  (outs gentype:$ret),
    (ins patomgentype:$object), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_load_explicit_g : OclBuiltin<"atomic_load_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, memory_order:$order), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_load_explicit_scope_g : OclBuiltin<"atomic_load_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_GLOBAL;

def atomic_exchange_g : OclBuiltin<"atomic_exchange",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$desired), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_exchange_explicit_g : OclBuiltin<"atomic_exchange_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$desired, memory_order:$order), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_exchange_explicit_scope_g : OclBuiltin<"atomic_exchange_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$desired, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_GLOBAL;

// static pass for generic address space requires us to have all combinations of pointers in argument list
// SECPTR is used to build gpgentype, lpgentype, ppgentype, and add posfix _gp, _lp, _pp to OclBuiltin define
// VOLATILE_GLOBAL is applied to all arguments pointers of the function. Thus, it is not allowed
// to use it on function that takes two pointer arguments with different address space or qualifiers.
foreach SECPTR = ["gp", "lp", "pp"] in {
  def atomic_compare_exchange_strong_g_#SECPTR : OclBuiltin<"atomic_compare_exchange_strong",  (outs bool:$ret),
      (ins vgpatomgentype:$object, !cast<OclGenType>(SECPTR#gentype):$expected, gentype:$desired), [], [], 1, 0>;
  def atomic_compare_exchange_strong_explicit_g_#SECPTR : OclBuiltin<"atomic_compare_exchange_strong_explicit",  (outs bool:$ret),
      (ins vgpatomgentype:$object, !cast<OclGenType>(SECPTR#gentype):$expected, gentype:$desired, memory_order:$success, memory_order:$failure), [], [], 1, 0>;
  def atomic_compare_exchange_strong_explicit_scope_g_#SECPTR : OclBuiltin<"atomic_compare_exchange_strong_explicit",  (outs bool:$ret),
      (ins vgpatomgentype:$object, !cast<OclGenType>(SECPTR#gentype):$expected, gentype:$desired, memory_order:$success, memory_order:$failure, memory_scope:$scope), [], [], 1, 0>;
  
  def atomic_compare_exchange_weak_g_#SECPTR : OclBuiltin<"atomic_compare_exchange_weak",  (outs bool:$ret),
      (ins vgpatomgentype:$object, !cast<OclGenType>(SECPTR#gentype):$expected, gentype:$desired), [], [], 1, 0>;
  def atomic_compare_exchange_weak_explicit_g_#SECPTR : OclBuiltin<"atomic_compare_exchange_weak_explicit",  (outs bool:$ret),
      (ins vgpatomgentype:$object, !cast<OclGenType>(SECPTR#gentype):$expected, gentype:$desired, memory_order:$success, memory_order:$failure), [], [], 1, 0>;
  def atomic_compare_exchange_weak_explicit_scope_g_#SECPTR : OclBuiltin<"atomic_compare_exchange_weak_explicit",  (outs bool:$ret),
      (ins vgpatomgentype:$object, !cast<OclGenType>(SECPTR#gentype):$expected, gentype:$desired, memory_order:$success, memory_order:$failure, memory_scope:$scope), [], [], 1, 0>;
  }
}

let Types = [v1i32, v1u32],
    Attrs = [OVERLOADABLE], NeedForwardDecl = 1 in {
def atomic_fetch_add_g : OclBuiltin<"atomic_fetch_add",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_add_explicit_g : OclBuiltin<"atomic_fetch_add_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_add_explicit_scope_g : OclBuiltin<"atomic_fetch_add_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_GLOBAL;

def atomic_fetch_sub_g : OclBuiltin<"atomic_fetch_sub",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_sub_explicit_g : OclBuiltin<"atomic_fetch_sub_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_sub_explicit_scope_g : OclBuiltin<"atomic_fetch_sub_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_GLOBAL;

def atomic_fetch_or_g : OclBuiltin<"atomic_fetch_or",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_or_explicit_g : OclBuiltin<"atomic_fetch_or_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_or_explicit_scope_g : OclBuiltin<"atomic_fetch_or_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_GLOBAL;

def atomic_fetch_xor_g : OclBuiltin<"atomic_fetch_xor",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_xor_explicit_g : OclBuiltin<"atomic_fetch_xor_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_xor_explicit_scope_g : OclBuiltin<"atomic_fetch_xor_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_GLOBAL;

def atomic_fetch_and_g : OclBuiltin<"atomic_fetch_and",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_and_explicit_g : OclBuiltin<"atomic_fetch_and_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_and_explicit_scope_g : OclBuiltin<"atomic_fetch_and_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_GLOBAL;

def atomic_fetch_min_g : OclBuiltin<"atomic_fetch_min",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_min_explicit_g : OclBuiltin<"atomic_fetch_min_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_min_explicit_scope_g : OclBuiltin<"atomic_fetch_min_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_GLOBAL;

def atomic_fetch_max_g : OclBuiltin<"atomic_fetch_max",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_max_explicit_g : OclBuiltin<"atomic_fetch_max_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_fetch_max_explicit_scope_g : OclBuiltin<"atomic_fetch_max_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_GLOBAL;
}

// atomic_flag
let Types = [atom_flag],
    Attrs = [OVERLOADABLE], NeedForwardDecl = 1 in {
def atomic_flag_test_and_set_g : OclBuiltin<"atomic_flag_test_and_set",  (outs bool:$ret),
    (ins pgentype:$object), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_flag_test_and_set_explicit_g : OclBuiltin<"atomic_flag_test_and_set_explicit",  (outs bool:$ret),
    (ins pgentype:$object, memory_order:$order), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_flag_test_and_set_explicit_scope_g : OclBuiltin<"atomic_flag_test_and_set_explicit",  (outs bool:$ret),
    (ins pgentype:$object, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_GLOBAL;

def atomic_flag_clear_g : OclBuiltin<"atomic_flag_clear",  (outs),
    (ins pgentype:$object), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_flag_clear_explicit_g : OclBuiltin<"atomic_flag_clear_explicit",  (outs),
    (ins pgentype:$object, memory_order:$order), [], [], 1, 0>, VOLATILE_GLOBAL;
def atomic_flag_clear_explicit_scope_g : OclBuiltin<"atomic_flag_clear_explicit",  (outs),
  (ins pgentype:$object, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_GLOBAL;
}

// VOLATILE_LOCAL
let Types = [v1i32, v1u32, v1f32],
    Attrs = [OVERLOADABLE], NeedForwardDecl = 1 in {
def atomic_init_l : OclBuiltin<"atomic_init",  (outs),
    (ins patomgentype:$object, gentype:$value), [], [], 1, 0>, VOLATILE_LOCAL;

def atomic_store_l : OclBuiltin<"atomic_store",  (outs),
    (ins patomgentype:$object, gentype:$desired), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_store_explicit_l : OclBuiltin<"atomic_store_explicit",  (outs),
    (ins patomgentype:$object, gentype:$desired, memory_order:$order), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_store_explicit_scope_l : OclBuiltin<"atomic_store_explicit",  (outs),
    (ins patomgentype:$object, gentype:$desired, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_LOCAL;

def atomic_load_l : OclBuiltin<"atomic_load",  (outs gentype:$ret),
    (ins patomgentype:$object), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_load_explicit_l : OclBuiltin<"atomic_load_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, memory_order:$order), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_load_explicit_scope_l : OclBuiltin<"atomic_load_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_LOCAL;

def atomic_exchange_l : OclBuiltin<"atomic_exchange",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$desired), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_exchange_explicit_l : OclBuiltin<"atomic_exchange_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$desired, memory_order:$order), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_exchange_explicit_scope_l : OclBuiltin<"atomic_exchange_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$desired, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_LOCAL;

// static pass for generic address space requires us to have all combinations of pointers in argument list
// SECPTR is used to build gpgentype, lpgentype, ppgentype, and add posfix _gp, _lp, _pp to OclBuiltin define
// VOLATILE_LOCAL is applied to all arguments pointers of the function. Thus, it is not allowed
// to use it on function that takes two pointer arguments with different address space or qualifiers.
foreach SECPTR = ["gp", "lp", "pp"] in {
  def atomic_compare_exchange_strong_l_#SECPTR : OclBuiltin<"atomic_compare_exchange_strong",  (outs bool:$ret),
      (ins vlpatomgentype:$object, !cast<OclGenType>(SECPTR#gentype):$expected, gentype:$desired), [], [], 1, 0>;
  def atomic_compare_exchange_strong_explicit_l_#SECPTR : OclBuiltin<"atomic_compare_exchange_strong_explicit",  (outs bool:$ret),
      (ins vlpatomgentype:$object, !cast<OclGenType>(SECPTR#gentype):$expected, gentype:$desired, memory_order:$success, memory_order:$failure), [], [], 1, 0>;
  def atomic_compare_exchange_strong_explicit_scope_l_#SECPTR : OclBuiltin<"atomic_compare_exchange_strong_explicit",  (outs bool:$ret),
      (ins vlpatomgentype:$object, !cast<OclGenType>(SECPTR#gentype):$expected, gentype:$desired, memory_order:$success, memory_order:$failure, memory_scope:$scope), [], [], 1, 0>;
  
  def atomic_compare_exchange_weak_l_#SECPTR : OclBuiltin<"atomic_compare_exchange_weak",  (outs bool:$ret),
      (ins vlpatomgentype:$object, !cast<OclGenType>(SECPTR#gentype):$expected, gentype:$desired), [], [], 1, 0>;
  def atomic_compare_exchange_weak_explicit_l_#SECPTR : OclBuiltin<"atomic_compare_exchange_weak_explicit",  (outs bool:$ret),
      (ins vlpatomgentype:$object, !cast<OclGenType>(SECPTR#gentype):$expected, gentype:$desired, memory_order:$success, memory_order:$failure), [], [], 1, 0>;
  def atomic_compare_exchange_weak_explicit_scope_l_#SECPTR : OclBuiltin<"atomic_compare_exchange_weak_explicit",  (outs bool:$ret),
      (ins vlpatomgentype:$object, !cast<OclGenType>(SECPTR#gentype):$expected, gentype:$desired, memory_order:$success, memory_order:$failure, memory_scope:$scope), [], [], 1, 0>;
  }
}

let Types = [v1i32, v1u32],
    Attrs = [OVERLOADABLE], NeedForwardDecl = 1 in {
def atomic_fetch_add_l : OclBuiltin<"atomic_fetch_add",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_add_explicit_l : OclBuiltin<"atomic_fetch_add_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_add_explicit_scope_l : OclBuiltin<"atomic_fetch_add_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_LOCAL;

def atomic_fetch_sub_l : OclBuiltin<"atomic_fetch_sub",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_sub_explicit_l : OclBuiltin<"atomic_fetch_sub_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_sub_explicit_scope_l : OclBuiltin<"atomic_fetch_sub_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_LOCAL;

def atomic_fetch_or_l : OclBuiltin<"atomic_fetch_or",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_or_explicit_l : OclBuiltin<"atomic_fetch_or_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_or_explicit_scope_l : OclBuiltin<"atomic_fetch_or_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_LOCAL;

def atomic_fetch_xor_l : OclBuiltin<"atomic_fetch_xor",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_xor_explicit_l : OclBuiltin<"atomic_fetch_xor_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_xor_explicit_scope_l : OclBuiltin<"atomic_fetch_xor_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_LOCAL;

def atomic_fetch_and_l : OclBuiltin<"atomic_fetch_and",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_and_explicit_l : OclBuiltin<"atomic_fetch_and_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_and_explicit_scope_l : OclBuiltin<"atomic_fetch_and_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_LOCAL;

def atomic_fetch_min_l : OclBuiltin<"atomic_fetch_min",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_min_explicit_l : OclBuiltin<"atomic_fetch_min_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_min_explicit_scope_l : OclBuiltin<"atomic_fetch_min_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_LOCAL;

def atomic_fetch_max_l : OclBuiltin<"atomic_fetch_max",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_max_explicit_l : OclBuiltin<"atomic_fetch_max_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_fetch_max_explicit_scope_l : OclBuiltin<"atomic_fetch_max_explicit",  (outs gentype:$ret),
    (ins patomgentype:$object, gentype:$operand, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_LOCAL;
}

// atomic_flag, atomic_work_item_fence, and atomic_work_item_image_fence builtins
let Types = [atom_flag],
    Attrs = [OVERLOADABLE], NeedForwardDecl = 1 in {
def atomic_flag_test_and_set_l : OclBuiltin<"atomic_flag_test_and_set",  (outs bool:$ret),
    (ins pgentype:$object), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_flag_test_and_set_explicit_l : OclBuiltin<"atomic_flag_test_and_set_explicit",  (outs bool:$ret),
    (ins pgentype:$object, memory_order:$order), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_flag_test_and_set_explicit_scope_l : OclBuiltin<"atomic_flag_test_and_set_explicit",  (outs bool:$ret),
    (ins pgentype:$object, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_LOCAL;

def atomic_flag_clear_l : OclBuiltin<"atomic_flag_clear",  (outs),
    (ins pgentype:$object), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_flag_clear_explicit_l : OclBuiltin<"atomic_flag_clear_explicit",  (outs),
    (ins pgentype:$object, memory_order:$order), [], [], 1, 0>, VOLATILE_LOCAL;
def atomic_flag_clear_explicit_scope_l : OclBuiltin<"atomic_flag_clear_explicit",  (outs),
  (ins pgentype:$object, memory_order:$order, memory_scope:$scope), [], [], 1, 0>, VOLATILE_LOCAL;

def atomic_work_item_fence_ord : OclBuiltin<"atomic_work_item_fence", (outs), (ins memory_order:$order), [], [], 1, 0>;
def atomic_work_item_fence_ord_sco : OclBuiltin<"atomic_work_item_fence", (outs), (ins memory_order:$order, memory_scope:$scope), [], [], 1, 0>;
def atomic_work_item_fence_flag_ord : OclBuiltin<"atomic_work_item_fence", (outs), (ins cl_mem_fence_flags:$flags, memory_order:$order), [], [], 1, 0>;
def atomic_work_item_fence_flag_ord_sco : OclBuiltin<"atomic_work_item_fence", (outs), (ins cl_mem_fence_flags:$flags, memory_order:$order, memory_scope:$scope), [], [], 1, 0>;

def atomic_work_item_image_fence : OclBuiltin<"atomic_work_item_image_fence", (outs), (ins memory_scope:$scope), [], [], 1, 0>;
}
