#pragma once
#include <chrono>
#include <thread>
/*class Timer
{
public:
    std::chrono::time_point<std::chrono::system_clock> m_StartTime;
    std::chrono::time_point<std::chrono::system_clock> m_EndTime;
    bool                                               m_bRunning = false;
    double elapsed;
    void start()
    {
        m_StartTime = std::chrono::system_clock::now();
        m_bRunning = true;
    }

    void stop()
    {
        m_EndTime = std::chrono::system_clock::now();
        m_bRunning = false;
    }
    double elapsedMilliseconds()
    {
        std::chrono::time_point<std::chrono::system_clock> endTime;

        if (m_bRunning)
        {
            endTime = std::chrono::system_clock::now();
        }
        else
        {
            endTime = m_EndTime;
        }

        return std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_StartTime).count();
    }
    void pause() {
        elapsed = elapsedMilliseconds(); // gotta fix resume
    }
    void resume() {
        m_StartTime += std::chrono::milliseconds((int)elapsedMilliseconds()) - std::chrono::milliseconds((int)elapsed); // this is stupid, this works perfect in gcc but not msvc :/
    }
    double elapsedSeconds()
    {
        return elapsedMilliseconds() / 1000.0;
    }
};

long fibonacci(unsigned n)
{
    if (n < 2) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}*/
typedef void(__cdecl* tickertype)();
class Timer {
public:
    void ticker() {
        while (true) {
            Sleep(1);
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