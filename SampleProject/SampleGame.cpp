//
// Created by Faith Kamaraju on 2026-01-14.
//

#include "SampleGame.h"
#include "Core/Events/EventManager.h"
#include "Core/Events/WindowEvent.h"
#include "Core/Logger.h"


int MyGame::Start() {

    START();

    LE_INFO("Game has been initialized!");

    // LE::Events::Subscribe<LE::WindowResizeEvent>([](const LE::WindowResizeEvent& e) {
    //     LE_INFO("Window Resized to {0} x {1}", e.width, e.height);
    // });

    LE::Events::Subscribe<LE::WindowLostFocusEvent>([](const LE::WindowLostFocusEvent& e) {
        LE_INFO("Window Lost focus");
    });
    LE::Events::Subscribe<LE::WindowFocusEvent>([](const LE::WindowFocusEvent& e) {
        LE_INFO("Window Gained focus");
    });

    return 1;
}

void MyGame::Update() {

    UPDATE();


}

LE::Game* LE::CreateGame() {
    Game* temp = new MyGame();
    temp->Start();
    return temp;
}