// Fifo.h: interface for the Fifo class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(_FIFO_H_)
#define _FIFO_H_
#include <afxmt.h>
#include <list>
using namespace std;

template<class T>
class Fifo
{
 public:
  bool Put(const T& data);
  bool Get(T& data);
 public:
  Fifo();

  virtual ~Fifo();
 private:
  CCriticalSection m_critical;    
  list<T> m_data;
};


template<class T>
Fifo<T>::Fifo()
{
 
}

template<class T>
Fifo<T>::~Fifo()
{
 
}


template<class T>
bool
Fifo<T>::Get(T& data)
{
  m_critical.Lock();
  if (m_data.empty())
  {
	  m_critical.Unlock();
	  return false;
  }
  data = m_data.front();
  m_data.pop_front();
  m_critical.Unlock();
  return true;
}


template<class T>
bool
Fifo<T>::Put(const T& data)
{
  m_critical.Lock();
  m_data.push_back(data);
  m_critical.Unlock();
  return true;
}

#endif 
