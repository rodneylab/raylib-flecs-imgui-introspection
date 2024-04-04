#include "game.h"

#include <raylib.h>

#include <queue>

void Game_Update(std::queue<int> *key_queue, bool *debug_menu)
{
    for (; !key_queue->empty(); key_queue->pop())
    {
        const int key{key_queue->front()};
        if (key == 0)
        {
            continue;
        }
        if (key == KEY_F9)
        {
            *(debug_menu) = !(*debug_menu);
        }
    }
}
