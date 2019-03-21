#include "colt.h"

noreturn void error(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "\033[0;31m[Error]\033[0;39m ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

/* vector */
static void vec_expand(Vec* v)
{
  v->cap *= 2;
  v->data = realloc(v->data, sizeof(void*) * v->cap);
}

Vec* vec_new(void)
{
  Vec* v = malloc(sizeof(Vec));
  v->data = malloc(sizeof(void*) * 4);
  v->len = 0;
  v->cap = 4;
}

void vec_push(Vec* v, void* e)
{
  if (v->len >= v->cap)
    vec_expand(v);
  v->data[v->len++] = e;
}

void* vec_pop(Vec* v)
{
  assert(v->len > 0);
  return v->data[--v->len];
}

void vec_set(Vec* v, void *e, int n)
{
  assert(n >= 0 && n < v->len);
  v->data[n] = e;
}

void* vec_get(Vec* v, int n)
{
  assert(n >= 0 && n < v->len);
  return v->data[n];
}

/* string */
static void str_expand(String* s)
{
  s->cap *= 2;
  s->data = realloc(s->data, sizeof(char) * s->cap);
}

String* str_new(void)
{
  String* v = malloc(sizeof(String));
  v->data = malloc(sizeof(char) * 8);
  v->len = 0;
  v->cap = 8;
}

void str_add(String* s, char c)
{
  if (s->len >= s->cap)
    str_expand(s);
  s->data[s->len++] = c;
}

char* str_dup(String* s)
{
  char *dupped = malloc(sizeof(char) * s->len + 1);
  memcpy(dupped, s->data, s->len);
  dupped[s->len] = 0;
  return dupped;
}

void str_clear(String* s)
{
  s->len = 0;
}

/* set */
Set* set_new(void)
{
  Set* s = (Set*)vec_new();
  return s;
}

void set_append(Set* s, char* e)
{
  Vec *v = (Vec*)s;
  for (int i = 0; i < v->len; i++) {
    if (!strcmp(vec_get(v, i), e))
      return;
  }
  vec_push(v, e);
}

int set_has(Set* s, char* e)
{
  Vec *v = (Vec*)s;
  for (int i = 0; i < v->len; i++) {
    if (!strcmp(vec_get(v, i), e))
      return 1;
  }
  return 0;
}
