#include "colt.h"

const char* regs[] = {
  "rdi", "rsi", "rdx", "rcx", "r8", "r9",
  "rax", "rbx", "r10", "r11", "r12", "r13", "r14", "r15",
};

#define FN_PROLOGUE \
  "  push rbp\n" \
  "  mov rbp, rsp\n"

#define FN_EPILOGUE \
  "  mov rsp, rbp\n" \
  "  pop rbp\n" \
  "  ret\n"

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

static int alloc_stack(AstNode* node)
{
  int param_num = node->fn_def.params->len;
  int var_def_num = get_var_def_num(node);
  int var_num = param_num + var_def_num;

  if (var_num) printf("  sub rsp, %d\n", var_num * 8);

  int i;
  Map* var_offset = map_new();

  // Move all local variables (including parameters) to allocated stack area
  for (i = 0; i < MIN(param_num, 6); i++) {
    map_set(var_offset, vec_get(node->fn_def.params, i), (i + 1) * 8);
    printf("  mov qword ptr [rbp-%d], %s\n", (i + 1) * 8, regs[i]);
  }

  for (; i >= 6 && i < param_num; i++) {
    AstNode* var_def = vec_get(node->fn_def.params, i);
    map_set(var_offset, var_def->var_def.var_name, (i + 1) * 8);
    printf("  mov rdi, qword ptr [rbp+%d]\n", 16 + (param_num - i - 1) * 8);
    printf("  mov qword ptr [rbp-%d], rdi\n", (i + 1) * 8);
  }

  return var_num;
}

static void dealloc_stack(int var_num)
{
  if (!var_num)
    return;
  printf("  add rsp, %d\n", var_num * 8);
}

static void gen_fn(AstNode* node)
{
  char *fn_name;

  if (!strcmp(node->fn_def.fn_name, "Main")) fn_name = "main";
  else fn_name = node->fn_def.fn_name;

  printf(".global %s\n", fn_name);
  printf("%s:\n", fn_name);

  int var_num;
  printf("%s", FN_PROLOGUE);
  var_num = alloc_stack(node);
  dealloc_stack(var_num);
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
