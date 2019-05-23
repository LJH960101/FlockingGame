// Fill out your copyright notice in the Description page of Project Settings.


#include "ReciveDataPool.h"

UReciveDataPool::UReciveDataPool() : _size(50)
{
	Resize();
}

void UReciveDataPool::ReturnObject(FReciveData* object)
{
	std::lock_guard<std::recursive_mutex> locker(_mt);
	_objects.Enqueue(object);
}

FReciveData* UReciveDataPool::GetObject()
{
	std::lock_guard<std::recursive_mutex> locker(_mt);
	FReciveData* retval = nullptr;
	while (true) {
		if (!_objects.Dequeue(retval)) {
			Resize();
		}
		else {
			if(retval) return retval;
		}
	}
}

void UReciveDataPool::Resize()
{
	std::lock_guard<std::recursive_mutex> locker(_mt);
	for (int i = 0; i < _size; ++i) {
		FReciveData* newData = new FReciveData();
		_objects.Enqueue(newData);
	}
}
