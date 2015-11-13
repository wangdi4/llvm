#include "SubGroupAdaptation.h"
#include "OCLPassSupport.h"
#include "InitializePasses.h"
#include "CompilationUtils.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DataLayout.h"
#include <assert.h>
#include "NameMangleAPI.h"
#include "Mangler.h"

#include <utility>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

extern "C" {
	/// @brief Creates new SubGroupAdaptationPass function pass
	/// @returns new SubGroupAdaptationPass function pass
	ModulePass *createSubGroupAdaptationPass() {
		return new intel::SubGroupAdaptation();
	}
}

namespace intel {

	char SubGroupAdaptation::ID = 0;

	OCL_INITIALIZE_PASS(SubGroupAdaptation, "sub-group-adaptation", "Replace sub-group built-ins with appropriate IR sequence", false, false);

	bool SubGroupAdaptation::runOnModule(Module &M) {

		m_pModule = &M;
		m_pLLVMContext = &M.getContext();
		unsigned pointerSizeInBits = M.getDataLayout()->getPointerSizeInBits(0);
		assert((32 == pointerSizeInBits || 64 == pointerSizeInBits) &&
			"Unsupported pointer size");
		m_pSizeT = IntegerType::get(*m_pLLVMContext, pointerSizeInBits);

		for (Module::iterator mi = M.begin(), me = M.end(); mi != me;) {
			Function * pFunc = mi++;

			// OCL built-ins must not be defined in the module at the moment the pass is running
			if (!pFunc || !pFunc->isDeclaration()) continue;
			llvm::StringRef func_name = pFunc->getName();
			if (CompilationUtils::isSubGroupBarrier(func_name)) {
				replaceFunction(pFunc, CompilationUtils::WG_BARRIER_FUNC_NAME);
			}
			else if (CompilationUtils::isGetSubGroupLocalID(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_GET_LINEAR_LID);
			else if (CompilationUtils::isGetSubGroupSize(func_name) || CompilationUtils::isGetMaxSubGroupSize(func_name)) {

				BasicBlock *entry = BasicBlock::Create(*m_pLLVMContext, "entry", pFunc);

				// Replace get_sub_group_size() with the following sequence.
				// get_enqueued_local_size(0) * get_enqueued_local_size(1) * get_enqueued_local_size(2) +
				CallInst *enqueued_local_size_0 = getWICall(entry, "elsz0", CompilationUtils::mangledGetEnqueuedLocalSize(), 0);
				CallInst *enqueued_local_size_1 = getWICall(entry, "elsz1", CompilationUtils::mangledGetEnqueuedLocalSize(), 1);
				CallInst *enqueued_local_size_2 = getWICall(entry, "elsz2", CompilationUtils::mangledGetEnqueuedLocalSize(), 2);

				Instruction *pRetVal = BinaryOperator::CreateMul(enqueued_local_size_0, enqueued_local_size_1, "op0", entry);
				pRetVal = BinaryOperator::CreateMul(pRetVal, enqueued_local_size_2, "res", entry);

				if (pRetVal->getType() != pFunc->getReturnType()) {
					pRetVal = CastInst::CreateIntegerCast(pRetVal, pFunc->getReturnType(), false, "cast", entry);
				}
				ReturnInst::Create(*m_pLLVMContext, pRetVal, entry);
			}
			else if (CompilationUtils::isSubGroupAll(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_ALL);
			else if (CompilationUtils::isSubGroupAny(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_ANY);
			else if (CompilationUtils::isGetNumSubGroups(func_name))
				replaceWithConst(pFunc, 1);
			else if (CompilationUtils::isGetEnqueuedNumSubGroups(func_name))
				replaceWithConst(pFunc, 0);
			else if (CompilationUtils::isGetSubGroupId(func_name))
				replaceWithConst(pFunc, 0);
			else if (CompilationUtils::isSubGroupBroadCast(func_name))
				replaceSubGroupBroadcast(pFunc);
			else if (CompilationUtils::isSubGroupReduceAdd(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_REDUCE_ADD);
			else if (CompilationUtils::isSubGroupReduceMax(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_REDUCE_MAX);
			else if (CompilationUtils::isSubGroupReduceMin(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_REDUCE_MIN);
			else if (CompilationUtils::isSubGroupScanExclusiveAdd(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_SCAN_EXCLUSIVE_ADD);
			else if (CompilationUtils::isSubGroupScanExclusiveMax(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_SCAN_EXCLUSIVE_MAX);
			else if (CompilationUtils::isSubGroupScanExclusiveMin(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_SCAN_EXCLUSIVE_MIN);
			else if (CompilationUtils::isSubGroupScanExclusiveAdd(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_SCAN_INCLUSIVE_ADD);
			else if (CompilationUtils::isSubGroupScanExclusiveMax(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_SCAN_INCLUSIVE_MAX);
			else if (CompilationUtils::isSubGroupScanExclusiveMin(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_SCAN_INCLUSIVE_MIN);
			else if (CompilationUtils::isSubGroupReserveReadPipe(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_RESERVE_READ_PIPE);
			else if (CompilationUtils::isSubGroupCommitReadPipe(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_COMMIT_READ_PIPE);
			else if (CompilationUtils::isSubGroupReserveWritePipe(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_RESERVE_WRITE_PIPE);
			else if (CompilationUtils::isSubGroupCommitWritePipe(func_name))
				replaceFunction(pFunc, CompilationUtils::NAME_WORK_GROUP_COMMIT_WRITE_PIPE);
		}
		return true;
	}

	void SubGroupAdaptation::replaceFunction(Function *oldFunc, std::string newFuncName) {
		std::string m_name  = oldFunc->getName();
		reflection::FunctionDescriptor fd = demangle(m_name.c_str());
		fd.name = newFuncName;
		std::string newName = mangle(fd);
		Function * newF = cast<Function>(m_pModule->getOrInsertFunction(
			newName, oldFunc->getFunctionType(), oldFunc->getAttributes()));
		newF->setCallingConv(oldFunc->getCallingConv());
		oldFunc->replaceAllUsesWith(newF);	
		oldFunc->eraseFromParent();
	}

	void SubGroupAdaptation::replaceWithConst(Function *oldFunc, unsigned constInt) {
		std::vector<Instruction*> callSgFunc;

		for (Function::user_iterator ui = oldFunc->user_begin(),
			ue = oldFunc->user_end();
			ui != ue; ++ui) {
			CallInst *pCallInst = cast<CallInst>(*ui);
			// Found a call instruction to sub-group built-in, collect it.
			if (pCallInst != nullptr)
				callSgFunc.push_back(pCallInst);
		}

		for (unsigned idx = 0; idx < callSgFunc.size(); idx++) {
			CallInst *CI = cast<CallInst>(callSgFunc[idx]);
			Value* pConstInt = ConstantInt::get(CI->getType(), constInt);
			CI->replaceAllUsesWith(pConstInt);
			CI->eraseFromParent();
		}
	}

	void SubGroupAdaptation::replaceSubGroupBroadcast(Function* pFunc){
		BasicBlock *entry = BasicBlock::Create(*m_pLLVMContext, "entry", pFunc);
		SmallVector<Value*, 4> params;

		CallInst *local_size_0 = getWICall(entry, "lsz0", CompilationUtils::mangledGetLocalSize(), 0);
		CallInst *local_size_1 = getWICall(entry, "lsz1", CompilationUtils::mangledGetLocalSize(), 1);

		CallInst *local_id_0 = getWICall(entry, "lid0", CompilationUtils::mangledGetLID(), 0);
		CallInst *local_id_1 = getWICall(entry, "lid1", CompilationUtils::mangledGetLID(), 1);
		CallInst *local_id_2 = getWICall(entry, "lid2", CompilationUtils::mangledGetLID(), 2);

		Function::ArgumentListType::iterator firstArg = pFunc->getArgumentList().begin();
		Function::ArgumentListType::iterator secondArg = pFunc->getArgumentList().begin()++;
		params.push_back(firstArg);

		// For 1-dim workgroup - return get_local_id(0)
		// <3-dimensional Linear-ID> % get local_size(0)
		Value *linid = secondArg;
		if (linid->getType() != local_size_0->getType())
			linid = CastInst::CreateIntegerCast(linid, local_size_0->getType(), false, "linid", entry);
		Instruction *lid0 = BinaryOperator::CreateURem(linid, local_size_0, "lid0.res", entry);
		params.push_back(lid0);

		// Calculate local_id(1):
		//  ((<3-dimensional Linear-ID> / get_local_size(0)) % get_local_size(1)
		BinaryOperator *rd1 = BinaryOperator::CreateUDiv(linid, local_size_0, "lid1.op", entry);
		BinaryOperator *lid1 = BinaryOperator::CreateURem(rd1, local_size_1, "lid1.res", entry);
		params.push_back(lid1);

		// Calculate local_id(2):
		// ((<3-dimensional Linear-ID> / get_local_size(0)) - get_local_id(1)) / get_local_size(1)
		BinaryOperator *rd2 = BinaryOperator::CreateSub(rd1, lid1, "lid2.op", entry);
		BinaryOperator *lid2 = BinaryOperator::CreateUDiv(rd2, local_size_1, "lid2.res", entry);
		params.push_back(lid2);

		std::string strFuncName = pFunc->getName();
		reflection::FunctionDescriptor fd = demangle(strFuncName.c_str());

		// replase built-in name
		fd.name = CompilationUtils::NAME_WORK_GROUP_BROADCAST.c_str();
		fd.parameters.pop_back();
		for (unsigned int i = 0; i < 3; ++i) {
			reflection::RefParamType F;
			if (m_pSizeT->getPrimitiveSizeInBits() == 64)
				F = new reflection::PrimitiveType(reflection::PRIMITIVE_ULONG);
			else
				F = new reflection::PrimitiveType(reflection::PRIMITIVE_UINT);
			fd.parameters.push_back(F);
		}
		std::string newFuncName = mangle(fd);

		std::vector<Type*>  types;
		types.push_back(firstArg->getType());
		for (unsigned int i = 0; i < 3; ++i)
			types.push_back(lid2->getType());

		FunctionType *pNewFuncType = FunctionType::get(pFunc->getReturnType(), types, false);
		Function * newF = cast<Function>(m_pModule->getOrInsertFunction(
			newFuncName, pNewFuncType, pFunc->getAttributes()));
		CallInst *pNewCall = CallInst::Create(newF, ArrayRef<Value*>(params), "CallWGBroadCast", entry);

		newF->setCallingConv(pFunc->getCallingConv());

		if (firstArg->getType() != pFunc->getReturnType()) {
			Instruction *pCast = CastInst::CreateIntegerCast(firstArg, pFunc->getReturnType(), false, "cast", entry);
			ReturnInst::Create(*m_pLLVMContext, pCast, entry);
		}
		else
			ReturnInst::Create(*m_pLLVMContext, firstArg, entry);
	}

	CallInst *SubGroupAdaptation::getWICall(BasicBlock *pAtEnd, char const* twine, std::string funcName, unsigned dimIdx) {
		Type *pInt32Type = Type::getInt32Ty(*m_pLLVMContext);;
		FunctionType *pFuncType = FunctionType::get(m_pSizeT, pInt32Type, false);
		Function *func = cast<Function>(m_pModule->getOrInsertFunction(funcName, pFuncType));
		CallInst *pCall = CallInst::Create(func, ArrayRef<Value*>(ConstantInt::get(pInt32Type, dimIdx)), twine, pAtEnd);
		assert(pCall && "Couldn't create CALL instruction!");
		return pCall;
	}
}