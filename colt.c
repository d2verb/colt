#include "colt.h"

int main(int argc, char* argv[])
{
  if (argc != 2) {
    fprintf(stderr, "Usage: colt FILE\n");
    exit(1);
  }
  Vec *tokens = tokenize(argv[1]);
  Vec* nodes = parse(tokens);
  dump_ast(nodes);
  semcheck(nodes);
}
