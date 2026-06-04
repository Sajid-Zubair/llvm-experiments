#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/raw_ostream.h"

#include <map>

using namespace llvm;

namespace {

struct CFCSS : public FunctionPass {
  static char ID;

  CFCSS() : FunctionPass(ID) {}

  bool runOnFunction(Function &F) override {

    Module *M = F.getParent();
    LLVMContext &Ctx = M->getContext();

    GlobalVariable *RuntimeSig =
        M->getNamedGlobal("RuntimeSig");

    if (!RuntimeSig) {
      RuntimeSig = new GlobalVariable(
          *M,
          Type::getInt32Ty(Ctx),
          false,
          GlobalValue::ExternalLinkage,
          ConstantInt::get(Type::getInt32Ty(Ctx), 0),
          "RuntimeSig");
    }

    std::map<BasicBlock *, int> SignatureMap;

    int Sig = 1;

    for (BasicBlock &BB : F) {
      SignatureMap[&BB] = Sig++;
    }

    for (BasicBlock &BB : F) {

      int ExpectedSig = SignatureMap[&BB];

      Instruction *First =
          &*BB.getFirstInsertionPt();

      IRBuilder<> EntryBuilder(First);

      Value *Loaded =
          EntryBuilder.CreateLoad(
              Type::getInt32Ty(Ctx),
              RuntimeSig);

      Value *Expected =
          ConstantInt::get(
              Type::getInt32Ty(Ctx),
              ExpectedSig);

      Value *Cmp =
          EntryBuilder.CreateICmpEQ(
              Loaded,
              Expected);

      errs() << "Checking block "
             << BB.getName()
             << " Signature="
             << ExpectedSig
             << "\n";

      Instruction *TI = BB.getTerminator();

      for (unsigned i = 0; i < TI->getNumSuccessors(); i++) {

    BasicBlock *Succ = TI->getSuccessor(i);

    int CurrentSig = SignatureMap[&BB];
    int SuccSig    = SignatureMap[Succ];

    // Delta = Current XOR Successor
    int Delta = CurrentSig ^ SuccSig;

    IRBuilder<> ExitBuilder(TI);

    // Load current runtime signature
    Value *CurrentRuntimeSig =
        ExitBuilder.CreateLoad(
            Type::getInt32Ty(Ctx),
            RuntimeSig);

    // XOR with delta
    Value *NewRuntimeSig =
        ExitBuilder.CreateXor(
            CurrentRuntimeSig,
            ConstantInt::get(
                Type::getInt32Ty(Ctx),
                Delta));

    // Store updated signature
    ExitBuilder.CreateStore(
        NewRuntimeSig,
        RuntimeSig);
}
    }

    return true;
  }
};

}

char CFCSS::ID = 0;

static RegisterPass<CFCSS>
X("cfcss",
  "Control Flow Checking by Software Signatures");