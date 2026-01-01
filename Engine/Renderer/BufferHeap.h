#pragma once
#include <cstddef>
#include <vector>

struct MemoryBlock {

    size_t offset;
    size_t size;
    bool free;
};

struct Heap{
    size_t totalSize;
    std::vector<MemoryBlock> blocks;
    size_t allocatedSize = 0;

    Heap(size_t size){
        totalSize = size;
        blocks.push_back(MemoryBlock{0, size, true});
    }

    //Simple first fit allocation
    MemoryBlock& Allocate(size_t size){
        for (size_t i = 0; i < blocks.size(); i++)
        {
            if (blocks[i].free && blocks[i].size >= size){
                size_t remainingSize = blocks[i].size - size;
                blocks[i].free = false;
                blocks[i].size = size;

                if (remainingSize > 0){
                    blocks.insert(blocks.begin() + i + 1, MemoryBlock{blocks[i].offset + size, remainingSize, true});
                }

                return blocks[i];
            }
        }

        //If no suitable block found, create block
        blocks.push_back({allocatedSize, size, false});
        allocatedSize += size;
        return blocks.back(); //Allocation failed
    }

    void Free(size_t offset){
        for (size_t i = 0; i < blocks.size(); i++)
        {
            if (blocks[i].offset == offset){
                blocks[i].free = true;
                //Coalesce adjacent free blocks
                if (i > 0 && blocks[i - 1].free){
                    blocks[i - 1].size += blocks[i].size;
                    blocks.erase(blocks.begin() + i);
                    i--;
                }
                if (i < blocks.size() - 1 && blocks[i + 1].free){
                    blocks[i].size += blocks[i + 1].size;
                    blocks.erase(blocks.begin() + i + 1);
                }
                return;
            }
        }
    }
};