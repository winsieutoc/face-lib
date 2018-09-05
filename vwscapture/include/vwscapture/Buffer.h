#ifndef BUFFER_H
#define BUFFER_H
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <queue>
#include <mutex>

#define DEFAULT_TIMEOUT         2000000 // us
#define DEFAULT_SLEEP_TIME      10000   // us

using namespace std;

template<typename T>
class Buffer
{
public:
    Buffer(std::queue<T> &m_queue_) :
    m_queue(m_queue_)
    {
        m_running = true;
        m_timeout = DEFAULT_TIMEOUT;
        m_sleepTime = DEFAULT_SLEEP_TIME;
    }
    void pushBack(T item);
    T pop();
    // getFirstItem. If queue is empty, waiting until new item is appended or timeout
    T waitingNewItem();

private:
    std::queue<T> &m_queue;
    std::mutex mtx;
    int m_timeout;
    int m_sleepTime;

    bool m_running;
    virtual bool isRunning()     {return m_running;}
};

template<typename T>
void Buffer<T>::pushBack(T item) {
    mtx.lock();
    m_queue.push(item);
    mtx.unlock();
}

template<typename T>
T Buffer<T>::pop()
{
    T newItem{};
    mtx.lock();
    if (!m_queue.empty()) {
        newItem = m_queue.front();
        m_queue.pop();
    }
    mtx.unlock();
    return newItem;
}

// getFirstItem. If queue is empty, waiting until new item is appended or timeout
template<typename T>
T Buffer<T>::waitingNewItem()
{
    T newItem{};
    int curTimeOut = m_timeout;
    while(isRunning()&&curTimeOut>0) {
        mtx.lock();
        if (!m_queue.empty()) {
            newItem = m_queue.front();
            m_queue.pop();

            mtx.unlock();
            return newItem; // break while()
        }
        else {
            curTimeOut-=m_sleepTime;
        }
        mtx.unlock();
        usleep(m_sleepTime);
    }
    if (0>=curTimeOut) {
        printf("[ERROR] timeout while waitingNewItem \n");
    }
    return newItem;
}

#endif // BUFFER_H
