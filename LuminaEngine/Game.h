//
// Created by Faith Kamaraju on 2026-01-14.
//

#pragma once

namespace LE {

    class Game {

    public:
        virtual ~Game();

        virtual int Start();
        virtual void Update();

    };

#define START() LE::Game::Start()
#define UPDATE() LE::Game::Update()

    Game* CreateGame();

}

