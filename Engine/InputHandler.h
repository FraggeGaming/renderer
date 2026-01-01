#include <map>
#include <queue>
#include <functional>
#include <vector>
#include <iostream>


#include <unordered_map>
#include <GLFW/glfw3.h>
#pragma once


#define KEYS 349

enum class KEY_PRESS_TYPE { CONT, TAP };

struct InputEvent{
    KEY_PRESS_TYPE type;
    std::vector<std::function<void(float)>> Callback;
};

class InputHandler {
private:
    static void KeyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods);
public:

    InputHandler();

    ~InputHandler();

    void SetWindow(GLFWwindow* window);

    void HandleKey(int key, int scancode, int action, int mods);

    void Update(float dt);

    
    void BindKey(int key, KEY_PRESS_TYPE type, std::function<void(float)> cb);



private:
    bool m_keyPressed[KEYS] = { false }; //Current frames keydown
    bool m_keyDownPrev[KEYS] = { false }; //Stores previous frames keydown
    GLFWwindow* m_window;
    std::unordered_map<int, InputEvent> keyCallbacks;
};
