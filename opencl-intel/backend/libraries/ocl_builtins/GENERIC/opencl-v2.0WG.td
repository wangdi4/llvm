
// Work-group functions and their vectorized versions
let NeedForwardDecl = 1 in {
  let Types = [ v1i32,  v4i32,  v8i32,  v16i32 ],
      Attrs = [OVERLOADABLE] in {
    def work_group_all : OclBuiltin<"work_group_all", (outs gentype:$ret), (ins gentype:$predicate), [], [], 1, 0>;
    def work_group_any : OclBuiltin<"work_group_any", (outs gentype:$ret), (ins gentype:$predicate), [], [], 1, 0>;
  }

  let Types = [ v1i32, v4i32, v8i32, v16i32,
                v1u32, v4u32, v8u32, v16u32,
                v1i64, v4i64, v8i64, v16i64,
                v1u64, v4u64, v8u64, v16u64,
                v1f32, v4f32, v8f32, v16f32,
                v1f64, v4f64, v8f64, v16f64],
      Attrs = [OVERLOADABLE] in {
    def work_group_broadcast1d        : OclBuiltin<"work_group_broadcast",     (outs gentype:$ret), (ins gentype:$src, size_t:$local_id), [], [], 1, 0>;
    def work_group_broadcast2d        : OclBuiltin<"work_group_broadcast",     (outs gentype:$ret), (ins gentype:$src, size_t:$local_id_x, size_t:$local_id_y), [], [], 1, 0>;
    def work_group_broadcast3d        : OclBuiltin<"work_group_broadcast",     (outs gentype:$ret), (ins gentype:$src, size_t:$local_id_x, size_t:$local_id_y, size_t:$local_id_z), [], [], 1, 0>;
    def work_group_reduce_add         : OclBuiltin<"work_group_reduce_add",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_reduce_min         : OclBuiltin<"work_group_reduce_min",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_reduce_max         : OclBuiltin<"work_group_reduce_max",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_exclusive_add : OclBuiltin<"work_group_scan_exclusive_add", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_exclusive_min : OclBuiltin<"work_group_scan_exclusive_min", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_exclusive_max : OclBuiltin<"work_group_scan_exclusive_max", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_inclusive_add : OclBuiltin<"work_group_scan_inclusive_add", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_inclusive_min : OclBuiltin<"work_group_scan_inclusive_min", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_inclusive_max : OclBuiltin<"work_group_scan_inclusive_max", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
  }
}


//
// Vectorizer table for scalar and vectorized versions of work-group functions
//
foreach func_name = [ "work_group_all", "work_group_any" ] in {
  def func_name#_cell1  : MapCell<!cast<OclBuiltin>(func_name), v1i32>;
  def func_name#_cell4  : MapCell<!cast<OclBuiltin>(func_name), v4i32>;
  def func_name#_cell8  : MapCell<!cast<OclBuiltin>(func_name), v8i32>;
  def func_name#_cell16 : MapCell<!cast<OclBuiltin>(func_name), v16i32>;
  def func_name#_vectorized : MapRow<[
    !cast<MapCell>(func_name#"_cell1"),  //scalar
    nullCell,                            //v2
    !cast<MapCell>(func_name#"_cell4"),  //v4
    !cast<MapCell>(func_name#"_cell8"),  //v8
    !cast<MapCell>(func_name#"_cell16"), //16
    nullCell],                           //3
    0,1>;          //not scalarizable, packetizable
}
foreach func_name = [ "work_group_broadcast1d", "work_group_broadcast2d", "work_group_broadcast3d",
                      "work_group_reduce_add", "work_group_reduce_min", "work_group_reduce_max",
                      "work_group_scan_exclusive_add", "work_group_scan_exclusive_min", "work_group_scan_exclusive_max",
                      "work_group_scan_inclusive_add", "work_group_scan_inclusive_min", "work_group_scan_inclusive_max"] in {
  foreach base_type = [ "i32", "u32", "i64", "u64", "f32", "f64" ] in {
    def func_name#_#base_type#_cell1  : MapCell<!cast<OclBuiltin>(func_name), !cast<OclType>("v1"#base_type)>;
    def func_name#_#base_type#_cell4  : MapCell<!cast<OclBuiltin>(func_name), !cast<OclType>("v4"#base_type)>;
    def func_name#_#base_type#_cell8  : MapCell<!cast<OclBuiltin>(func_name), !cast<OclType>("v8"#base_type)>;
    def func_name#_#base_type#_cell16 : MapCell<!cast<OclBuiltin>(func_name), !cast<OclType>("v16"#base_type)>;
    def func_name#_#base_type#_vectorized : MapRow<[
    !cast<MapCell>(func_name#_#base_type#"_cell1"),  //scalar
    nullCell,                                        //v2
    !cast<MapCell>(func_name#_#base_type#"_cell4"),  //v4
    !cast<MapCell>(func_name#_#base_type#"_cell8"),  //v8
    !cast<MapCell>(func_name#_#base_type#"_cell16"), //16
    nullCell],                                       //3
    0,1>;          //not scalarizable, packetizable
  }
}


// -------------------------------------------------------------------------------------------------------------------------------
// Per-WI versions of work-group functions. These functions have the following arguments:
//   1. WI (scalar or vectorized packet) argument of the same type as corresponding WG function
//   2. Pointer to accumulator. It is the most recent return value (for previous WI).
//      It is of the same type and width as WI argument.
//      Thus, - for uniform functions it contains the intermediate result ad hoc;
//            - for scan function it contains the result for previous WI;
//            - for broadcast function it contains broadcasted value or arbitrary value
//              (until the broadcasted value will be encountered)
//   3. For broadcast function ONLY: linear form of linear_id(x,y,z) argument of corresponding WG function, and linearID of the WI
//  Return value: intermediate result of calculation (for uniform and broadcast functions) or per-WI result (for scan functions)
// -------------------------------------------------------------------------------------------------------------------------------

// Forward definitions of utility add/all/any functions used by other helpers
let NeedForwardDecl = 1 in { /* work around the missing prototype in opencl.h */
  let Types = [ v1i32, v4i32, v8i32, v16i32,
                v1u32, v4u32, v8u32, v16u32,
                v1i64, v4i64, v8i64, v16i64,
                v1u64, v4u64, v8u64, v16u64,
                v1f32, v4f32, v8f32, v16f32,
                v1f64, v4f64, v8f64, v16f64],
      Attrs = [OVERLOADABLE] in {
    def work_group_add_helper_util  : OclBuiltin<"work_group_add_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
  }
  let Types = [ v1i32, v4i32, v8i32, v16i32],
      Attrs = [OVERLOADABLE] in {
    def work_group_all_helper_util  : OclBuiltin<"work_group_all_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
    def work_group_any_helper_util  : OclBuiltin<"work_group_any_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
  }
}

let Types = [ v1i32, v4i32, v8i32, v16i32 ],
    Attrs = [OVERLOADABLE] in {
  def work_group_all_helper : OclBuiltin<"work_group_all", (outs gentype:$ret), (ins gentype:$predicate, ppgentype:$accum), [], [], 1, 0>;
  def work_group_any_helper : OclBuiltin<"work_group_any", (outs gentype:$ret), (ins gentype:$predicate, ppgentype:$accum), [], [], 1, 0>;
}

let Types = [ v1i32, v4i32, v8i32, v16i32,
              v1u32, v4u32, v8u32, v16u32,
              v1i64, v4i64, v8i64, v16i64,
              v1u64, v4u64, v8u64, v16u64,
              v1f32, v4f32, v8f32, v16f32,
              v1f64, v4f64, v8f64, v16f64],
    Attrs = [OVERLOADABLE] in {
  def work_group_broadcast_helper : OclBuiltin<"work_group_broadcast", (outs gentype:$ret), (ins gentype:$src, size_t:$linear_local_id, size_t:$linear_id, ppgentype:$accum), [], [], 1, 0>;
}

foreach function_logic = ["reduce", "scan_exclusive", "scan_inclusive"] in {
  foreach function_op = ["add", "min", "max"] in {
    let Types = [ v1i32, v4i32, v8i32, v16i32,
                  v1u32, v4u32, v8u32, v16u32,
                  v1i64, v4i64, v8i64, v16i64,
                  v1u64, v4u64, v8u64, v16u64,
                  v1f32, v4f32, v8f32, v16f32,
                  v1f64, v4f64, v8f64, v16f64],
        Attrs = [OVERLOADABLE] in {
      def work_group_#function_logic#_#function_op#_helper :
                OclBuiltin<"work_group_"#function_logic#"_"#function_op, (outs gentype:$ret), (ins gentype:$src, ppgentype:$accum), [], [], 1, 0>;
    }
  }
}

// -----------------------------------------------------------------------------------------------------------------------
// Finalization functions for uniform non-broadcast vectorized WG functions. These functions have the following argument:
// 1. Value of the last (accumulated) return value (vectorized packet) of per-WI call
// Return value: uniform vector result of all WI calls - to be used by function call usages
// -----------------------------------------------------------------------------------------------------------------------

let Types = [ v4i32, v8i32, v16i32 ],
    Attrs = [OVERLOADABLE] in {
  def work_group_all_finalize_helper : OclBuiltin<"work_group_all_finalize", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_any_finalize_helper : OclBuiltin<"work_group_any_finalize", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
}

let Types = [ v4i32, v8i32, v16i32,
              v4u32, v8u32, v16u32,
              v4i64, v8i64, v16i64,
              v4u64, v8u64, v16u64,
              v4f32, v8f32, v16f32,
              v4f64, v8f64, v16f64],
    Attrs = [OVERLOADABLE] in {
  def work_group_reduce_add_finalize_helper : OclBuiltin<"work_group_reduce_add_finalize", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_reduce_min_finalize_helper : OclBuiltin<"work_group_reduce_min_finalize", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_reduce_max_finalize_helper : OclBuiltin<"work_group_reduce_max_finalize", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
}
