#include <iostream>
#include <deque>
#include <random>
#include <chrono>
#include "rlutil.h"

using namespace std;

const int LCD_BG = rlutil::GREY;
const int LCD_FG = rlutil::BLACK;

struct Pt {
    int x, y;
    bool operator==(const Pt &other) const { return x == other.x && y == other.y; }
};

static Pt randomFood(int w, int h, const deque<Pt> &snake) {
    static std::mt19937 rng((unsigned)std::chrono::high_resolution_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<int> dx(1, (w / 2) - 2);
    std::uniform_int_distribution<int> dy(2, h - 1);
    Pt f;
    do { f = { (dx(rng) * 2), dy(rng) }; } while ([&](){
        for(auto const& s : snake) if(s == f) return true;
        return false;
    }());
    return f;
}

// 強力閃爍 + 震動效果
void flashAndShake(int w, int h) {
    for (int i = 0; i < 3; i++) {
        // 反轉顏色：黑底灰字
        rlutil::setBackgroundColor(rlutil::BLACK);
        rlutil::setColor(rlutil::WHITE);
        rlutil::cls();
        rlutil::locate(w/2 - 5, h/2);
        cout << " SPEED UP!! ";
        cout.flush();
        rlutil::msleep(60); // 增加停留時間

        // 回到 LCD 顏色
        rlutil::setBackgroundColor(LCD_BG);
        rlutil::setColor(LCD_FG);
        rlutil::cls();
        rlutil::msleep(40);
    }
}

void drawScene(int w, int h) {
    rlutil::setBackgroundColor(LCD_BG);
    rlutil::cls(); 
    rlutil::setColor(LCD_FG);
    for (int x = 1; x <= w; x++) {
        rlutil::locate(x, 1); cout << "▀";
        rlutil::locate(x, h); cout << "▄";
    }
    for (int y = 1; y <= h; y++) {
        rlutil::locate(1, y); cout << "█";
        rlutil::locate(w, y); cout << "█";
    }
}

int main() {
    rlutil::hidecursor();
    const int W = 80; 
    const int H = 22; 
    const int BASE_TICK = 170; 
    const int MIN_TICK = 50;   

    auto startSnake = [&]() {
        drawScene(W, H);
        deque<Pt> snake = {{40, 10}, {38, 10}, {36, 10}};
        Pt food = randomFood(W, H, snake);
        Pt lastTail = {0, 0};
        int dx = 2, dy = 0;
        int score = 0;
        bool dead = false;
        int currentTick = BASE_TICK;

        while (true) {
            if (kbhit()) {
                int k = rlutil::getkey();
                if (k == rlutil::KEY_ESCAPE) exit(0);
                if (k == 'w' || k == 'W') { if(dy!=1){dx=0; dy=-1;} }
                else if (k == 's' || k == 'S') { if(dy!=-1){dx=0; dy=1;} }
                else if (k == 'a' || k == 'A') { if(dx!=2){dx=-2; dy=0;} }
                else if (k == 'd' || k == 'D') { if(dx!=-2){dx=2; dy=0;} }
                else if (k == 'r' || k == 'R') return;
            }

            if (!dead) {
                Pt next = {snake.front().x + dx, snake.front().y + dy};
                if (next.x <= 1 || next.x >= W-1 || next.y <= 1 || next.y >= H) dead = true;
                for (auto const& s : snake) if (s == next) dead = true;

                if (!dead) {
                    lastTail = snake.back();
                    snake.push_front(next);
                    if (next == food) {
                        score += 10;
                        food = randomFood(W, H, snake);
                        
                        // 加速判定：每多 3 節
                        if (snake.size() > 3 && (snake.size() - 3) % 3 == 0) {
                            currentTick = max(MIN_TICK, currentTick - 20); 
                            flashAndShake(W, H); // 執行強閃爍
                            drawScene(W, H);     // 重新畫牆壁
                        }
                    } else {
                        snake.pop_back();
                    }
                }
            }

            // 渲染蛇與食物
            rlutil::setColor(LCD_FG);
            if (lastTail.x != 0) { rlutil::locate(lastTail.x, lastTail.y); cout << "  "; }
            rlutil::locate(food.x, food.y); cout << "▓▓"; 
            for (int i = 0; i < (int)snake.size(); i++) {
                rlutil::locate(snake[i].x, snake[i].y);
                cout << "██";
            }

            // 下方資訊欄
            rlutil::locate(2, H + 1);
            rlutil::setBackgroundColor(rlutil::BLACK);
            rlutil::setColor(rlutil::WHITE);
            cout << " SCORE: " << score << " | LV: " << (BASE_TICK - currentTick)/20 + 1 << " | [R] RESTART ";
            rlutil::setBackgroundColor(LCD_BG);

            if (dead) {
                rlutil::setColor(rlutil::WHITE);
                rlutil::setBackgroundColor(rlutil::BLACK);
                rlutil::locate(W/2 - 7, H/2);
                cout << "  GAME OVER  ";
                rlutil::setBackgroundColor(LCD_BG);
            }

            cout.flush();
            rlutil::msleep(currentTick);
        }
    };

    while (true) startSnake();
    return 0;
}