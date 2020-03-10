#pragma once

#include <QtCore>
#include "support.h"			// for class UsageCount

/*=============================================================
 * A deletable element
 * REMARKS:	- T must be
 *				assignable, copyable, can test equality
 *------------------------------------------------------------*/
template<typename T> struct UndeletableItem
{
	int batch = -100;	// it was deleted in this batch 
						// -1: not deleted, -100 invalid
						// (after group delete group undo possible)
	T t;				// object deleted/restored
	bool IsValid() { return batch != -100; }

	UndeletableItem() {}
	UndeletableItem(const T &t) : t(t) {}
	operator T() { return t; }
	operator T() const { return t;  }
};

/*=============================================================
* A list of element with delete and undo delete functionality
* REMARKS:	- more than one item can be deleted at the same time
*			- deleted elements can be undeleted
*			- can delete element only once
*			- consequtive undeletes possible
*			- items deleted together are undeleted together
*			- when a deleted item is undeleted no hint remains that
*			- it was deleted before
*
*			- all indexes in arguments refer to not-deleted items
*------------------------------------------------------------*/
template<typename T> class UndeletableItemList : public QVector<UndeletableItem<T> >
{
	typedef QVector < UndeletableItem<T> > BaseClassType;   

	int _lastBatchUsed = -1;		// each batch of co-deleted elements has a batch index
	UndeletableItem<T> _dummy;		// used to return an invalid item
	UsageCount _changed;			// incremented when element added to list, deleted
									// or un-deleted

	int _size = 0;					// count of non-deleted items ( < size() )
					// for speedup:
	int _prev_log = -0,				// previous logical index
		_prev_phys = 0;				// previous physical index 

	/*========================================================
	 * TASK:	gets real index for logical index
	 * PARAMS:	index - logical index
	 *			realFrom -start looking from this index
	 * GLOBALS:
	 * RETURNS: physical index or -1 index out of range
	 * REMARKS: - index  < _size
	 *-------------------------------------------------------*/
	int _RealIndexFor(int index)	
	{
		if (index < 0 || index > size())
			return -1;

		int ind = -1;
		int li = index;
		if(_size)
		{
			if (index < _prev_log)			// start from beginning
				_prev_log = _prev_phys = 0;	// may count backwards, but its not frequent I suppose

			index -= _prev_log;

			_prev_log = li;

			for (ind = _prev_phys; index >= 0 && ind < BaseClassType::size(); ++ind)
				if (BaseClassType::operator[](ind).batch < 0 )	// not deleted
					--index;
			_prev_phys = --ind;
		}
		return index >=0 ? -1 : ind;
	}
	//int _RealIndexFor(int index, int realFrom = 0)		  
	//{
	//	return /*const_cast<UndeletableItemList*>(this)->*/_RealIndexFor(index, realFrom);
	//}
public:
	UndeletableItemList() { _dummy.batch = -100;  }	// invalidate dummy
	UndeletableItemList(const UndeletableItemList &dtl)	: UndeletableItemList()
	{
		(void)operator=(dtl);
	}
	UndeletableItemList &operator=(const UndeletableItemList &other)
	{
		_lastBatchUsed = other._lastBatchUsed;
		_size = other._size;
		_changed = other._changed;
		QVector<UndeletableItem<T> >::operator=(other);
		return *this;
	}

	int size() const { return _size; }

	// return index-th not deleted element
	UndeletableItem<T> &operator[](int index) 
	{ 
		int i = _RealIndexFor(index);
		return  i >= 0 ? BaseClassType::operator[](i) : _dummy;
	}

	/*========================================================
	 * TASK:	check if element t is on the list (it may be
	 *			deleted)
	 * PARAMS:	t reference to element to be checked
	 *			asDeleted - only return true if the element is
	 *				deleted
	 * RETURNS: if element is present
	 *-------------------------------------------------------*/
	bool Contains(T &t, bool asDeleted = false) const
	{
		int i = BaseClassType::indexOf(t) >= 0;
		if (i < 0)
			return false;

		if(asDeleted)
			return BaseClassType::operator[](i).batch >= 0;
		return true;
	}

	/*========================================================
	 * TASK:	check if element t is deleted
	 * PARAMS:	reference to element checked
	 * RETURNS: if element is deleted
	 *-------------------------------------------------------*/
	bool IsDeleted(T &t) const
	{
		return Contains(t, true);
	}
	/*========================================================
	 * TASK:	check if element with index i is deleted
	 * PARAMS:	physical index of element to check
	 * RETURNS: if element is deleted
	 *-------------------------------------------------------*/
	bool IsDeleted(int i) const
	{
		return i >= 0 && i < BaseClassType::size() && BaseClassType::operator[](i).batch >= 0;
	}

	// delete index-th not deleted item
	void Delete(int index, int realFrom = 0)
	{
		int i = _RealIndexFor(index, realFrom);
		if (i >= 0)
		{
			T &t = (*this)[i];
			t.batch = ++_lastBatchUsed;		// batch > 0 then elem deleted
			--_size;
		}
		++_changed;
	}

	// delete all undeleted items with logical index 
	// in list 'what'
	void Delete(QList<int> what, int realFrom = 0)
	{
		int ind = 0;
		int batch = ++_lastBatchUsed;
		for (auto i : what)
		{
			ind = _RealIndexFor(i, ind);
			if (ind >= 0)
			{
				T &t = (*this)[ind];
				t.batch = batch;
				--_size;
			}
		}
		++_changed;
	}

	// delete all undeleted items with index 
	// between from and until, inclusive
	void Delete(int from, int until, int realFrom = 0)
	{
		int i;
		int ind = 0;
		int batch = ++_lastBatchUsed;
		for (i = from; i <= until && ind >= 0; ++i)
		{
			ind = _RealIndexFor(i, ind);
			if (ind >= 0)
			{
				T &t = (*this)[ind];
				t.batch = batch;
				--_size;
			}
		}
		++_changed;
	}
	// undelete last deleted item(s)
	void Undelete()		// last batch
	{
		if (!BaseClassType::size() || _lastBatchUsed < 0)
			return;
		for (int i = 0; i < BaseClassType::size(); ++i)
			if (BaseClassType::operator[](i).batch == _lastBatchUsed)
			{
				BaseClassType::operator[](i).batch = -1;
				++_size;
			}
		--_lastBatchUsed;
		--_changed;
	}

	void push_back(const T&t)
	{
		const UndeletableItem<T> dti(t);
		BaseClassType::push_back(dti);
		++_size;
		++_changed;
	}

	bool NothingToUndo() const { return _size == 0; }
	bool Changed() const { return _changed; }
};