#include "colt.h"

static char *code;
static int cur_pos = 0;

static void skip_white(void)
{
  for (;;) {
    char ch = code[cur_pos];
    if (!strchr(" \t\n\v", ch) || ch == 0)
      break;
    cur_pos += 1;
  }
}

static Token* token_new(int id)
{
  Token* token = malloc(sizeof(Token));
  token->id = id;
  return token;
}

static Token *make_int_token(void)
{
  int val = 0;
  while (isdigit(code[cur_pos])) {
    val = val * 10 + (code[cur_pos] - '0');
    cur_pos += 1;
  }

  Token *token = token_new(TokenIdInt);
  token->data.int_val = val;
  
  return token;
}

static Token *make_id_token(TokenId id)
{
  String *s = str_new();
  while (isalnum(code[cur_pos]) || code[cur_pos] == '_') {
    str_add(s, code[cur_pos++]);
  }

  Token *token = token_new(id);
  token->data.str_val = str_dup(s);

  if (!strcmp(token->data.str_val, "fn")) token->id = TokenIdFn;
  if (!strcmp(token->data.str_val, "var")) token->id = TokenIdVar;
  if (!strcmp(token->data.str_val, "if")) token->id = TokenIdIf;
  if (!strcmp(token->data.str_val, "else")) token->id = TokenIdElse;
  if (!strcmp(token->data.str_val, "for")) token->id = TokenIdFor;
  if (!strcmp(token->data.str_val, "return")) token->id = TokenIdReturn;
  if (!strcmp(token->data.str_val, "println")) token->id = TokenIdPrintln;

  return token;
}

void read_code(char* filename)
{
  FILE *fp = fopen(filename, "r");
  fseek(fp, 0, SEEK_END);
  long fsize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  code = malloc(fsize + 1);
  fread(code, fsize, 1, fp);
  fclose(fp);

  code[fsize] = 0;
}

Vec* tokenize(char* filename)
{
  // Open & read file contents
  read_code(filename);
  Vec *tokens = vec_new();

  // Tokenize
  for (;;) {
    skip_white();

    switch (code[cur_pos]) {
    case ';':
      vec_push(tokens, token_new(TokenIdSemi));
      cur_pos++;
      break;
    case ',':
      vec_push(tokens, token_new(TokenIdComma));
      cur_pos++;
      break;
    case '+':
      vec_push(tokens, token_new(TokenIdAdd));
      cur_pos++;
      break;
    case '-':
      vec_push(tokens, token_new(TokenIdSub));
      cur_pos++;
      break;
    case '*':
      vec_push(tokens, token_new(TokenIdMul));
      cur_pos++;
      break;
    case '/':
      vec_push(tokens, token_new(TokenIdDiv));
      cur_pos++;
      break;
    case '<':
      vec_push(tokens, token_new(TokenIdLt));
      cur_pos++;
      break;
    case '(':
      vec_push(tokens, token_new(TokenIdLparen));
      cur_pos++;
      break;
    case ')':
      vec_push(tokens, token_new(TokenIdRparen));
      cur_pos++;
      break;
    case '{':
      vec_push(tokens, token_new(TokenIdLbrace));
      cur_pos++;
      break;
    case '}':
      vec_push(tokens, token_new(TokenIdRbrace));
      cur_pos++;
      break;
    case '0'...'9':
      vec_push(tokens, make_int_token());
      break;
    case 'a'...'z':
      vec_push(tokens, make_id_token(TokenIdVarId));
      break;
    case 'A'...'Z':
      vec_push(tokens, make_id_token(TokenIdFnId));
      break;
    case '=':
      if (code[cur_pos+1] == '=') {
        cur_pos++;
        vec_push(tokens, token_new(TokenIdEqEq));
      } else {
        vec_push(tokens, token_new(TokenIdEq));
      }
      cur_pos++;
      break;
    case 0:
      vec_push(tokens, token_new(TokenIdEof));
      return tokens;
    default:
      error("Invalid character is found `%c`\n", code[cur_pos]);
    }
  }
}

void dump_tokens(Vec* tokens)
{
  Token *token;
  for (int i = 0; i < tokens->len; i++) {
    token = vec_get(tokens, i);
    printf("[%02d] %s ", i, token_repr(token->id));
    switch (token->id) {
      case TokenIdInt:
        printf("%d", token->data.int_val);
        break;
      case TokenIdVarId:
      case TokenIdFnId:
        printf("%s", token->data.str_val);
        break;
    }
    printf("\n");
  }
}

const char* token_repr(TokenId id)
{
  switch (id) {
  case TokenIdAdd: return "+";
  case TokenIdSub: return "-";
  case TokenIdMul: return "*";
  case TokenIdDiv: return "/";
  case TokenIdEq: return "=";
  case TokenIdEqEq: return "==";
  case TokenIdLt: return "<";
  case TokenIdLparen: return "(";
  case TokenIdRparen: return ")";
  case TokenIdLbrace: return "{";
  case TokenIdRbrace: return "}";
  case TokenIdSemi: return ";";
  case TokenIdComma: return ",";
  case TokenIdFn: return "fn";
  case TokenIdVar: return "var";
  case TokenIdFor: return "for";
  case TokenIdIf: return "if";
  case TokenIdElse: return "else";
  case TokenIdReturn: return "return";
  case TokenIdVarId: return "<VarId>";
  case TokenIdFnId: return "<FnId>";
  case TokenIdInt: return "<Int>";
  case TokenIdEof: return "<Eof>";
  default: return "<Unknown>";
  }
}
