#include "InputHandler.h"
#include "AppContext.h"
#pragma once


InputHandler::InputHandler()
{
    
}

InputHandler::~InputHandler()
{
}

void InputHandler::SetWindow(GLFWwindow *window)
{
    m_window = window;

    glfwSetKeyCallback(m_window, KeyCallBack);
}

//Binds a key with a press type with a callback
void InputHandler::BindKey(int key, KEY_PRESS_TYPE type, std::function<void(float)> cb) {
    auto &evt = keyCallbacks[key];
    evt.type = type;
    evt.Callback.push_back(std::move(cb));
}

//Runs once each frame, calling the callbacks of each key, change later to state dependent callbacks
void InputHandler::Update(float dt){
        for (int key = 0; key < KEYS; key++)
        {

            bool down = m_keyPressed[key];
            bool prev = m_keyDownPrev[key];

            bool pressed  =  down && !prev;
            bool held     =  down;
            bool released = !down &&  prev;
            
            if(down){

                //handle key binding
                auto it = keyCallbacks.find(key);
                if(it != keyCallbacks.end()){

                    
                    if(it->second.type == KEY_PRESS_TYPE::CONT){
                        if(held){
                            for (auto& cb : it->second.Callback){
                                cb(dt);
                            }
                        }
                    }

                    if(it->second.type == KEY_PRESS_TYPE::TAP){
                        if(pressed){
                            for (auto& cb : it->second.Callback){
                                cb(dt);
                            }
                        }
                    }
                }
            }


            
            m_keyDownPrev[key] = down;
        }
    }

void InputHandler::KeyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods){
    AppContext* ctx = static_cast<AppContext*>(glfwGetWindowUserPointer(window));

    if(ctx->input){
        ctx->input->HandleKey(key, scancode, action, mods);
    }
}

//Handles key input each frame
void InputHandler::HandleKey(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_UNKNOWN) return;
    if (key < 0 || key >= KEYS)   return;

    if(action == GLFW_PRESS){
        m_keyPressed[key] = true;
    } else if (action == GLFW_RELEASE){
        m_keyPressed[key] = false;
    }
}
