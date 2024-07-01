#include "InstructionWrapper.hpp"

using namespace pdg;
using namespace llvm;

std::string InstructionWrapper::getOpTypeOrNull() const {
    if (!this->Inst) {
        return "unnamed";
    }

    std::string result = Inst->getOpcodeName();
    if (auto *call = llvm::dyn_cast<llvm::CallBase>(Inst)) {
        if (auto *fn = call->getCalledFunction()) {
            if (fn->isIntrinsic() && fn->hasName()) {
                result = fn->getName().str();
            }
        }
    }

    return result;
}

std::string InstructionWrapper::getBasicBlockIndex() const {
    if (!Inst) {
        return std::to_string(-1);
    }

    llvm::BasicBlock *Parent = Inst->getParent();
    llvm::Function *Fn = Parent->getParent();
    int Index = 0;
    for (auto &BB: *Fn) {
        if (&BB == Parent) {
            return std::to_string(Index);
        }
        Index ++;
    }

    return std::to_string(-1);
}

std::string InstructionWrapper::getDotAttrs() const {
    std::string attrs = "";
    
    attrs += "op=\"" + getOpTypeOrNull() +"\", ";
    attrs += "bid=\"" + getBasicBlockIndex() + "\"";

    switch (getGraphNodeType())
    {
        case GraphNodeType::ENTRY:
        case GraphNodeType::GLOBAL_VALUE:
        case GraphNodeType::STRUCT_FIELD:
            break;
        case GraphNodeType::FORMAL_IN:
        case GraphNodeType::ACTUAL_IN:
        case GraphNodeType::FORMAL_OUT:
        case GraphNodeType::ACTUAL_OUT:
        case GraphNodeType::PARAMETER_FIELD:
            attrs += ", color=\"blue\"";
            break;
        case GraphNodeType::POINTER_RW:
            attrs += ", color=\"red\"";
            break;
        default:
            break;
    }

    return attrs;
}
