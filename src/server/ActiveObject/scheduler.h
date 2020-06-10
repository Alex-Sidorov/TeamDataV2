#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "abstracttask.h"

#include <list>
#include <vector>
#include <tuple>
#include <thread>
#include <atomic>
#include <mutex>
#include <chrono>

namespace ActiveObject
{
    class Scheduler
    {
    public:

        typedef enum class TypeDefferedTask {DEFAULT = 0, EXPRESS}TypeTask;

        AbstractTask *pop_task();
        unsigned int push_task(AbstractTask *task);
        unsigned int push_task(AbstractTask *task, int msec, TypeTask type = TypeTask::DEFAULT);
        bool remove_task(unsigned int index);
        void wait_all();
        bool run_all();

        Scheduler(unsigned int count_thread = DEFAULT_THREAD);
        virtual ~Scheduler();

    private:
        static constexpr unsigned int DEFAULT_THREAD = 1;

        using steady_clk = std::chrono::steady_clock;
        using tm_point = std::chrono::time_point<steady_clk>;
        using ms = std::chrono::milliseconds;

        using tuple_for_deffered_task =
        std::tuple<unsigned int,AbstractTask*,
        TypeTask,
        int,
        tm_point>;

        using tuple_for_simple_task = std::tuple<unsigned int,AbstractTask*>;

        std::list<tuple_for_simple_task> _queue_tasks;
        std::list<tuple_for_deffered_task> _queue_deffered_tasks;
        std::vector<std::thread> _threads;
        std::mutex _access_to_queue;
        std::mutex _access_to_deffered_queue;

        unsigned int _index;
        unsigned int _count_thread;
        std::atomic_bool _is_run;

        void refresh_queue();
        void import_task(std::list<tuple_for_deffered_task>::iterator it);

    };
}

#endif // SCHEDULER_H
