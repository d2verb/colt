#include "colt.h"

static Vec* tokens;
static int cur_pos;

/************************************************
 * Helper functions
 ************************************************/
static Token* cur_token(void)
{
  return (Token*)vec_get(tokens, cur_pos);
}

static TokenId cur_token_id(void)
{
  return cur_token()->id;
}

static void expect(TokenId id)
{
  if (cur_token_id() != id)
    error("%s is expected, but got %s", token_repr(id), token_repr(cur_token_id()));
}

static void eat(TokenId id)
{
  expect(id);
  cur_pos++;
}

static AstNode* make_node(NodeId id)
{
  AstNode* node = malloc(sizeof(AstNode));
  node->id = id;
  return node;
}

/************************************************
 * Parser functions
 ************************************************/
AstNode* parse_expr(void);
AstNode* parse_stmt(void);
AstNode* parse_fn_call(void);
AstNode* parse_fn_call_stmt(void);

AstNode* parse_pri_expr(void) {
  AstNode* node;

  switch (cur_token_id()) {
  case TokenIdInt:
    node = make_node(NodeIdInt);
    node->int_val.val = cur_token()->data.int_val;
    eat(TokenIdInt);
    break;
  case TokenIdVarId:
    node = make_node(NodeIdVarRef);
    node->var_ref.var_name = cur_token()->data.str_val;
    eat(TokenIdVarId);
    break;
  case TokenIdFnId:
    node = parse_fn_call();
    break;
  case TokenIdLparen:
    eat(TokenIdLparen);
    node = parse_expr();
    eat(TokenIdRparen);
    break;
  }

  return node;
}

AstNode* parse_mul_expr(void)
{
  AstNode* lhs = parse_pri_expr();
  AstNode* rhs;
  AstNode* node;

  switch (cur_token_id()) {
  case TokenIdMul:
    eat(TokenIdMul);
    rhs = parse_mul_expr();
    node = make_node(NodeIdBinOp);
    node->bin_op.op = BinOpMul;
    node->bin_op.lhs = lhs;
    node->bin_op.rhs = rhs;
    return node;
  case TokenIdDiv:
    eat(TokenIdDiv);
    rhs = parse_mul_expr();
    node = make_node(NodeIdBinOp);
    node->bin_op.op = BinOpDiv;
    node->bin_op.lhs = lhs;
    node->bin_op.rhs = rhs;
    return node;
  default:
    return lhs;
  }
}

AstNode* parse_add_expr(void)
{
  AstNode* lhs = parse_mul_expr();
  AstNode* rhs;
  AstNode* node;

  switch (cur_token_id()) {
  case TokenIdAdd:
    eat(TokenIdAdd);
    rhs = parse_add_expr();
    node = make_node(NodeIdBinOp);
    node->bin_op.op = BinOpAdd;
    node->bin_op.lhs = lhs;
    node->bin_op.rhs = rhs;
    return node;
  case TokenIdSub:
    eat(TokenIdSub);
    rhs = parse_add_expr();
    node = make_node(NodeIdBinOp);
    node->bin_op.op = BinOpSub;
    node->bin_op.lhs = lhs;
    node->bin_op.rhs = rhs;
    return node;
  default:
    return lhs;
  }
}

AstNode* parse_cond_expr(void)
{
  AstNode* lhs = parse_add_expr();
  AstNode* rhs;
  AstNode* node;

  switch (cur_token_id()) {
  case TokenIdEqEq:
    eat(TokenIdEqEq);
    rhs = parse_cond_expr();
    node = make_node(NodeIdBinOp);
    node->bin_op.op = BinOpEq;
    node->bin_op.lhs = lhs;
    node->bin_op.rhs = rhs;
    return node;
  case TokenIdLt:
    eat(TokenIdLt);
    rhs = parse_cond_expr();
    node = make_node(NodeIdBinOp);
    node->bin_op.op = BinOpLt;
    node->bin_op.lhs = lhs;
    node->bin_op.rhs = rhs;
    return node;
  default:
    return lhs;
  }
}

AstNode* parse_expr(void)
{
  return parse_cond_expr();
}

AstNode* parse_ret_stmt(void)
{
  AstNode* node = make_node(NodeIdRetStmt);
  eat(TokenIdReturn);
  node->ret_stmt.val = parse_expr();
  eat(TokenIdSemi);
  return node;
}

AstNode* parse_for_stmt(void)
{
  AstNode* node = make_node(NodeIdForStmt);
  eat(TokenIdFor);
  node->for_stmt.cond = parse_expr();
  eat(TokenIdLbrace);
  node->for_stmt.body = vec_new();
  while (cur_token_id() != TokenIdRbrace) {
    vec_push(node->for_stmt.body, parse_stmt());
  }
  eat(TokenIdRbrace);
  return node;
}

AstNode* parse_if_stmt(void)
{
  AstNode* node = make_node(NodeIdIfStmt);
  eat(TokenIdIf);
  node->if_stmt.cond = parse_expr();

  node->if_stmt.then_block = NULL;
  node->if_stmt.else_block = NULL;

  eat(TokenIdLbrace);
  node->if_stmt.then_block = vec_new();
  while (cur_token_id() != TokenIdRbrace) {
    vec_push(node->if_stmt.then_block, parse_stmt());
  }
  eat(TokenIdRbrace);

  if (cur_token_id() != TokenIdElse)
    return node;

  eat(TokenIdElse);

  eat(TokenIdLbrace);
  node->if_stmt.else_block = vec_new();
  while (cur_token_id() != TokenIdRbrace) {
    vec_push(node->if_stmt.else_block, parse_stmt());
  }
  eat(TokenIdRbrace);

  return node;
}

AstNode* parse_fn_call_stmt(void)
{
  AstNode* node = parse_fn_call();
  eat(TokenIdSemi);
  return node;
}

AstNode* parse_fn_call(void)
{
  AstNode* node = make_node(NodeIdFnCall);
  node->fn_call.fn_name = cur_token()->data.str_val;
  node->fn_call.args = vec_new();

  eat(TokenIdFnId);
  eat(TokenIdLparen);
  while (cur_token_id() != TokenIdRparen) {
    vec_push(node->fn_call.args, parse_expr());
    if (cur_token_id() == TokenIdComma)
      eat(TokenIdComma);
  }
  eat(TokenIdRparen);

  return node;
}

AstNode* parse_set_stmt(void)
{
  AstNode* node = make_node(NodeIdSetStmt);
  node->set_stmt.dst = cur_token()->data.str_val;
  eat(TokenIdVarId);
  eat(TokenIdEq);
  node->set_stmt.src = parse_expr();
  eat(TokenIdSemi);
  return node;
}

AstNode* parse_print_stmt(void)
{
  AstNode* node = make_node(NodeIdPrintStmt);
  eat(TokenIdPrintln);
  eat(TokenIdLparen);
  node->print_stmt.val = parse_expr();
  eat(TokenIdRparen);
  eat(TokenIdSemi);
  return node;
}

AstNode* parse_stmt(void)
{
  switch (cur_token_id()) {
  case TokenIdVarId: return parse_set_stmt();
  case TokenIdFnId: return parse_fn_call_stmt();
  case TokenIdIf: return parse_if_stmt();
  case TokenIdFor: return parse_for_stmt();
  case TokenIdReturn: return parse_ret_stmt();
  case TokenIdPrintln: return parse_print_stmt();
  default: error("Unexpected token `%s` is found\n", token_repr(cur_token_id()));
  }
}

AstNode* parse_var_def(void)
{
  AstNode* node = make_node(NodeIdVarDef);
  eat(TokenIdVar);
  node->var_def.var_name = cur_token()->data.str_val;
  eat(TokenIdVarId);
  eat(TokenIdSemi);
  return node;
}

AstNode* parse_fn_def(void)
{
  AstNode* node = make_node(NodeIdFnDef);
  eat(TokenIdFn);

  node->fn_def.fn_name = cur_token()->data.str_val;
  node->fn_def.params = vec_new();
  node->fn_def.body = vec_new();

  eat(TokenIdFnId);
  eat(TokenIdLparen);

  // Parse parameters
  // e.g. "(a, b, c, d)"
  while (cur_token_id() != TokenIdRparen) {
    expect(TokenIdVarId);

    vec_push(node->fn_def.params, cur_token()->data.str_val);
    eat(TokenIdVarId);
    if (cur_token_id() == TokenIdComma)
      eat(TokenIdComma);
  }

  eat(TokenIdRparen);
  eat(TokenIdLbrace);

  // Parse variable definition
  while (cur_token_id() == TokenIdVar) {
    vec_push(node->fn_def.body, parse_var_def());
  }

  // Parse statements
  while (cur_token_id() != TokenIdRbrace) {
    vec_push(node->fn_def.body, parse_stmt());
  }

  eat(TokenIdRbrace);

  return node;
}

Vec* parse(Vec* _tokens)
{
  tokens = _tokens;
  Vec* fn_defs = vec_new();

  while (cur_token_id() != TokenIdEof) {
    expect(TokenIdFn);
    vec_push(fn_defs, parse_fn_def());
  }

  return fn_defs;
}

static const char* op_repr(Operator op)
{
  switch (op) {
  case BinOpAdd: return "+";
  case BinOpSub: return "-";
  case BinOpMul: return "*";
  case BinOpDiv: return "/";
  case BinOpLt: return "<";
  case BinOpEq: return "==";
  }
}

static void indent(int depth)
{
  for (int i = 0; i < depth * 4; i++)
    printf(" ");
}

static void _dump_ast(AstNode* node, int depth)
{
  switch (node->id) {
  case NodeIdFnDef:
    indent(depth); printf("<<Function>>\n");
    indent(depth); printf("name: %s\n", node->fn_def.fn_name);
    indent(depth); printf("params: ");
    for (int i = 0; i < node->fn_def.params->len; i++) {
      if (i) printf(", ");
      printf("%s", (char *)vec_get(node->fn_def.params, i));
    }
    printf("\n");
    indent(depth); printf("body:\n");
    for (int i = 0; i < node->fn_def.body->len; i++) {
      _dump_ast(vec_get(node->fn_def.body, i), depth + 1);
    }
    break;
  case NodeIdVarDef:
    indent(depth); printf("<<Variable Definition>>\n");
    indent(depth); printf("name: %s\n", node->var_def.var_name);
    break;
  case NodeIdIfStmt:
    indent(depth); printf("<<If Statement>>\n");
    indent(depth); printf("condition:\n");
    _dump_ast(node->if_stmt.cond, depth + 1);
    indent(depth); printf("then:\n");
    for (int i = 0; i < node->if_stmt.then_block->len; i++) {
      _dump_ast(vec_get(node->if_stmt.then_block, i), depth + 1);
    }
    if (node->if_stmt.else_block != NULL) {
      indent(depth); printf("else:\n");
      for (int i = 0; i < node->if_stmt.else_block->len; i++) {
        _dump_ast(vec_get(node->if_stmt.else_block, i), depth + 1);
      }
    }
    break;
  case NodeIdForStmt:
    indent(depth); printf("<<For Statement>>\n");
    indent(depth); printf("condition:\n");
    _dump_ast(node->for_stmt.cond, depth + 1);
    indent(depth); printf("body:\n");
    for (int i = 0; i < node->for_stmt.body->len; i++) {
      _dump_ast(vec_get(node->for_stmt.body, i), depth + 1);
    }
    break;
  case NodeIdSetStmt:
    indent(depth); printf("<<Set Statement>>\n");
    indent(depth); printf("dst: %s\n", node->set_stmt.dst);
    indent(depth); printf("src:\n");
    _dump_ast(node->set_stmt.src, depth + 1);
    break;
  case NodeIdRetStmt:
    indent(depth); printf("<<Return Statement>>\n");
    indent(depth); printf("value:\n");
    _dump_ast(node->ret_stmt.val, depth + 1);
    break;
  case NodeIdPrintStmt:
    indent(depth); printf("<<Print Statement>>\n");
    indent(depth); printf("value:\n");
    _dump_ast(node->print_stmt.val, depth + 1);
    break;
  case NodeIdBinOp:
    indent(depth); printf("<<BinOp Statement>>\n");
    indent(depth); printf("Op: %s\n", op_repr(node->bin_op.op));
    indent(depth); printf("lhs:\n");
    _dump_ast(node->bin_op.lhs, depth + 1);
    indent(depth); printf("rhs:\n");
    _dump_ast(node->bin_op.rhs, depth + 1);
    break;
  case NodeIdInt:
    indent(depth); printf("<<Int Literal>>\n");
    indent(depth); printf("value: %d\n", node->int_val.val);
    break;
  case NodeIdVarRef:
    indent(depth); printf("<<Variable Reference>>\n");
    indent(depth); printf("name: %s\n", node->var_ref.var_name);
    break;
  case NodeIdFnCall:
    indent(depth); printf("<<Function Call>>\n");
    indent(depth); printf("name: %s\n", node->fn_call.fn_name);
    indent(depth); printf("args:\n");
    for (int i = 0; i < node->fn_call.args->len; i++) {
      _dump_ast(vec_get(node->fn_call.args, i), depth + 1);
    }
    break;
  }
}

void dump_ast(Vec* nodes)
{
  for (int i = 0; i < nodes->len; i++)
    _dump_ast(vec_get(nodes, i), 0);
}
