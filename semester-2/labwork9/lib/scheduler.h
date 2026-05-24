#pragma once
#include <vector>
#include <memory>
#include <type_traits>

class BaseTask {
public:
    virtual ~BaseTask() = default;
    virtual void process() {}
    void* res_ptr = nullptr;
};

template<typename T>
struct FutureResult {
    BaseTask* task;
};

template<typename F, typename Arg1, typename Arg2>
class Task : public BaseTask {
public:
    F func;
    Arg1 arg1;
    Arg2 arg2;

    Task(F f, Arg1 a1, Arg2 a2)
        : func{ f }, arg1{ a1 }, arg2{ a2 } {}

    void process() override {
        if (!res_ptr) {
            if constexpr (std::is_member_function_pointer<F>::value) {
                auto tmp = (get(arg1).*func)(get(arg2));
                res_ptr = new decltype(tmp)(tmp);
            }
            else {
                auto result = func(get(arg1), get(arg2));
                res_ptr = new decltype(result)(result);
            }
        }
    }

private:
template<typename U>
    U& get(FutureResult<U>& future) {
        future.task->process();
        return *static_cast<U*>(future.task->res_ptr);
    }
    template<typename U>
    U& get(U& value) {
        return value;
    }
};

class TTaskScheduler {
public:
    template<typename F, typename Arg1, typename Arg2>
    BaseTask* add(F func, Arg1 arg1, Arg2 arg2) {
        auto t = std::make_unique<Task<F, Arg1, Arg2>>(func, arg1, arg2);
        BaseTask* ptr = t.get();
        tasks.emplace_back(std::move(t));
        return ptr;
    }

    template<typename T>
    FutureResult<T> getFutureResult(BaseTask* task) {
        return FutureResult<T>{ task };
    }

    template<typename T>
    T getResult(BaseTask* task) {
        task->process();
        return *static_cast<T*>(task->res_ptr);
    }

    void executeAll() {
        for (auto& t : tasks) {
            t->process();
        }
        tasks.clear();
    }

private:
    std::vector<std::unique_ptr<BaseTask>> tasks;
};
