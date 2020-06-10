#ifndef PROXYACTIVEOBJECT_H
#define PROXYACTIVEOBJECT_H

#include "scheduler.h"

namespace ActiveObject
{
    class ProxyActiveObject
    {
        using TypeTask = Scheduler::TypeDefferedTask;
    public:
        bool start();
        void wait();
        unsigned int push(AbstractTask *task);
        unsigned int push(AbstractTask *task, int msec, TypeTask type = TypeTask::DEFAULT);
        bool remove(unsigned int index);

        ProxyActiveObject();
        ProxyActiveObject(unsigned int _count_thread);
        virtual ~ProxyActiveObject();
    private:
        Scheduler _scheduler;
    };
}


#endif // PROXYACTIVEOBJECT_H
