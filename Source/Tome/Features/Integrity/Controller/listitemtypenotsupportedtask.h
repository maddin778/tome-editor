#ifndef LISTITEMTYPENOTSUPPORTEDTASK_H
#define LISTITEMTYPENOTSUPPORTEDTASK_H

#include "../../Tasks/Controller/task.h"

namespace Tome
{
    class TaskContext;

    class ListItemTypeNotSupportedTask : public Task
    {
        public:
            ListItemTypeNotSupportedTask();

            const MessageList execute(const TaskContext& context) const;

            static const QString MessageCode;
    };
}

#endif // LISTITEMTYPENOTSUPPORTEDTASK_H