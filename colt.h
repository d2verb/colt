#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <ctype.h>
#include <stdnoreturn.h>
#include <assert.h>
#include <stdbool.h>

/*************************************************
 * util.c
 ************************************************/
noreturn void error(const char *fmt, ...);

typedef struct Vec {
  int len;
  int cap;
  void **data;
} Vec;

Vec* vec_new(void);
void vec_push(Vec* v, void* e);
void* vec_pop(Vec* v);
void vec_set(Vec* v, void* e, int n);
void* vec_get(Vec* v, int n);

typedef struct String {
  int len;
  int cap;
  char *data;
} String;

String* str_new(void);
void str_add(String* s, char c);
char* str_dup(String* s);
void str_clear(String* s);

typedef Vec Set;
Set* set_new(void);
void set_append(Set* s, char* e);
int set_has(Set* s, char* e);

/*************************************************
 * tokenize.c
 ************************************************/
typedef enum TokenId {
  // Keywords
  TokenIdFn,
  TokenIdIf,
  TokenIdElse,
  TokenIdReturn,
  TokenIdFor,
  TokenIdVar,
  TokenIdPrintln,

  // Symbols
  TokenIdLparen,
  TokenIdRparen,
  TokenIdLbrace,
  TokenIdRbrace,
  TokenIdSemi,
  TokenIdComma,

  // Operators
  TokenIdLt,
  TokenIdEqEq,
  TokenIdEq,
  TokenIdAdd,
  TokenIdSub,
  TokenIdMul,
  TokenIdDiv,

  // Etc
  TokenIdInt,
  TokenIdVarId,
  TokenIdFnId,
  TokenIdEof,
} TokenId;

typedef struct Token {
  TokenId id;

  union {
    int int_val;
    char *str_val;
  } data;

} Token;

Vec* tokenize(char* filename);
void dump_tokens(Vec* tokens);
const char* token_repr(TokenId id);

/*************************************************
 * parser.c
 ************************************************/
typedef enum NodeId {
  // Definition
  NodeIdFnDef,
  NodeIdVarDef,

  // Statement
  NodeIdIfStmt,
  NodeIdForStmt,
  NodeIdSetStmt,
  NodeIdRetStmt,
  NodeIdPrintStmt,

  // Expression
  NodeIdBinOp,
  NodeIdInt,
  NodeIdVarRef,

  // Statement & Expression
  NodeIdFnCall,
} NodeId;

typedef enum Operator {
  BinOpAdd,
  BinOpSub,
  BinOpMul,
  BinOpDiv,
  BinOpLt,
  BinOpEq,
} Operator;

typedef struct AstNode {
  NodeId id;

  union {
    // Function Definition
    struct {
      char* fn_name;
      Vec* params;
      Vec* body;
    } fn_def;

    // Variable Definition
    struct {
      char* var_name;
    } var_def;

    // Statements
    struct {
      struct AstNode* cond;
      Vec* then_block;
      Vec* else_block;
    } if_stmt;

    struct {
      struct AstNode* cond;
      Vec* body;
    } for_stmt;

    struct {
      char* dst;
      struct AstNode* src;
    } set_stmt;

    struct {
      struct AstNode* val;
    } ret_stmt;

    struct {
      struct AstNode* val;
    } print_stmt;

    // Expression
    struct {
      Operator op;
      struct AstNode* lhs;
      struct AstNode* rhs;
    } bin_op;

    struct {
      int val;
    } int_val;

    struct {
      char* var_name;
    } var_ref;

    // Statement & Expression
    struct {
      char* fn_name;
      Vec* args;
    } fn_call;
  };
} AstNode;

Vec* parse(Vec* _tokens);
void dump_ast(Vec* nodes);

/*************************************************
 * semcheck.c
 ************************************************/
void semcheck(Vec* nodes);
