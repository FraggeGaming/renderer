#pragma once

struct Component{
    int parent_idx = -1;

    Component(){

    }

    virtual ~Component() = default; //typeid now know that its polymorphic, dont remove this

    Component(int parent) : parent_idx(parent){
        
    }

    void SetParent(int parent) {
        parent_idx = parent;
    }
};