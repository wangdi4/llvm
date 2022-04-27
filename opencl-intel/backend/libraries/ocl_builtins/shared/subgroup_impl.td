defvar subgroup_fake_code = [{
    return ($ReturnType)$Arg0VarName;
  }];

def : OclBuiltinImpl<sub_group_ballot, sub_group_ballot.Types, false, subgroup_fake_code>;
foreach vf = [4, 8, 16, 32, 64] in {
  defvar builtin = !cast<OclBuiltin>("sub_group_ballot_vf" # vf);
  // Implementation is written in LLVM IR (shared/ll_subgroup_impl.td).
  def : OclBuiltinImpl<builtin, builtin.Types, /*IsDeclOnly*/true, [{}]>;
} // foreach vf
