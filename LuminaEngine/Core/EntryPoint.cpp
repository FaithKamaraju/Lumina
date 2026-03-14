//
// Created by Faith Kamaraju on 2026-01-14.
//

#include "EntryPoint.h"
#include "Core/EngineStatics.h"
#include "Core/Logger.h"

int main(int argc, char** argv) {

    LE::Logger::init();

    LE::gEngine = new LE::Engine();

    auto result = LE::gEngine->InitializeSubSystems();
    if (result == LE_SUCCESS) {

        LE::gEngine->SetGameInstance(LE::gameCreate());
        LE::gEngine->StartEngineLoop();
        LE::gEngine->HandleShutdown();
    }

    delete LE::gEngine;

    return 0;
}