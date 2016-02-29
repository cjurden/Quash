#ifndef CONTAINER_H
#define CONTAINER_H

#include <iostream>

class Container
{
    public:
      Container();
      long int getValue();
      void setValue(long int val);
      Container* getNext();
      void setNext(Container* next);
      Container* getPrev();
      void setPrev(Container* prev);

    private:
      long int m_value;
      Container* m_next;
      Container* m_prev;
};

#include "Container.hpp"
#endif
