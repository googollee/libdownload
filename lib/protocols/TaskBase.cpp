#include "TaskBase.h"

boost::signal<void (TaskBase *task)> TaskBase::downloadFinishSignal;
boost::signal<void (TaskBase *task)> TaskBase::uploadFinishSignal;
boost::signal<void (TaskBase *task, int error)> TaskBase::errorSignal;
boost::signal<void (TaskBase *task, const char* log)> TaskBase::logSignal;
