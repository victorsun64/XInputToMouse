#include <windows.h>
#include <Xinput.h>
#include <iostream>
#include <thread>
#include <winuser.h>
#include "config.h"

#pragma comment(lib, "Xinput.lib")

bool running = true;
bool mouseControlOn = true;
//position of the cursor
POINT point;

void enableANSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode;
    GetConsoleMode(hOut, &mode);
    SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void clearScreen() {
    std::cout << "\033[2F\033[J";
}

void toggleMouseControl() {
    mouseControlOn = !mouseControlOn;
}

void stopProgram() {
    running = false;
}

void moveCursor(XINPUT_STATE &state) {
    GetCursorPos(&point);
    point.x += (state.Gamepad.sThumbLX * config::sensitivity) / SHRT_MAX / 2;
    point.y -= (state.Gamepad.sThumbLY * config::sensitivity) / SHRT_MAX / 2;
    SetCursorPos(point.x, point.y);
}

void leftClick() {
    mouse_event(MOUSEEVENTF_LEFTDOWN, point.x, point.y, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 30 + 20));
    mouse_event(MOUSEEVENTF_LEFTUP, point.x, point.y, 0, 0);
}

void middleMouseClick() {
    mouse_event(MOUSEEVENTF_MIDDLEDOWN, point.x, point.y, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 30 + 20));
    mouse_event(MOUSEEVENTF_MIDDLEUP, point.x, point.y, 0, 0);
}

void rightClick() {
    mouse_event(MOUSEEVENTF_RIGHTDOWN, point.x, point.y, 0, 0);
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 30 + 20));
    mouse_event(MOUSEEVENTF_RIGHTUP, point.x, point.y, 0, 0);
}

void mouseWheelScroll(XINPUT_STATE &state) {
    mouse_event(MOUSEEVENTF_WHEEL, point.x, point.y, (state.Gamepad.sThumbRY * WHEEL_DELTA) / SHRT_MAX, 0);
}

void processInput() {
    XINPUT_STATE state;
    DWORD result = XInputGetState(0, &state);
    WORD buttons = state.Gamepad.wButtons;

    //latches so each function is only called once per press
    static bool toggleheld = false;
    static bool leftheld = false;
    static bool middleheld = false;
    static bool rightheld = false;

    if (result == ERROR_SUCCESS) {
        //on/off switch for mouse control
        if (config::toggle == (buttons & config::toggle)) {
            if (!toggleheld) {
                toggleMouseControl();
                toggleheld = true;
            }
        } else {
            toggleheld = false;
        }
        //close program if entry is equal to the config
        if (buttons == config::exit) {
            stopProgram();
        }
        //move cursor with joysticks
        if (mouseControlOn && (state.Gamepad.sThumbLX != 0 || state.Gamepad.sThumbLY != 0)) {
            moveCursor(state);
        }
        //left click
        if (mouseControlOn && buttons & config::leftClick) {
            if (!leftheld) {
                leftClick();
                leftheld = true;
            }
        } else {
            leftheld = false;
        }
        //middle mouse click
        if (mouseControlOn && buttons & config::middleClick) {
            if (!middleheld) {
                middleMouseClick();
                middleheld = true;
            }
        } else {
            middleheld = false;
        }
        //right click
        if (mouseControlOn && buttons & config::rightClick) {
            if (!rightheld) {
                rightClick();
                rightheld = true;
            }
        } else {
            rightheld = false;
        }
        //move mousewheel
        if (mouseControlOn && state.Gamepad.sThumbRY != 0) {
            mouseWheelScroll(state);
        }

        clearScreen();
        std::cout << "Running" << std::endl;
        std::cout << "Mouse control is " << (mouseControlOn ? "ON" : "OFF") << std::endl;
    } else {

        clearScreen();
        std::cout << "Disconnected";
    }
}

int main() {
    enableANSI();
    while (running) {
        processInput();
        //~60hz poll rate
        std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }
    return 0;
}
