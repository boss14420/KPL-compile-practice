/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdlib.h>

#include "reader.h"
#include "scanner.h"
#include "parser.h"
#include "error.h"

Token *currentToken;
Token *lookAhead;

void scan(void) {
  Token* tmp = currentToken;
  currentToken = lookAhead;
  lookAhead = getValidToken();
  free(tmp);
}

void eat(TokenType tokenType) {
  if (lookAhead->tokenType == tokenType) {
    printToken(lookAhead);
    scan();
  } else missingToken(tokenType, lookAhead->lineNo, lookAhead->colNo);
}

void compileProgram(void) {
  assert("Parsing a Program ....");
  eat(KW_PROGRAM);
  eat(TK_IDENT);
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_PERIOD);
  assert("Program parsed!");
}

void compileBlock(void) {
  assert("Parsing a Block ....");
  if (lookAhead->tokenType == KW_CONST) {
    eat(KW_CONST);
    compileConstDecl();
    compileConstDecls();
    compileBlock2();
  } 
  else compileBlock2();
  assert("Block parsed!");
}

void compileBlock2(void) {
  if (lookAhead->tokenType == KW_TYPE) {
    eat(KW_TYPE);
    compileTypeDecl();
    compileTypeDecls();
    compileBlock3();
  } 
  else compileBlock3();
}

void compileBlock3(void) {
  if (lookAhead->tokenType == KW_VAR) {
    eat(KW_VAR);
    compileVarDecl();
    compileVarDecls();
    compileBlock4();
  } 
  else compileBlock4();
}

void compileBlock4(void) {
  compileSubDecls();
  compileBlock5();
}

void compileBlock5(void) {
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
}

void compileConstDecls(void) {
    while (lookAhead->tokenType == TK_IDENT) {
        compileConstDecl();
    }
}

void compileConstDecl(void) {
    eat(TK_IDENT);
    eat(SB_EQ);
    compileConstant();
    eat(SB_SEMICOLON);
}

void compileTypeDecls(void) {
    while (lookAhead->tokenType == TK_IDENT) {
        compileTypeDecl();
    }
}

void compileTypeDecl(void) {
    eat(TK_IDENT);
    eat(SB_EQ);
    compileType();
    eat(SB_SEMICOLON);
}

void compileVarDecls(void) {
    while (lookAhead->tokenType == TK_IDENT) {
        compileVarDecl();
    }
}

void compileVarDecl(void) {
    eat(TK_IDENT);
    eat(SB_COLON);
    compileType();
    eat(SB_SEMICOLON);
}

void compileSubDecls(void) {
  assert("Parsing subtoutines ....");
  while (1) {
      if (lookAhead->tokenType == KW_FUNCTION) {
          compileFuncDecl();
      } else if (lookAhead->tokenType == KW_PROCEDURE) {
          compileProcDecl();
      } else
          break;
  }
  assert("Subtoutines parsed ....");
}

void compileFuncDecl(void) {
  assert("Parsing a function ....");
  eat(KW_FUNCTION);
  eat(TK_IDENT);
  compileParams();
  eat(SB_COLON);
  compileBasicType();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  assert("Function parsed ....");
}

void compileProcDecl(void) {
  assert("Parsing a procedure ....");
  eat(KW_PROCEDURE);
  eat(TK_IDENT);
  compileParams();
  eat(SB_SEMICOLON);
  compileBlock();
  eat(SB_SEMICOLON);
  assert("Procedure parsed ....");
}

void compileUnsignedConstant(void) {
    TokenType type = lookAhead->tokenType;
    if (type != TK_NUMBER && type != TK_IDENT && type != TK_CHAR) {
        error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    }
    printToken(lookAhead);
    scan();
}

void compileConstant(void) {
    switch (lookAhead->tokenType) {
        case SB_PLUS:
        case SB_MINUS:
            printToken(lookAhead);
            scan();
            compileConstant2();
            break;
        case TK_CHAR:
            printToken(lookAhead);
            scan();
            break;
        default:
            compileConstant2();
            break;
    }
}

void compileConstant2(void) {
    TokenType type = lookAhead->tokenType;
    if (type != TK_NUMBER && type != TK_IDENT) {
        error(ERR_INVALIDCONSTANT, lookAhead->lineNo, lookAhead->colNo);
    }
    printToken(lookAhead);
    scan();
}

void compileType(void) {
    while(1) {
        switch(lookAhead->tokenType) {
            case KW_INTEGER:
            case KW_CHAR:
            case TK_IDENT:
                printToken(lookAhead);
                scan();
                return;
            case KW_ARRAY:
                printToken(lookAhead);
                scan();
                eat(SB_LSEL);
                eat(TK_NUMBER);
                eat(SB_RSEL);
                eat(KW_OF);
                break;
            default:
                error(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
        }
    }
}

void compileBasicType(void) {
    TokenType type = lookAhead->tokenType;
    if (type != KW_INTEGER && type != KW_CHAR)
        error(ERR_INVALIDTYPE, lookAhead->lineNo, lookAhead->colNo);
    printToken(lookAhead);
    scan();
}

void compileParams(void) {
    if (lookAhead->tokenType == SB_LPAR) {
        printToken(lookAhead);
        scan();
        compileParam();
        compileParams2();
        eat(SB_RPAR);
    }
}

void compileParams2(void) {
    while (lookAhead->tokenType == SB_SEMICOLON) {
        printToken(lookAhead);
        scan();
        compileParam();
    }
}

void compileParam(void) {
    if (lookAhead->tokenType == TK_IDENT) {
        printToken(lookAhead);
        scan();
        eat(SB_COLON);
        compileBasicType();
    } else if (lookAhead->tokenType == KW_VAR) {
        printToken(lookAhead);
        scan();
        eat(TK_IDENT);
        eat(SB_COLON);
        compileBasicType();
    } else {
        error(ERR_INVALIDPARAM, lookAhead->lineNo, lookAhead->colNo);
    }
}

void compileStatements(void) {
    compileStatement();
    compileStatements2();
}

void compileStatements2(void) {
    while (lookAhead->tokenType == SB_SEMICOLON) {
        printToken(lookAhead);
        scan();
        compileStatement();
    }
}

void compileStatement(void) {
  switch (lookAhead->tokenType) {
  case TK_IDENT:
    compileAssignSt();
    break;
  case KW_CALL:
    compileCallSt();
    break;
  case KW_BEGIN:
    compileGroupSt();
    break;
  case KW_IF:
    compileIfSt();
    break;
  case KW_WHILE:
    compileWhileSt();
    break;
  case KW_FOR:
    compileForSt();
    break;
    // EmptySt needs to check FOLLOW tokens
  case SB_SEMICOLON:
  case KW_END:
  case KW_ELSE:
    break;
    // Error occurs
  default:
    error(ERR_INVALIDSTATEMENT, lookAhead->lineNo, lookAhead->colNo);
    break;
  }
}

void compileAssignSt(void) {
  assert("Parsing an assign statement ....");
  eat(TK_IDENT);
  if (lookAhead->tokenType == SB_LSEL)
      compileIndexes();
  eat(SB_ASSIGN);
  compileExpression();
  assert("Assign statement parsed ....");
}

void compileCallSt(void) {
  assert("Parsing a call statement ....");
  eat(KW_CALL);
  eat(TK_IDENT);
  compileArguments();
  assert("Call statement parsed ....");
}

void compileGroupSt(void) {
  assert("Parsing a group statement ....");
  eat(KW_BEGIN);
  compileStatements();
  eat(KW_END);
  assert("Group statement parsed ....");
}

void compileIfSt(void) {
  assert("Parsing an if statement ....");
  eat(KW_IF);
  compileCondition();
  eat(KW_THEN);
  compileStatement();
  if (lookAhead->tokenType == KW_ELSE) 
    compileElseSt();
  assert("If statement parsed ....");
}

void compileElseSt(void) {
  eat(KW_ELSE);
  compileStatement();
}

void compileWhileSt(void) {
  assert("Parsing a while statement ....");
  eat(KW_WHILE);
  compileCondition();
  eat(KW_DO);
  compileStatement();
  assert("While statement parsed ....");
}

void compileForSt(void) {
  assert("Parsing a for statement ....");
  eat(KW_FOR);
  eat(TK_IDENT);
  eat(SB_ASSIGN);
  compileExpression();
  eat(KW_TO);
  compileExpression();
  eat(KW_DO);
  compileStatement();
  assert("For statement parsed ....");
}

void compileArguments(void) {
    if (lookAhead->tokenType == SB_LPAR) {
        printToken(lookAhead);
        scan();
        compileExpression();
        compileArguments2();
        eat(SB_RPAR);
    }
}

void compileArguments2(void) {
    while (lookAhead->tokenType == SB_COMMA) {
        printToken(lookAhead);
        scan();
        compileExpression();
    }
}

void compileCondition(void) {
    compileExpression();
    compileCondition2();
}

void compileCondition2(void) {
    switch (lookAhead->tokenType) {
        case SB_EQ:
        case SB_NEQ:
        case SB_LE:
        case SB_LT:
        case SB_GE:
        case SB_GT:
            printToken(lookAhead);
            scan();
            compileExpression();
            break;
        default:
            error(ERR_INVALIDCOMPARATOR, lookAhead->lineNo, lookAhead->colNo);
            break;
    }
}

void compileExpression(void) {
  assert("Parsing an expression");
  if (lookAhead->tokenType == SB_PLUS || lookAhead->tokenType == SB_MINUS) {
      printToken(lookAhead);
      scan();
      compileExpression2();
  } else
      compileExpression2();
  assert("Expression parsed");
}

void compileExpression2(void) {
    compileTerm();
    compileExpression3();
}


void compileExpression3(void) {
    while (lookAhead->tokenType == SB_PLUS || lookAhead->tokenType == SB_MINUS)
    {
        printToken(lookAhead);
        scan();
        compileTerm();
    }
}

void compileTerm(void) {
    compileFactor();
    compileTerm2();
}

void compileTerm2(void) {
    while (lookAhead->tokenType == SB_TIMES || lookAhead->tokenType == SB_SLASH)
    {
        printToken(lookAhead);
        scan();
        compileFactor();
    }
}

void compileFactor(void) {
    switch (lookAhead->tokenType) {
        case TK_IDENT:
            printToken(lookAhead);
            scan();
            if (lookAhead->tokenType == SB_LSEL)
                compileIndexes();
            else if (lookAhead->tokenType == SB_LPAR)
                compileArguments();
            break;
        case SB_LPAR:
            printToken(lookAhead);
            scan();
            compileExpression();
            eat(SB_RPAR);
            break;
        case TK_NUMBER:
        case TK_CHAR:
            compileUnsignedConstant();
            break;
        default:
            error(ERR_INVALIDFACTOR, lookAhead->lineNo, lookAhead->colNo);
    }
}

void compileIndexes(void) {
    while (lookAhead->tokenType == SB_LSEL) {
        printToken(lookAhead);
        scan();
        compileExpression();
        eat(SB_RSEL);
    }
}

int compile(char *fileName) {
  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  currentToken = NULL;
  lookAhead = getValidToken();

  compileProgram();

  free(currentToken);
  free(lookAhead);
  closeInputStream();
  return IO_SUCCESS;

}
