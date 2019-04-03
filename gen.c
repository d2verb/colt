#include "colt.h"

#define FN_PROLOGUE \
  "  push rbp\n" \
  "  mov rbp, rsp\n"

#define FN_EPILOGUE \
  "  mov rsp, rbp\n" \
  "  pop rbp\n" \
  "  ret\n"

// rdi: Given number
// rsi: Pointer to current stack top
// rcx: Is negative flag
// #define PRINTLN \
//   ".globl: println" \
//   "println:" \
//   "  push 0" \
//   "  mov rsi, rsp" \
//   "  inc rsi" \
//   "  mov byte ptr [rsi], 10" \
//   "  pop rax" \
//   FN_PROLOGUE \
//   FN_EPILOGUE

char *current_fn;
int label_no;

static int get_var_def_num(AstNode* fn_def)
{
  int i;
  for (i = 0; i < fn_def->fn_def.body->len; i++) {
    AstNode* node = vec_get(fn_def->fn_def.body, i);
    if (node->id != NodeIdVarDef)
      break;
  }
  return i;
}

static void calc_var_offset(AstNode* node, int var_def_num, Map* var_offset)
{
  int param_num = node->fn_def.params->len;

  for (int off = 8 * param_num + 8, i = 0; off > 8; off -= 8, i++) {
    map_set(var_offset, vec_get(node->fn_def.params, i), off);
  }

  for (int i = 0; i < var_def_num; i++) {
    AstNode* var_def = vec_get(node->fn_def.body, i);
    map_set(var_offset, var_def->var_def.var_name, -(i + 1) * 8);
  }
}

// Expression value will on the top of the stack
static void gen_expr(AstNode* node, Map* var_offset)
{
  switch (node->id) {
  case NodeIdInt:
    printf("  push %d\n", node->int_val.val);
    break;
  case NodeIdVarRef:
    {
      int off = map_get(var_offset, node->var_ref.var_name);
      printf("  mov rax, qword ptr [rbp%s%d]\n", off < 0 ? "" : "+", off);
      printf("  push rax\n");
    }
    break;
  case NodeIdBinOp:
    gen_expr(node->bin_op.lhs, var_offset);
    gen_expr(node->bin_op.rhs, var_offset);
    printf("  pop rcx\n");
    printf("  pop rax\n");
    switch (node->bin_op.op) {
    case BinOpAdd:
      printf("  add rax, rcx\n");
      printf("  push rax\n");
      break;
    case BinOpSub:
      printf("  sub rax, rcx\n");
      printf("  push rax\n");
      break;
    case BinOpMul:
      printf("  imul rax, rcx\n");
      printf("  push rax\n");
      break;
    case BinOpDiv:
      printf("  idiv rax, rcx\n");
      printf("  push rax\n");
      break;
    }
    break;
  }
}

static void gen_stmt(AstNode* node, Map* var_offset)
{
  switch (node->id) {
  default:
    error("Not implemented error in gen_stmt()");
    break;
  case NodeIdFnCall:
    for (int i = 0; i < node->fn_call.args->len; i++) {
      gen_expr(vec_get(node->fn_call.args, i), var_offset);
    }
    printf("  call %s\n", node->fn_call.fn_name);
    break;
  case NodeIdSetStmt:
    {
      int off = map_get(var_offset, node->set_stmt.dst);
      gen_expr(node->set_stmt.src, var_offset);
      printf("  pop rax\n");
      printf("  mov qword ptr [rbp%s%d], rax\n", off < 0 ? "" : "+", off);
    }
    break;
  case NodeIdRetStmt:
    gen_expr(node->ret_stmt.val, var_offset);
    printf("  pop rax\n");
    printf("  jmp %s_EPILOGUE\n", current_fn);
    break;
  }
}

static void gen_fn(AstNode* node)
{
  char *fn_name;

  if (!strcmp(node->fn_def.fn_name, "Main")) fn_name = "main";
  else fn_name = node->fn_def.fn_name;

  current_fn = fn_name;
  label_no = 0;

  int var_def_num = get_var_def_num(node);
  Map* var_offset = map_new();
  calc_var_offset(node, var_def_num, var_offset);

  // gen function prologue
  printf(".global %s\n", fn_name);
  printf("%s:\n", fn_name);
  printf("%s", FN_PROLOGUE);
  if (var_def_num) printf("  sub rsp, %d\n", var_def_num * 8);

  // gen body
  for (int i = var_def_num; i < node->fn_def.body->len; i++)
    gen_stmt(vec_get(node->fn_def.body, i), var_offset);

  // gen function epilogue
  if (var_def_num) printf("  add rsp, %d\n", var_def_num * 8);
  printf("%s_EPILOGUE:\n", current_fn);
  printf("%s", FN_EPILOGUE);
}

void gen_x86(Vec* nodes)
{
  printf(".intel_syntax noprefix\n");

  for (int i = 0; i < nodes->len; i++) {
    if (i) printf("\n");
    gen_fn(vec_get(nodes, i));
  }
}
