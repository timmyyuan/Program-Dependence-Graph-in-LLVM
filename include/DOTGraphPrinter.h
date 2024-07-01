#ifndef _PDG_LLVM_DOTGRAPH_PRINTER_H
#define _PDG_LLVM_DOTGRAPH_PRINTER_H

#include "llvm/Analysis/CFGPrinter.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/LockFileManager.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/Support/Path.h"
#include <unordered_set>

namespace llvm {

class LockFileManager;

template <typename GraphT>
void printGraphForFuncWithFilename(Function &F, GraphT Graph, StringRef Name,
                           bool IsSimple) {
  auto Basename = sys::path::filename(F.getParent()->getName());
  if (Basename.starts_with(".")) {
    Basename = Basename.split(".").second;
  }

  while (sys::path::has_extension(Basename)) {
    Basename = sys::path::stem(Basename);
  }

  std::string Filename = Name.str() + "." + Basename.str() + "." + F.getName().str();
  shortenFileName(Filename);
  Filename = Filename + ".dot";
  std::error_code EC;

  if (sys::fs::exists(Filename)) {
    errs() << "Skip repeat '" << Filename << "'...\n";
    return;
  }

  errs() << "Writing '" << Filename << "'...";

  raw_fd_ostream File(Filename, EC, sys::fs::OF_TextWithCRLF);
  std::string GraphName = DOTGraphTraits<GraphT>::getGraphName(Graph);

  if (!EC)
    WriteGraph(File, Graph, IsSimple,
               GraphName + " for '" + F.getName() + "' function");
  else
    errs() << "  error opening file for writing!";
  errs() << "\n";
}

template <typename AnalysisT, bool IsSimple, typename GraphT = AnalysisT *,
          typename AnalysisGraphTraitsT =
              LegacyDefaultAnalysisGraphTraits<AnalysisT, GraphT>>
class CustomGraphPrinterWrapperPass : public FunctionPass {
public:
  CustomGraphPrinterWrapperPass(StringRef GraphName, char &ID)
      : FunctionPass(ID), Name(GraphName) {}

  /// Return true if this function should be processed.
  ///
  /// An implementation of this class my override this function to indicate that
  /// only certain functions should be printed.
  ///
  /// @param Analysis The current analysis result for this function.
  virtual bool processFunction(Function &F, AnalysisT &Analysis) {
    return true;
  }

  bool runOnFunction(Function &F) override {
    auto &Analysis = getAnalysis<AnalysisT>();

    if (!processFunction(F, Analysis))
      return false;

    GraphT Graph = AnalysisGraphTraitsT::getGraph(&Analysis);
    printGraphForFuncWithFilename(F, Graph, Name, IsSimple);

    return false;
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesAll();
    AU.addRequired<AnalysisT>();
  }

private:
  std::string Name;
};

}

#endif