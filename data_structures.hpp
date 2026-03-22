#include <stdio.h>
#include <numbers>
#include <cstdint>


class Node
{
    public:
        Node* next;
        Node* prev;
        std::uint16_t data;

    Node(uintptr_t data)
    {
        this->data = data;
    }
    Node()
    {
        this->data = 0;
        this->prev = NULL;
        this->next = NULL;
    }
    public:


    
};

