#include "PDGUtils.hpp" 
#include "llvm/IR/InstIterator.h"

using namespace llvm;
using namespace pdg;

InstructionWrapper *PDGUtils::getOrInsertInstWrapper(Value *v) {
  if (instWrapperAllocator.count(v)) {
    return instWrapperAllocator[v].get();
  }

  if (auto *inst = dyn_cast<llvm::Instruction>(v)) {
    instWrapperAllocator[v] = std::make_unique<InstructionWrapper>(inst, GraphNodeType::INST);
  }

  if (auto *func = dyn_cast<llvm::Function>(v)) {
    instWrapperAllocator[v] = std::make_unique<InstructionWrapper>(func, GraphNodeType::ENTRY);
  }

  return instWrapperAllocator[v].get();
}

FunctionWrapper *PDGUtils::getOrInsertFuncWrapper(llvm::Function *f) {
  if (funcWrapperAllocator.count(f)) {
    return funcWrapperAllocator[f].get();
  }

  funcWrapperAllocator[f] = std::make_unique<FunctionWrapper>(f);
  return funcWrapperAllocator[f].get();
}


void PDGUtils::constructInstMap(Function &F)
{
  for (inst_iterator I = inst_begin(F); I != inst_end(F); ++I)
  {
    if (G_instMap.find(&*I) == G_instMap.end())
    {
      InstructionWrapper *instW = getOrInsertInstWrapper(&*I);
      G_instMap[&*I] = instW;
      G_funcInstWMap[&F].insert(instW); 
    }
  }
}

void PDGUtils::constructFuncMap(Module &M)
{

  if (visitedModules.count(&M)) {
    return;
  }

  visitedModules.insert(&M);

  for (Module::iterator FI = M.begin(); FI != M.end(); ++FI)
  {
    if (FI->isDeclaration())
      continue;
    constructInstMap(*FI);
    if (G_funcMap.find(&*FI) == G_funcMap.end())
    {
      FunctionWrapper *funcW = getOrInsertFuncWrapper(&*FI);
      G_funcMap[&*FI] = funcW;
    }
  }
}

void PDGUtils::collectGlobalInsts(Module &M)
{
  for (Module::global_iterator globalIt = M.global_begin(); globalIt != M.global_end(); ++globalIt)
  {
    InstructionWrapper *globalW = new InstructionWrapper(dyn_cast<Value>(&(*globalIt)), GraphNodeType::GLOBAL_VALUE);
    G_globalInstsSet.insert(globalW);
  }
}

void PDGUtils::categorizeInstInFunc(Function &F)
{
  // sort store/load/return/CallInst in function
  for (inst_iterator I = inst_begin(F), IE = inst_end(F); I != IE; ++I)
  {
    Instruction *inst = dyn_cast<Instruction>(&*I);
    if (isa<StoreInst>(inst))
      G_funcMap[&F]->addStoreInst(inst);

    if (isa<LoadInst>(inst))
      G_funcMap[&F]->addLoadInst(inst);

    if (isa<ReturnInst>(inst))
      G_funcMap[&F]->addReturnInst(inst);

    if (isa<CallInst>(inst))
      G_funcMap[&F]->addCallInst(inst);

    if (isa<CastInst>(inst))
      G_funcMap[&F]->addCastInst(inst);
  }
}
