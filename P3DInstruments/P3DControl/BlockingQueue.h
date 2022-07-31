#pragma once
#include <queue>
#include "../P3DCommon/CriticalSection.h"

template <class T>  class BlockingQueue
{
	HANDLE semaphore;
	CriticalSection cs;
	std::queue<T> queue;

public: 
	BlockingQueue() : semaphore(0) {
		semaphore = ::CreateSemaphore(0, 0, LONG_MAX, 0);
	};
	~BlockingQueue();

	void enqueue(const T& value);
	T dequeue();
	bool empty() ;
};

template<class T>
inline BlockingQueue<T>::~BlockingQueue()
{
	::CloseHandle(semaphore);
}

template<class T>
inline void BlockingQueue<T>::enqueue( const T& value)
{
	{
		CriticalSection::Lock lock(cs);
		queue.push(value);
	} // end of lock scope
	::ReleaseSemaphore(semaphore, 1, NULL);
}

template<class T>
inline T BlockingQueue<T>::dequeue()
{
	::WaitForSingleObject(semaphore, INFINITE);
	CriticalSection::Lock lock(cs);
	T value(queue.front());
	queue.pop();
	return value;
}

template<class T>
inline bool BlockingQueue<T>::empty() 
{
	CriticalSection::Lock lock(cs);
	bool isEmpty = queue.empty();
	return isEmpty;
}
