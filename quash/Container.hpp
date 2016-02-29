
Container::Container()
{
	m_next = nullptr;
	m_prev = nullptr;
	m_value = 0;
}



long int Container::getValue()
{
	return(m_value);
}


void Container::setValue(long int value)
{
	m_value = value;
}


Container* Container::getNext()
{
	return(m_next);
}


void Container::setNext(Container* next)
{
	m_next = next;
}


Container* Container::getPrev()
{
	return(m_prev);
}



void Container::setPrev(Container* prev)
{
	m_prev = prev;
}
