#include "proxyactiveobject.h"

namespace ao = ActiveObject;

ao::ProxyActiveObject::ProxyActiveObject(){}

ao::ProxyActiveObject::ProxyActiveObject(unsigned int _count_thread):
    _scheduler(_count_thread)
{}

ao::ProxyActiveObject::~ProxyActiveObject(){}

bool ao::ProxyActiveObject::start()
{
    return _scheduler.run_all();
}
void ao::ProxyActiveObject::wait()
{
    _scheduler.wait_all();
}

unsigned int ao::ProxyActiveObject::push(AbstractTask *task)
{
    return _scheduler.push_task(task);
}

unsigned int ao::ProxyActiveObject::push(AbstractTask *task, int msec, TypeTask type)
{
    return _scheduler.push_task(task,msec,type);
}

bool ao::ProxyActiveObject::remove(unsigned int index)
{
    return _scheduler.remove_task(index);
}
