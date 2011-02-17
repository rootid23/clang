//===--- ScopeInfo.h - Information about a semantic context -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines FunctionScopeInfo and BlockScopeInfo.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_SEMA_SCOPE_INFO_H
#define LLVM_CLANG_SEMA_SCOPE_INFO_H

#include "clang/AST/Type.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SetVector.h"

namespace clang {

class BlockDecl;
class IdentifierInfo;
class LabelDecl;
class ReturnStmt;
class Scope;
class SwitchStmt;

namespace sema {

/// \brief Retains information about a function, method, or block that is
/// currently being parsed.
class FunctionScopeInfo {
public:

  /// \brief Whether this scope information structure defined information for
  /// a block.
  bool IsBlockInfo;

  /// \brief Whether this function contains a VLA, @try, try, C++
  /// initializer, or anything else that can't be jumped past.
  bool HasBranchProtectedScope;

  /// \brief Whether this function contains any switches or direct gotos.
  bool HasBranchIntoScope;

  /// \brief Whether this function contains any indirect gotos.
  bool HasIndirectGoto;

  /// \brief Used to determine if errors occurred in this function or block.
  DiagnosticErrorTrap ErrorTrap;

  /// LabelMap - This is a mapping from label identifiers to the LabelDecl for
  /// it.  Forward referenced labels have a LabelDecl created for them with a
  /// null statement.
  llvm::DenseMap<IdentifierInfo*, LabelDecl*> LabelMap;

  /// SwitchStack - This is the current set of active switch statements in the
  /// block.
  llvm::SmallVector<SwitchStmt*, 8> SwitchStack;

  /// \brief The list of return statements that occur within the function or
  /// block, if there is any chance of applying the named return value
  /// optimization.
  llvm::SmallVector<ReturnStmt *, 4> Returns;

  void setHasBranchIntoScope() {
    HasBranchIntoScope = true;
  }

  void setHasBranchProtectedScope() {
    HasBranchProtectedScope = true;
  }

  void setHasIndirectGoto() {
    HasIndirectGoto = true;
  }

  bool NeedsScopeChecking() const {
    return HasIndirectGoto ||
          (HasBranchProtectedScope && HasBranchIntoScope);
  }
  
  FunctionScopeInfo(Diagnostic &Diag)
    : IsBlockInfo(false),
      HasBranchProtectedScope(false),
      HasBranchIntoScope(false),
      HasIndirectGoto(false),
      ErrorTrap(Diag) { }

  virtual ~FunctionScopeInfo();

  /// checkLabelUse - This checks to see if any labels are used without being
  /// defined, emiting errors and returning true if any are found.  This also
  /// warns about unused labels.
  bool checkLabelUse(Stmt *Body, Sema &S);
  
  /// \brief Clear out the information in this function scope, making it
  /// suitable for reuse.
  void Clear();

  static bool classof(const FunctionScopeInfo *FSI) { return true; }
};

/// \brief Retains information about a block that is currently being parsed.
class BlockScopeInfo : public FunctionScopeInfo {
public:
  BlockDecl *TheDecl;
  
  /// TheScope - This is the scope for the block itself, which contains
  /// arguments etc.
  Scope *TheScope;

  /// ReturnType - The return type of the block, or null if the block
  /// signature didn't provide an explicit return type.
  QualType ReturnType;

  /// BlockType - The function type of the block, if one was given.
  /// Its return type may be BuiltinType::Dependent.
  QualType FunctionType;

  /// CaptureMap - A map of captured variables to (index+1) into Captures.
  llvm::DenseMap<VarDecl*, unsigned> CaptureMap;

  /// Captures - The captured variables.
  llvm::SmallVector<BlockDecl::Capture, 4> Captures;

  /// CapturesCXXThis - Whether this block captures 'this'.
  bool CapturesCXXThis;

  BlockScopeInfo(Diagnostic &Diag, Scope *BlockScope, BlockDecl *Block)
    : FunctionScopeInfo(Diag), TheDecl(Block), TheScope(BlockScope),
      CapturesCXXThis(false)
  {
    IsBlockInfo = true;
  }

  virtual ~BlockScopeInfo();

  static bool classof(const FunctionScopeInfo *FSI) { return FSI->IsBlockInfo; }
  static bool classof(const BlockScopeInfo *BSI) { return true; }
};

}
}

#endif
