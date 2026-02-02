//
// Created by Faith Kamaraju on 2026-01-23.
//
#include "TaskScheduler.h"

#pragma once

namespace LE {

    class TasksManager {
    public:

        TasksManager();
        ~TasksManager();




    private:

        enki::TaskScheduler g_TS{};

    };

}
