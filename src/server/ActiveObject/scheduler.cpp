#include "scheduler.h"
#include <iostream>

namespace AO = ActiveObject;

AO::AbstractTask* AO::Scheduler::pop_task()
{
    refresh_queue();
    std::lock_guard<std::mutex> guard(_access_to_queue);
    if(_queue_tasks.empty())
    {
        return nullptr;
    }
    auto task = std::get<1>(_queue_tasks.front());
    _queue_tasks.pop_front();
    return task;
}

unsigned int AO::Scheduler::push_task(AbstractTask *task)
{
    std::lock_guard<std::mutex> guard(_access_to_queue);
    _queue_tasks.push_back(std::make_tuple(_index,task));
    return _index++;
}

unsigned int AO::Scheduler::push_task(AbstractTask *task, int msec, TypeTask type)
{
    std::lock_guard<std::mutex> guard(_access_to_deffered_queue);
    _queue_deffered_tasks.push_back(std::make_tuple(_index,task,type,msec,steady_clk::now()));
    return _index++;
}

void AO::Scheduler::refresh_queue()
{
    std::lock_guard<std::mutex> guard(_access_to_deffered_queue);
    auto it = _queue_deffered_tasks.begin();
    while(it != _queue_deffered_tasks.end())
    {
        _access_to_queue.lock();
        bool is_empty = _queue_tasks.empty();
        _access_to_queue.unlock();

        auto delete_item = it;
        it++;

        if(is_empty)
        {
            if(std::get<2>(*delete_item) == TypeTask::EXPRESS)
            {
                import_task(delete_item);
                continue;
            }
        }
        if(std::chrono::duration_cast<ms>(steady_clk::now() - std::get<4>(*delete_item)).count() >= std::get<3>(*delete_item))
        {
            import_task(delete_item);
            continue;
        }
    }
}

void AO::Scheduler::import_task(decltype (_queue_deffered_tasks)::iterator it)
{
    _access_to_queue.lock();
    _queue_tasks.push_back(std::make_tuple(std::get<0>(*it),std::get<1>(*it)));
    _access_to_queue.unlock();
    _queue_deffered_tasks.erase(it);
}

bool AO::Scheduler::remove_task(unsigned int index)
{
    std::lock_guard<std::mutex> first_guard(_access_to_queue);
    std::lock_guard<std::mutex> second_guard(_access_to_deffered_queue);
    auto queue_it = _queue_tasks.begin();
    auto deffered_queue_it = _queue_deffered_tasks.begin();
    while(queue_it != _queue_tasks.end() || deffered_queue_it != _queue_deffered_tasks.end())
    {
        if(std::get<0>(*queue_it) == index)
        {
            delete std::get<1>(*queue_it);
            _queue_tasks.erase(queue_it);
            return true;
        }
        if(std::get<0>(*deffered_queue_it) == index)
        {
            delete std::get<1>(*deffered_queue_it);
            _queue_deffered_tasks.erase(deffered_queue_it);
            return true;
        }
        if(queue_it != _queue_tasks.end())
        {
            queue_it++;
        }
        if(deffered_queue_it != _queue_deffered_tasks.end())
        {
            deffered_queue_it++;
        }
    }
    return false;
}

void AO::Scheduler::wait_all()
{
    _is_run = false;
    for (unsigned int i = 0; i < _count_thread; i++)
    {
        if(_threads[i].joinable())
        {
            _threads[i].join();
        }
    }
}

bool AO::Scheduler::run_all()
{
    if(_is_run)
    {
        return false;
    }

    auto fun = [&]
    {
        while (_is_run)
        {
            auto task = this->pop_task();
            if(task)
            {
                task->run_process();
            }
        }
    };

    _is_run = true;
    for (unsigned int i = 0; i < _count_thread; i++)
    {
        _threads[i] = std::thread(fun);
    }
    return true;
}

AO::Scheduler::Scheduler(unsigned int count_thread):
    _threads(count_thread),
    _index(0),
    _count_thread(count_thread),
    _is_run(false)
{
}

AO::Scheduler::~Scheduler()
{
    wait_all();
    auto first_it = _queue_tasks.begin();
    while(first_it != _queue_tasks.end())
    {
        delete std::get<1>(*first_it);
        first_it++;
    }
    auto second_it = _queue_deffered_tasks.begin();
    while(second_it != _queue_deffered_tasks.end())
    {
        delete std::get<1>(*second_it);
        second_it++;
    }

}
