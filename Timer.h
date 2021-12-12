#pragma once
#include <chrono>
#include <thread>
class Timer {
public:
    void ticker() {
        while (true) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            if (!paused)
                ms++;
            if (!running)
                break;
        }
        return;
    }
    void stop() {
        tickerthread = nullptr;
        running = false;
        ms = 0;
    }
    void reset() {
        tickerthread = nullptr;
        running = false;
        ms = 0;
        running = true;
        tickerthread = new std::thread([this] { ticker(); });
    }
    void start() {
        running = true;
        tickerthread = new std::thread([this] { ticker(); });
        tickerthread->detach();
    }
    void pause() {
        tickerthread = nullptr;
        running = false;
    }
    void resume() {
        start();
    }
    int elapsedMilliseconds() {
        return ms;
    }
private:
    bool running;
    bool paused;
    int ms = 0;
    std::thread* tickerthread;
};