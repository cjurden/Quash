#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <iostream>
#include "Container.h"

class LinkedList
{
  public:
      LinkedList();
      ~LinkedList();
      void add(long int x);
      bool isEmpty();
      Container* find(long int x);
      int size();
      Container* m_back;
      Container* m_front;
  private:

      int m_size;
};

#include "LinkedList.hpp"
#endif
