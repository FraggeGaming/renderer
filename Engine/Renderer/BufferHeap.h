#pragma once
#include <cstddef>
#include <vector>

#include <iostream>

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

    //Allocate a block of memory of given size
    MemoryBlock Allocate(size_t size){
        for (size_t i = 0; i < blocks.size(); i++)
        {
            if (blocks[i].free && blocks[i].size >= size){
                size_t remainingSize = blocks[i].size - size;
                blocks[i].free = false;
                blocks[i].size = size;
                allocatedSize += size;

                if (remainingSize > 0){
                    blocks.insert(blocks.begin() + i + 1, MemoryBlock{blocks[i].offset + size, remainingSize, true});
                }
                // Copy and return
                MemoryBlock result = blocks[i];
                return result;
            }
        }

        // Check if we can grow within totalSize limit
        if (allocatedSize + size > totalSize) {

            // Out of capacity
            static MemoryBlock outOfMemory{0, 0, false};
            //std::cout << "OUT OF HEAP MEMORY" << std::endl;
            return outOfMemory;
        }

        //TODO: because i switched to non resizable buffers, mby change this
        //Grow the heap
        blocks.push_back({allocatedSize, size, false});
        allocatedSize += size;
        return blocks.back();
    }

    void SetTotalSize(size_t newSize) {
        //Cant get smaller
        if (newSize < allocatedSize) {
            return;
        }
        totalSize = newSize;
    }

    void Free(size_t offset){
        for (size_t i = 0; i < blocks.size(); i++)
        {
            if (blocks[i].offset == offset){
                allocatedSize -= blocks[i].size;
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