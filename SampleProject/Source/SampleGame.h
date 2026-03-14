//
// Created by Faith Kamaraju on 2026-01-14.
//

#pragma once

#include "LuminaEngine.h"

class MyGame : public LE::Game {

public:
    int Start() override;
    void Update() override;

};

inline LE::Game* CreateGame() {
    LE::Game* temp = new MyGame();
    temp->Start();
    return temp;
}