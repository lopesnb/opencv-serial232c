#include <iostream>
#include <thread>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <opencv2/opencv.hpp>
#include "common.h"
// スレッドセーフなキューの簡略版
template <typename T>
class SafeQueue {
private:
    std::queue<T> q;
    std::mutex m;
    std::condition_variable cv;
public:
    void push(T val) {
        std::lock_guard<std::mutex> lock(m); // 書き込み時にロック
        q.push(val);
        cv.notify_one(); // 待機中のスレッドに通知
    }

    bool try_pop(T& val) {
        std::lock_guard<std::mutex> lock(m);
        if (q.empty()) return false;
        val = q.front();
        q.pop();
        return true;
    }
};