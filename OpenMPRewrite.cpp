//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//
#include <sstream>
#include <fstream>
#include <iostream>
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Sema/Sema.h"
#include "llvm/Support/raw_ostream.h"
#include "clang/Rewrite/Core/Rewriter.h"

using namespace clang;

namespace {

  // Runtime header
std::string RuntimeFuncDecl = 
  "#include <stdio.h>\n"
  "#include <stdint.h>\n"
  "int __tgt_register_mem(void* mem, uint64_t size) {"
  "printf(\"mem :%p, size: %lld\\n\",mem,size);"
  "return 1; }\n";

class OpenMPRewriteASTVisitor : public RecursiveASTVisitor<OpenMPRewriteASTVisitor> {
  Rewriter &OpenMPRewriter;
  CompilerInstance &CI;
public:
  OpenMPRewriteASTVisitor(Rewriter &R, CompilerInstance &CI) : OpenMPRewriter(R), CI(CI) {}

  bool VisitCastExpr(CastExpr *ce) {
    // If same address and size on stack?
    QualType t2 = ce->getSubExpr()->getType().getDesugaredType(CI.getASTContext());
    if (isa<ArrayType>(t2)) {
      //uint64_t size = cat->getSize().getZExtValue ();
      Expr *e = ce->getSubExpr();
      // TODO check side effect
      SourceLocation loc = e->getBeginLoc();
      const SourceManager &sm = CI.getSourceManager();
      const LangOptions &lopt = CI.getLangOpts();
      SmallVector<char, 25> buffer;
      StringRef VarName = Lexer::getSpelling(sm.getSpellingLoc(loc), buffer,
                            sm, lopt, nullptr);
      size_t size = VarName.size();
      SourceLocation endloc = loc.getLocWithOffset(size);

      std::stringstream InsertTxt;
      InsertTxt << "(__tgt_register_mem((void*)" << VarName.str() << ", sizeof(" << VarName.str() << "))?";
      std::string EndString = ":NULL)";

      OpenMPRewriter.InsertText(loc, InsertTxt.str(), true, true);
      OpenMPRewriter.InsertText(endloc, EndString, true, true);
    }
    return true;
  }
};


class OpenMPRewriteConsumer : public ASTConsumer {
  CompilerInstance &CI;
  OpenMPRewriteASTVisitor Visitor;
  Rewriter &OpenMPRewriter;

public:
  OpenMPRewriteConsumer(CompilerInstance &Instance, Rewriter &R)
      : CI(Instance), Visitor(R, Instance), OpenMPRewriter(R) {}

  void HandleTranslationUnit(ASTContext& context) override {
    Visitor.TraverseDecl(context.getTranslationUnitDecl());
    FileID fid = CI.getSourceManager().getMainFileID();
    const RewriteBuffer *buf = OpenMPRewriter.getRewriteBufferFor(fid);
    if (buf) {
			SourceManager &sm = CI.getSourceManager();
			SourceLocation loc = sm.getLocForStartOfFile(fid);
			OpenMPRewriter.InsertText(loc, RuntimeFuncDecl, true, true);

			std::fstream OutFile;
      std::string name = sm.getFilename(loc).str()+".c";
			OutFile.open(name, std::ios::out|std::ios::trunc);
			OutFile.write(std::string(buf->begin(), buf->end()).c_str(), buf->size());
			OutFile.close();
    }
  }
};

class OpenMPRewriteAction : public PluginASTAction {
  Rewriter OpenMPRewriter;
  CompilerInstance *CI;
public:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &CI,
                                                 llvm::StringRef) override {
    this->CI = &CI;
    SourceManager &SourceMgr = CI.getSourceManager();
    OpenMPRewriter.setSourceMgr(SourceMgr, CI.getLangOpts());
    return llvm::make_unique<OpenMPRewriteConsumer>(CI, OpenMPRewriter);
  }
  bool ParseArgs(const CompilerInstance &CI,
                 const std::vector<std::string> &args) override {
    for (unsigned i = 0, e = args.size(); i != e; ++i) {
    }
    if (!args.empty() && args[0] == "help") {
      PrintHelp(llvm::errs());
    }
    return true;
  }
  void PrintHelp(llvm::raw_ostream& ros) {
    ros << "usage: \nclang -cc1 -load <PATH-TO-LLVM-BUILD>/lib/OpenMPRewrite.so -plugin omp-rewtr <INPUT-FILE>\n";
  }
};
}

static FrontendPluginRegistry::Add<OpenMPRewriteAction>
 X("omp-rewtr", "OpenMP Rewriter");
