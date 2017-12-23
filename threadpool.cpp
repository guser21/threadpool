#include <thread>
#include <vector>
#include <condition_variable>
#include <future>
#include <list>
#include <queue>
#include <iostream>

//
// Created by guser on 12/21/17.
//

template<typename R>
class threadpool {
public:
    explicit threadpool(int n_threads) : n_threads(n_threads) {
        for (int i = 0; i < n_threads; ++i) {
            threads.push_back(std::thread([&]() -> void {
                while (true) {
                    std::function<void()> runnable;
                    {
                        std::unique_lock<std::mutex> lock(vector_mutex);
                        cond.wait(lock, [&] { return !(still_running && tasks.empty()); });

                        if (tasks.empty() && !still_running) return;
                        runnable = std::move(tasks.front());
                        tasks.pop_front();
                    }
                    runnable();
                }
            }));
        }
    }

    std::future<R> push(std::function<R()>&& fun) {
        auto p_task = std::make_shared<std::packaged_task<R()>>(std::move(fun));
        if (!n_threads) {
            (*p_task)();
            return p_task->get_future();
        } else {
            {
                std::unique_lock<std::mutex> lock(vector_mutex);
                //TODO perfect forward fun

                auto job = [p_task]() -> void {//really weird
                    (*p_task)();
                };
                tasks.push_back(std::move(job));
            }
            cond.notify_one();
            return p_task->get_future();
        }
    }

    ~threadpool() {
        still_running = false;
        cond.notify_all();
        for (int i = 0; i < threads.size(); ++i) {
            threads[i].join();
        }
    }
    int thread_count(){
        return n_threads;
    }
private:
    int n_threads = 0;//additional threads
    std::atomic<bool> still_running{true};
    std::vector<std::thread> threads;
    std::list<std::function<void()>> tasks;

    std::mutex vector_mutex;
    std::condition_variable cond;
};

//std::function<int()> funGen() {
//    return ([]() -> int {
//        int a=(rand()%10);
//        std::this_thread::sleep_for(std::chrono::operator""ms(a));
//        return 3;
//    });
//}
//
//int main() {
//    threadpool<int> threadpool(200);
//    std::vector<std::future<int>> futures;
//    for (int i = 0; i <10000 ; ++i) {
//        futures.push_back(threadpool.push(funGen()));
//    }
//    int res=0;
//    for (int j = 0; j < futures.size(); ++j) {
//        res+=futures[j].get();
//    }
//
//    std::cout<<res<<std::endl;
//    return 0;
//}