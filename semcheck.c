#include "colt.h"

typedef struct FnSignature {
  char* fn_name;
  int params_num;
} FnSignature;

static Vec* fn_signatures;

static int fn_already_defined(char* fn_name)
{
  for (int i = 0; i < fn_signatures->len; i++) {
    FnSignature* fn_sig = vec_get(fn_signatures, i);
    if (!strcmp(fn_name, fn_sig->fn_name))
      return 1;
  }
  return 0;
}

static FnSignature* get_fn_signature(char* fn_name)
{
  for (int i = 0; i < fn_signatures->len; i++) {
    FnSignature* fn_sig = vec_get(fn_signatures, i);
    if (!strcmp(fn_name, fn_sig->fn_name))
      return fn_sig;
  }
  return NULL;
}

static void collect_fn_signatures(Vec* nodes)
{
  int main_fn_found = 0;

  for (int i = 0; i < nodes->len; i++) {
    AstNode* node = vec_get(nodes, i);

    if (fn_already_defined(node->fn_def.fn_name))
      error("`%s()` function is already defined", node->fn_def.fn_name);

    if (!main_fn_found && !strcmp(node->fn_def.fn_name, "Main"))
      main_fn_found = 1;

    FnSignature *fn_sig = malloc(sizeof(FnSignature));
    fn_sig->fn_name = node->fn_def.fn_name;
    fn_sig->params_num = node->fn_def.params->len;
    vec_push(fn_signatures, fn_sig);
  }

  if (!main_fn_found) 
      error("`Main()` function not found");
}

static void _reference_checker(AstNode* node, Set* env)
{
  switch (node->id) {
  case NodeIdVarDef:
    if (set_has(env, node->var_def.var_name)) error("`%s` is already defined", node->var_def.var_name);
    else set_append(env, node->var_def.var_name);
    break;
  case NodeIdIfStmt:
    _reference_checker(node->if_stmt.cond, env);
    for (int i = 0; i < node->if_stmt.then_block->len; i++) {
      _reference_checker(vec_get(node->if_stmt.then_block, i), env);
    }
    if (node->if_stmt.else_block != NULL) {
      for (int i = 0; i < node->if_stmt.else_block->len; i++) {
        _reference_checker(vec_get(node->if_stmt.else_block, i), env);
      }
    }
    break;
  case NodeIdForStmt:
    _reference_checker(node->for_stmt.cond, env);
    for (int i = 0; i < node->for_stmt.body->len; i++) {
      _reference_checker(vec_get(node->for_stmt.body, i), env);
    }
    break;
  case NodeIdSetStmt:
    if (!set_has(env, node->set_stmt.dst))
      error("`%s` is not defined", node->set_stmt.dst);
    _reference_checker(node->set_stmt.src, env);
    break;
  case NodeIdRetStmt:
    _reference_checker(node->ret_stmt.val, env);
    break;
  case NodeIdPrintStmt:
    _reference_checker(node->print_stmt.val, env);
    break;
  case NodeIdBinOp:
    _reference_checker(node->bin_op.lhs, env);
    _reference_checker(node->bin_op.rhs, env);
    break;
  case NodeIdVarRef:
    if (!set_has(env, node->var_ref.var_name))
      error("`%s` is not defined", node->var_ref.var_name);
    break;
  case NodeIdFnCall:
    if (!fn_already_defined(node->fn_call.fn_name))
      error("`%s()` is not defined", node->fn_call.fn_name);
    {
      FnSignature* fn_sig = get_fn_signature(node->fn_call.fn_name);
      if (fn_sig->params_num != node->fn_call.args->len)
        error("`%s()` has %d parameters. got %d arguments", node->fn_call.fn_name, fn_sig->params_num, node->fn_call.args->len);
    }
    break;
  }
}

static void reference_checker(Vec* nodes)
{
  for (int i = 0; i < nodes->len; i++) {
    Set* env = set_new();
    AstNode* node = vec_get(nodes, i);

    for (int j = 0; j < node->fn_def.params->len; j++) {
      char* param = vec_get(node->fn_def.params, j);
      if (set_has(env, param))
        error("`%s` is already defined", param);
      set_append(env, param);
    }

    for (int j = 0; j < node->fn_def.body->len; j++)
      _reference_checker(vec_get(node->fn_def.body, j), env);
  }
}

void semcheck(Vec* nodes) {
  fn_signatures = vec_new();
  collect_fn_signatures(nodes);
  reference_checker(nodes);
}
