/* Scanner
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reader.h"
#include "charcode.h"
#include "token.h"
#include "error.h"


extern int lineNo;
extern int colNo;
extern int currentChar;

extern CharCode charCodes[];

#define NEXT_CHAR_IS(CC) (readChar() != EOF && charCodes[currentChar] == CC)
#define NEXT_CHAR_NOT(CC) !NEXT_CHAR_IS(CC)
#define CUR_CHAR_IS(CC) (currentChar != EOF && charCodes[currentChar] == CC)

#define CASE_CH_TK(CH, TK) case CH: \
                        token = makeToken(TK, lineNo, colNo); \
                        readChar(); return token

#define CASE_12CH_TK(CH1, CH2, TK1, TK2) case CH1: \
                        ln = lineNo, cn = colNo; \
                        if (NEXT_CHAR_IS(CH2)) { \
                            token = makeToken(TK2, ln, cn); \
                            readChar(); \
                        } else { \
                            token = makeToken(TK1, ln, cn); \
                        } \
                        return token

/***************************************************************/

void skipBlank() {
  // TODO
  while ( NEXT_CHAR_IS(CHAR_SPACE ));
}

void skipComment() {
  // TODO
  while (1) {
      while (NEXT_CHAR_NOT(CHAR_TIMES)) {
          if (currentChar == EOF) {
              error(ERR_ENDOFCOMMENT, lineNo, colNo);
          }
      }
      if (currentChar == EOF)
          return;
      if (NEXT_CHAR_IS(CHAR_RPAR)) {
          readChar();
          return;
      }
  }
}

Token* readIdentKeyword(void) {
  // TODO
  char ident[MAX_IDENT_LEN + 1] = {[0] = currentChar };
  int identLength = 1;
  int ln = lineNo, cn = colNo;
  while ((readChar() != EOF) && ( (charCodes[currentChar] == CHAR_LETTER) ||
                                  (charCodes[currentChar] == CHAR_DIGIT) ) )
  {
      if (identLength > MAX_IDENT_LEN) {
          Token *token = makeToken(TK_NONE, ln, cn);
          error(ERR_IDENTTOOLONG, lineNo, colNo);
          readChar(); 
          return token;
      }

      ident[identLength++] = currentChar;
  }

  ident[identLength] = '\0';
  TokenType tokenType = checkKeyword(ident);
  if (tokenType == TK_NONE)
      tokenType = TK_IDENT;

  Token *token = makeToken(tokenType, ln, cn);
  strncpy(token->string, ident, identLength);
  return token;
}

Token* readNumber(void) {
  // TODO
  char number[MAX_NUMBER_LEN + 1] = {[0] = currentChar };
  int numberLength = 1;
  int ln = lineNo, cn = colNo;
  while (NEXT_CHAR_IS(CHAR_DIGIT)) {
      if (numberLength > MAX_NUMBER_LEN) {
          Token *token = makeToken(TK_NONE, ln, cn);
          error(ERR_NUMBERTOOLONG, lineNo, colNo);
          readChar(); 
          return token;
      }

      number[numberLength++] = currentChar;
  }

  Token *token = makeToken(TK_NUMBER, ln, cn);
  number[numberLength] = '\0';
  strncpy(token->string, number, numberLength);
  token->value = atoi(number);
  return token;
}

Token* readConstChar(void) {
  // TODO
  int ln = lineNo, cn = colNo;
  int ch = readChar();
  if (ch == EOF) {
      goto invalid;
  } else if (charCodes[ch] != CHAR_SINGLEQUOTE) {
    if(NEXT_CHAR_NOT(CHAR_SINGLEQUOTE))
        goto invalid;
  } else if ( (NEXT_CHAR_NOT(CHAR_SINGLEQUOTE)) ||
              (NEXT_CHAR_NOT(CHAR_SINGLEQUOTE))   )
      goto invalid;

  Token *token = makeToken(TK_CHAR, ln, cn);
  token->value = ch;
  token->string[0] = ch;
  token->string[1] = '\0';

  readChar();
  return token;

invalid:
  token = makeToken(TK_NONE, ln, cn);
  error(ERR_INVALIDCHARCONSTANT, ln, cn);
  readChar(); 
  return token;
}

Token* getToken(void) {
  Token *token;
  int ln, cn;

  if (currentChar == EOF) 
    return makeToken(TK_EOF, lineNo, colNo);

  switch (charCodes[currentChar]) {
  case CHAR_SPACE: skipBlank(); return getToken();
  case CHAR_LETTER: return readIdentKeyword();
  case CHAR_DIGIT: return readNumber();
  case CHAR_PLUS: 
    token = makeToken(SB_PLUS, lineNo, colNo);
    readChar(); 
    return token;
    // ....
    // TODO
    // ....
  CASE_CH_TK(CHAR_MINUS, SB_MINUS);
  CASE_CH_TK(CHAR_TIMES, SB_TIMES);
  CASE_CH_TK(CHAR_SLASH, SB_SLASH);
  CASE_12CH_TK(CHAR_LT, CHAR_EQ, SB_LT, SB_LE);
  CASE_12CH_TK(CHAR_GT, CHAR_EQ, SB_GT, SB_GE);
  case CHAR_EXCLAIMATION:
    ln = lineNo, cn = colNo;
    if (NEXT_CHAR_IS(CHAR_EQ)) {
        token = makeToken(SB_NEQ, ln, cn);
        readChar();
        return token;
    } else {
        token = makeToken(TK_NONE, ln, cn);
        error(ERR_INVALIDSYMBOL, ln, cn);
        readChar();
        return token;
    }
  CASE_CH_TK(CHAR_EQ, SB_EQ);
  CASE_CH_TK(CHAR_COMMA, SB_COMMA);
  CASE_12CH_TK(CHAR_PERIOD, CHAR_RPAR, SB_PERIOD, SB_RSEL);
  CASE_12CH_TK(CHAR_COLON, CHAR_EQ, SB_COLON, SB_ASSIGN);
  CASE_CH_TK(CHAR_SEMICOLON, SB_SEMICOLON);
  case CHAR_SINGLEQUOTE: return readConstChar();
  case CHAR_LPAR:
    ln = lineNo, cn = colNo;
    if (NEXT_CHAR_IS(CHAR_TIMES)) {
        skipComment();
        return getToken();
    } else if (CUR_CHAR_IS(CHAR_PERIOD)) {
        token = makeToken(SB_LSEL, ln, cn);
        readChar();
    } else {
        token = makeToken(SB_LPAR, ln, cn);
    }
    return token;
  CASE_CH_TK(CHAR_RPAR, SB_RPAR);
  default:
    token = makeToken(TK_NONE, lineNo, colNo);
    error(ERR_INVALIDSYMBOL, lineNo, colNo);
    readChar(); 
    return token;
  }
}


/******************************************************************/

void printToken(Token *token) {

  printf("%d-%d:", token->lineNo, token->colNo);

  switch (token->tokenType) {
  case TK_NONE: printf("TK_NONE\n"); break;
  case TK_IDENT: printf("TK_IDENT(%s)\n", token->string); break;
  case TK_NUMBER: printf("TK_NUMBER(%s)\n", token->string); break;
  case TK_CHAR: printf("TK_CHAR(\'%s\')\n", token->string); break;
  case TK_EOF: printf("TK_EOF\n"); break;

  case KW_PROGRAM: printf("KW_PROGRAM\n"); break;
  case KW_CONST: printf("KW_CONST\n"); break;
  case KW_TYPE: printf("KW_TYPE\n"); break;
  case KW_VAR: printf("KW_VAR\n"); break;
  case KW_INTEGER: printf("KW_INTEGER\n"); break;
  case KW_CHAR: printf("KW_CHAR\n"); break;
  case KW_ARRAY: printf("KW_ARRAY\n"); break;
  case KW_OF: printf("KW_OF\n"); break;
  case KW_FUNCTION: printf("KW_FUNCTION\n"); break;
  case KW_PROCEDURE: printf("KW_PROCEDURE\n"); break;
  case KW_BEGIN: printf("KW_BEGIN\n"); break;
  case KW_END: printf("KW_END\n"); break;
  case KW_CALL: printf("KW_CALL\n"); break;
  case KW_IF: printf("KW_IF\n"); break;
  case KW_THEN: printf("KW_THEN\n"); break;
  case KW_ELSE: printf("KW_ELSE\n"); break;
  case KW_WHILE: printf("KW_WHILE\n"); break;
  case KW_DO: printf("KW_DO\n"); break;
  case KW_FOR: printf("KW_FOR\n"); break;
  case KW_TO: printf("KW_TO\n"); break;

  case SB_SEMICOLON: printf("SB_SEMICOLON\n"); break;
  case SB_COLON: printf("SB_COLON\n"); break;
  case SB_PERIOD: printf("SB_PERIOD\n"); break;
  case SB_COMMA: printf("SB_COMMA\n"); break;
  case SB_ASSIGN: printf("SB_ASSIGN\n"); break;
  case SB_EQ: printf("SB_EQ\n"); break;
  case SB_NEQ: printf("SB_NEQ\n"); break;
  case SB_LT: printf("SB_LT\n"); break;
  case SB_LE: printf("SB_LE\n"); break;
  case SB_GT: printf("SB_GT\n"); break;
  case SB_GE: printf("SB_GE\n"); break;
  case SB_PLUS: printf("SB_PLUS\n"); break;
  case SB_MINUS: printf("SB_MINUS\n"); break;
  case SB_TIMES: printf("SB_TIMES\n"); break;
  case SB_SLASH: printf("SB_SLASH\n"); break;
  case SB_LPAR: printf("SB_LPAR\n"); break;
  case SB_RPAR: printf("SB_RPAR\n"); break;
  case SB_LSEL: printf("SB_LSEL\n"); break;
  case SB_RSEL: printf("SB_RSEL\n"); break;
  }
}

int scan(char *fileName) {
  Token *token;

  if (openInputStream(fileName) == IO_ERROR)
    return IO_ERROR;

  token = getToken();
  while (token->tokenType != TK_EOF) {
    printToken(token);
    free(token);
    token = getToken();
  }

  free(token);
  closeInputStream();
  return IO_SUCCESS;
}

/******************************************************************/

int main(int argc, char *argv[]) {
  if (argc <= 1) {
    printf("scanner: no input file.\n");
    return -1;
  }

  if (scan(argv[1]) == IO_ERROR) {
    printf("Can\'t read input file!\n");
    return -1;
  }
    
  return 0;
}



