// No include guard
VPLAN_MAP(Value, VPValue)
VPLAN_MAP(Instruction, VPInstruction)
VPLAN_MAP(ICmpInst, VPCmpInst)
VPLAN_MAP(ConstantInt, VPConstantInt)
// TODO:
// *Operator and ConstantExpr types need fine grained counterparts in VPlan.
VPLAN_MAP(BinaryOperator, VPInstruction)
VPLAN_MAP(UnaryOperator, VPInstruction)
VPLAN_MAP(Operator, VPInstruction)
VPLAN_MAP(ConstantExpr, VPInstruction)
VPLAN_MAP(CallInst, VPCallInstruction)
VPLAN_MAP(BasicBlock, VPBasicBlock)
VPLAN_MAP(DominatorTree, VPDominatorTree)
VPLAN_MAP(PostDominatorTree, VPPostDominatorTree)
VPLAN_MAP(LoopInfo, VPLoopInfo)
VPLAN_MAP(Loop, VPLoop)
VPLAN_MAP(Function, VPlan)
