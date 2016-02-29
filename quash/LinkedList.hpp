LinkedList::LinkedList()
{
	m_front = nullptr;
	m_back = nullptr;
	m_size = 0;
}

LinkedList::~LinkedList()
{
	Container* temp = m_front;
	Container* next = nullptr;

	while(temp != nullptr)
	{
		next = temp->getNext();
		delete temp;
		m_size--;
		temp = next;
	}
  m_back = nullptr;
}


int LinkedList::size()
{
	return(m_size);
}



bool LinkedList::isEmpty()
{
	return(m_size == 0);
}


void LinkedList::add(long int value)
{
	Container* temp = new Container();
	temp->setValue(value);
	if(m_size==0)
	{
		m_front = temp;
		m_back	= m_front;
	}
	if(m_size>0)
	{
		m_back->setNext(temp);
		temp->setPrev(m_back);
		m_back=temp;
	}
	m_size++;
}



Container* LinkedList::find(long int x)
{
  Container* temp = m_front;
  while(temp!= nullptr)
  {
		if(temp->getValue()!=x){
      temp = temp->getNext();
		}
		else if(temp->getValue() == x){
			return temp;
		}
  }

  return(temp);
}

/*
bool LinkedList::operator<< (const Container& rhs) const
{
	return(rhs.getValue());
}
*/
