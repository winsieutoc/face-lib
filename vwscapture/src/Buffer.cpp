#include "vwscapture/Buffer.h"

// Buffer()
//  :
// m_running{true},
// m_timeout{DEFAULT_TIMEOUT},
// m_sleepTime{DEFAULT_SLEEP_TIME}
// {
//     m_running = true;
// }

// Buffer(int t1,int t2) :
// m_running{true},
// m_timeout{t1},
// m_sleepTime{t2}
// {}
// template<typename T>
// void Buffer<T>::pushBack(T item)
// {
//     mtx.lock();
//     m_queue.push(item);
//     mtx.unlock();
// }

// template<typename T>
// T Buffer<T>::getFirstItem()
// {
//     T newItem{};
//     mtx.lock();
//     if (!m_queue.empty()) {
//         newItem = m_queue.front();
//         m_queue.pop();
//     }
//     mtx.unlock();
//     return newItem;
// }

// template<typename T>
// T Buffer<T>::waitingNewItem()
// {
//     T newItem{};
//     int curTimeOut = m_timeout;
//     while(isRunning()&&(NULL == newItem)&&curTimeOut>0) {
//         mtx.lock();
//         if (!m_queue.empty()) {
//             newItem = m_queue.front();
//             m_queue.pop();

//             mtx.unlock();
//             return newItem;
//         }
//         else {
//             curTimeOut-=m_sleepTime;
//         }
//         mtx.unlock();
//         usleep(m_sleepTime);
//     }
//     if (0>=curTimeOut) {
//         printf("[ERROR] timeout while waitingNewItem \n");
//     }
//     return newItem;
// }
