#include <memory>
#include <queue>
#include <condition_variable>
#include <mutex>

template<typename T>
class ThreadSafeQueue
{
private:
    mutable std::mutex mutex;
    std::queue<std::shared_ptr<T>> queue;
    std::condition_variable condition;

public:
    ThreadSafeQueue()
    {
    }

    ThreadSafeQueue(const ThreadSafeQueue& other)
    {
        std::scoped_lock lk(mutex);
        queue = other.queue;
    }

    ThreadSafeQueue& operator= (const ThreadSafeQueue&) = delete;

    void push(T value)
    {
        auto ptr = std::make_shared<T>(std::move(value));
        std::scoped_lock lk(mutex);
        queue.push(ptr);
        condition.notify_one();
    }

    bool try_pop(T& value)
    {
        std::scoped_lock lk(mutex);
        if (queue.empty())
            return false;

        value = std::move(*queue.front());
        queue.pop();
        return true;
    }

    std::shared_ptr<T> try_pop()
    {
        std::scoped_lock lk(mutex);
        if (queue.empty())
            return nullptr;
        auto ptr = queue.front();
        queue.pop();
        return ptr;
    }

    void wait_and_pop(T& value)
    {
        std::unique_lock lk(mutex);
        condition.wait(lk, [&]{return !queue.empty();});
        value = std::move(*queue.front());
        queue.pop();
    }

    std::shared_ptr<T> wait_and_pop()
    {
        std::unique_lock lk(mutex);
        condition.wait(lk, [&]{return !queue.empty();});
        auto ptr = queue.front();
        queue.pop();
        return ptr;
    }

    bool empty() const
    {
        std::scoped_lock lk(mutex);
        return queue.empty();
    }
};