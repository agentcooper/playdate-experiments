#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "pd_api.h"

static PlaydateAPI *pd = NULL;

#include "stack.h"

#define setPixel(data, x, y, rowbytes)                                         \
  (data[(y) * rowbytes + (x) / 8] &= ~(1 << (uint8_t)(7 - ((x) % 8))))

#define samplePixel(data, x, y, rowbytes)                                      \
  (((data[(y) * rowbytes + (x) / 8] & (1 << (uint8_t)(7 - ((x) % 8)))) != 0)   \
       ? kColorWhite                                                           \
       : kColorBlack)

static int update(void *userdata);
void dfsFloodFill(int x, int y);

static LCDBitmap *level = NULL;
int playerX = 30;
int playerY = 30;
int playerSpeed = 2;

void setup(void) {
  level = pd->graphics->newBitmap(LCD_COLUMNS, LCD_ROWS, kColorWhite);
  pd->graphics->pushContext(level);
  pd->graphics->drawRect(80, 80, 150, 50, kColorBlack);
  pd->graphics->drawRect(100, 100, 150, 50, kColorBlack);
  pd->graphics->drawRect(120, 120, 50, 50, kColorBlack);
  pd->graphics->drawRect(150, 100, 50, 50, kColorBlack);
  pd->graphics->drawRect(120, 120, 50, 50, kColorBlack);
  pd->graphics->popContext();
}

int eventHandler(PlaydateAPI *playdateAPI, PDSystemEvent event, uint32_t arg) {
  if (event == kEventInit) {
    pd = playdateAPI;
    pd->system->setUpdateCallback(update, pd);
    setup();
  }
  return 0;
}

void checkButtons(void) {
  PDButtons current;
  PDButtons pushed;
  pd->system->getButtonState(&current, &pushed, NULL);

  if (current & kButtonA || current & kButtonB) {
    dfsFloodFill(playerX, playerY);
  }

  int dx = 0;
  int dy = 0;
  if (current & kButtonRight) {
    dx = playerSpeed;
  }
  if (current & kButtonLeft) {
    dx = -playerSpeed;
  }
  if (current & kButtonUp) {
    dy = -playerSpeed;
  }
  if (current & kButtonDown) {
    dy = +playerSpeed;
  }
  playerX += dx;
  playerY += dy;
}

static int update(void *userdata) {
  checkButtons();

  pd->graphics->clear(kColorWhite);

  pd->graphics->drawBitmap(level, 0, 0, kBitmapUnflipped);

  pd->graphics->fillRect(playerX, playerY, 10, 10, kColorWhite);
  pd->graphics->drawRect(playerX, playerY, 10, 10, kColorBlack);

  pd->system->drawFPS(0, 0);

  return 1;
}

void dfsFloodFill(int x, int y) {
  uint8_t *bitmap_data = NULL;
  int bitmap_rowbytes = 0;
  pd->graphics->getBitmapData(level, NULL, NULL, &bitmap_rowbytes, NULL,
                              &bitmap_data);

  struct Stack *s = stack_create(LCD_COLUMNS * LCD_ROWS);
  stack_push((struct StackItem){.x = x, .y = y}, s);

  pd->graphics->pushContext(level);
  while (!stack_is_empty(s)) {
    struct StackItem top = stack_top(s);
    stack_pop(s);
    int x = top.x;
    int y = top.y;
    int p = samplePixel(bitmap_data, x, y, LCD_ROWSIZE);
    bool is_valid = x >= 0 && x < LCD_COLUMNS && y >= 0 && y < LCD_ROWS;
    if (is_valid && p == kColorWhite) {
      setPixel(bitmap_data, x, y, LCD_ROWSIZE);
      stack_push((struct StackItem){.x = x + 1, .y = y}, s);
      stack_push((struct StackItem){.x = x - 1, .y = y}, s);
      stack_push((struct StackItem){.x = x, .y = y + 1}, s);
      stack_push((struct StackItem){.x = x, .y = y - 1}, s);
    }
  }
  pd->graphics->popContext();

  stack_destroy(s);
}
