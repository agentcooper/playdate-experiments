#include "pd_api.h"

static PlaydateAPI *pd;

struct StackItem {
  int x;
  int y;
};

struct Stack {
  int max;
  int top;
  struct StackItem *item;
};

struct Stack *stack_create(int m) {
  struct Stack *S =
      (struct Stack *)pd->system->realloc(NULL, sizeof(struct Stack));
  S->max = m;
  S->top = -1;
  S->item = (struct StackItem *)pd->system->realloc(
      NULL, m * sizeof(struct StackItem));
  return S;
}

int stack_is_empty(struct Stack *s) { return (s->top == -1); }

void stack_push(struct StackItem x, struct Stack *S) {
  S->top++;
  S->item[S->top] = x;
}

struct StackItem stack_pop(struct Stack *S) {
  struct StackItem x = S->item[S->top];
  S->top--;
  return x;
}

struct StackItem stack_top(struct Stack *S) { return S->item[S->top]; }

void stack_destroy(struct Stack *S) {
  pd->system->realloc(S->item, 0);
  pd->system->realloc(S, 0);
  S = NULL;
}