#pragma once
#include <type_traits>
#include <initializer_list>
#include <new>
#include <cassert>
#include <stddef.h>
#include "FreeListNode.h"
#pragma warning(disable : 4101)
#pragma warning(disable : 4984)
#define PROFILE
#include "myProfiler.h"

template <typename T>
class FreeList
{
public:
	FreeList(bool constr_dest_on_alloc_free = true, int capacity = 0)
		:constr_dest_on_alloc_free_{ constr_dest_on_alloc_free }, head_{}
#ifdef _DEBUG
	,capacity_{ 0 }, size_{ 0 }
#endif
	{
		top_ = &head_;
		for (int i = 0; i < capacity; ++i)
		{
			Push(GetNodeFromHeap());
		}
	}
#ifdef _DEBUG
	// ��������Ʈ�� ��尡 �ִ뿳������ ����, �ᱹ �Ҹ��� ȣ��������� capacity_ != size_�̸� �޸� ������ �߻��Ѱ�
	int capacity_;
	// ���� �������մ� ��������Ʈ�� ����
	int size_;
#ifdef DETECT_MEMORY_LEAK
	// �Ҹ��ڿ����� ȣ��Ǿ�� �ϴ� �Լ�
	__forceinline bool isMemoryLeak()
	{
		return capacity_ != size_;
	}
#endif
#endif

	~FreeList()
	{
		if (IsEmpty())
			return;
		FreeListNode<T>* temp;
		while (top_->pNext)
		{
			FreeListNode<T>* temp = top_;
			top_ = top_->pNext;
			delete temp;
		}
	}
	
	// constr_dest_on_alloc_free_�� false�ε� ȣ���ϸ� ���α׷� ũ������
	template<typename V>
	__forceinline FreeListNode<T>* GetNodeFromHeap(const V& other)
	{
#ifdef _DEBUG
		++capacity_;
#endif
		return new FreeListNode<T>{ other };
	}
// Ÿ�Ի������.
	FreeListNode<T>* GetNodeFromHeap()
	{
#ifdef _DEBUG
		++capacity_;
#endif
		return new FreeListNode<T>;
	}

	template<typename U = T, typename V, typename = std::enable_if_t<std::is_class_v<U>>>
	T* Alloc(const V& other)
	{
		T* pData_to_user;
		if (IsEmpty())
			pData_to_user = &GetNodeFromHeap(other)->data;
		else
		{
			if(constr_dest_on_alloc_free_)
				new (pData_to_user) T(other);
		}
		return pData_to_user;
	}

	T* Alloc(void)
	{
		T* pData_to_user;
		if (IsEmpty())
		{
			pData_to_user = &GetNodeFromHeap()->data;
		}
		else
		{
			pData_to_user = &top_->data;
			top_ = top_->pNext;
			if (constr_dest_on_alloc_free_)
				new (pData_to_user) T{};
		}
#ifdef _DEBUG
			--size_;
			if (size_ < 0) size_ = 0;
#endif
		return pData_to_user;
	}

	__forceinline bool IsEmpty()
	{
		return top_ == &head_;
	}
	__forceinline void Push(FreeListNode<T>* pData)
	{
		FreeListNode<T>* next_node = top_;// ���� ž ��ġ ����
		top_ = pData;
		pData->pNext = next_node;
#ifdef _DEBUG
		++size_;
		if(size_ > capacity_)
			capacity_ = size_;
#endif 
	}
	__forceinline FreeListNode<T>* Top()
	{
		if(IsEmpty())
			return nullptr;

		return top_;
	}
	void Free(T* p)
	{
		if (constr_dest_on_alloc_free_)
			p->~T();
		int offset = offsetof(FreeListNode<T>, data);
		FreeListNode<T>* node = reinterpret_cast<FreeListNode<T>*>(reinterpret_cast<char*>(p) - offset);
		Push(node);
	}
	__forceinline void Pop()
	{
		if (IsEmpty())
			return;

		FreeListNode<T>* pop_target = top_;
		top_ = top_->pNext;
#ifdef _DEBUG
		--size_;
		if (size_ < 0) size_ = 0;
#endif
	}
	FreeListNode<T>* top_;
	FreeListNode<T> head_;
	bool constr_dest_on_alloc_free_;
};



