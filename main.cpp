#include <iostream>
#include <RF24/RF24.h>

#include <chrono>
#include <thread>
#include <algorithm>

#include <SDL2/SDL.h>

using namespace std::chrono_literals;

struct __attribute__((__packed__)) ControlData {
    uint8_t mainMotorPower = 0;
    int8_t rudderPosition = 0;
    int8_t elevatorPosition = 0;
    int8_t leftAileronPosition = 0;
    int8_t rightAileronPosition = 0;
};

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
int main() {
    std::cout << "Controller for Spitfire started." << std::endl;
    RF24 radio(22, 0);

    const uint8_t pipes[][6] = {"1Node", "2Node"};

    radio.begin();
    radio.setRetries(15, 15);
    radio.printDetails();

    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1, pipes[0]);

    if (SDL_Init(SDL_INIT_JOYSTICK) != 0) {
        throw std::runtime_error("Failed to init SDL2.");
    }

    if (SDL_NumJoysticks() < 1) {
        throw std::runtime_error("No gamepad connected.");
    }

    const auto joystickHandle = SDL_JoystickOpen(0);

    if (!joystickHandle) {
        throw std::runtime_error("Failed to open gamepad.");
    }

    float leftX = 0, leftY = 0, rightX = 0, rightY = 0, leftTrigger = 0, rightTrigger = 0;

    while (true) {
        SDL_Event event;

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_JOYAXISMOTION) {
                switch (event.caxis.axis) {
                    case 0:
                        leftX = event.caxis.value / 32767.0f;
                        break;
                    case 1:
                        leftY = event.caxis.value / -32767.0f;
                        break;
                    case 2:
                        leftTrigger = (event.caxis.value / 32767.0f + 1) / 2.0f;
                        break;
                    case 3:
                        rightX = event.caxis.value / 32767.0f;
                        break;
                    case 4:
                        rightY = event.caxis.value / -32767.0f;
                        break;
                    case 5:
                        rightTrigger = (event.caxis.value / 32767.0f + 1) / 2.0f;
                        break;
                }
            }
        }

        ControlData controlData = {
                .mainMotorPower = static_cast<uint8_t>(std::clamp(std::abs(leftY) * 255, 0.0f, 255.0f)),
                .rudderPosition = static_cast<int8_t>(std::clamp((leftTrigger - rightTrigger) * 80, -127.0f, 127.0f)),
                .elevatorPosition = static_cast<int8_t>(std::clamp(rightY * 80, -127.0f, 127.0f)),
                .leftAileronPosition = static_cast<int8_t>(std::clamp(rightX * 80, -127.0f, 127.0f)),
                .rightAileronPosition = static_cast<int8_t>(std::clamp(-rightX * 80, -127.0f, 127.0f)),
        };

        bool ok = radio.write(&controlData, sizeof(ControlData));

        if (!ok) {
            std::cout << "Failed to send control data to the plane." << std::endl;
        }

        std::this_thread::sleep_for(10ms);
    }

    SDL_JoystickClose(joystickHandle);

    return 0;
}
#pragma clang diagnostic pop
