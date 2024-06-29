#ifndef DATADEPENDENCYGRAPH_H_
#define DATADEPENDENCYGRAPH_H_
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Analysis/BasicAliasAnalysis.h"

#include "DependencyGraph.hpp"
#include "CallWrapper.hpp"
#include "FunctionWrapper.hpp"

namespace pdg
{
class DataDependencyGraph : public llvm::FunctionPass
{
public:
  static char ID;
  DataDependencyGraph() : FunctionPass(ID)
  {
    DDG = new DependencyGraph<InstructionWrapper>();
  }

  ~DataDependencyGraph()
  {
    delete DDG;
  }

  void initializeMemoryDependencyPasses();
  void constructFuncMapAndCreateFunctionEntry();
  void collectDefUseDependency(llvm::Instruction *inst);
  void collectCallInstDependency(llvm::Instruction *inst);
  void collectWriteToDependency(llvm::Instruction *loadInst);
  std::vector<llvm::Instruction *> getRAWDepList(llvm::Instruction *loadInst);
  void collectRAWDependency(llvm::Instruction *inst);
  void collectReadFromDependency(llvm::Instruction *inst);
  void collectNonLocalDependency(llvm::Instruction *inst);
  void collectDataDependencyInFunc();
  void collectAliasDependencies();
  DependencyNode<InstructionWrapper> *getNodeByData(llvm::Instruction *inst);
  typename DependencyNode<InstructionWrapper>::DependencyLinkList getNodeDepList(llvm::Instruction *inst);
  virtual bool runOnFunction(llvm::Function &Func);
  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;
  virtual llvm::StringRef getPassName() const { return "Data Dependency Graph"; }
  DependencyGraph<InstructionWrapper> *_getDDG() {return DDG;}
  
  bool isAlias(llvm::LoadInst *lhs, llvm::LoadInst *rhs) {
    return lhs->getPointerOperand() == rhs->getPointerOperand();
  }
  bool isAlias(llvm::LoadInst *lhs, llvm::StoreInst *rhs) {
    return lhs->getPointerOperand() == rhs->getPointerOperand();
  }
  bool isAlias(llvm::StoreInst *lhs, llvm::StoreInst *rhs) {
    return lhs->getPointerOperand() == rhs->getPointerOperand();
  }


private:
  DependencyGraph<InstructionWrapper> *DDG;
  llvm::Function *Func;
  llvm::MemoryDependenceResults *MD;

  llvm::AAResults *AA;
};
} // namespace pdg

#endif