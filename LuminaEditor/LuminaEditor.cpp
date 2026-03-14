//
// Created by Faith Kamaraju on 2026-03-02.
//

#include <iostream>
#include "Core/Logger.h"
#include "Core/EngineStatics.h"

int main() {

    LE::Logger::init();

    LE::gEngine = new LE::Engine();

    auto result = LE::gEngine->InitializeSubSystems();
    if (result == LE_SUCCESS) {

        // LE::gEngine->SetGameInstance(nullptr);
        LE::gEngine->StartEngineLoop();
        LE::gEngine->HandleShutdown();
    }

    delete LE::gEngine;

    return 0;
}