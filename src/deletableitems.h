#pragma once

#include <QtCore>
#include "support.h"			// for class UsageCount

/*=============================================================
 * A deletable element
 * REMARKS:
 *------------------------------------------------------------*/
template<typename T> struct DeletableItem
{
	int batch = -100;	// it was deleted in this batch 
						// -1: not deleted, -100 invalid
						// (after group delete group undo possible)
	T t;				// object deleted/restored
	bool IsValid() { return batch != -100; }

	DeletableItem() {}
	DeletableItem(const T &t) : t(t) {}
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
template<typename T> class DeletableItemList : public QVector<DeletableItem<T> >
{
	int _lastBatchUsed = -1;		// each batch of co-deleted elements has a batch index
	DeletableItem<T> _dummy;		// used to return an invalid item
	UsageCount _changed;			// incremented when element added or deleted
									// undelete decrements it
	typedef QVector < DeletableItem<T> > DeleteItemVector;

	int _size = 0;					// count of non-deleted items (can be < size() )
	int _prev_log = -0,				// previous logical index
		_prev_phys = 0;				// previous physical index (for speedup)

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
		if (index < 0 || index > 100)
			return -1;

		int ind = -1;
		int li = index;
		if(_size)
		{
			if (index < _prev_log)			// start from beginning
				_prev_log = _prev_phys = 0;	// may count backwards, but its not frequent I suppose

			index -= _prev_log;

			_prev_log = li;

			for (ind = _prev_phys; index >= 0 && ind < DeleteItemVector::size(); ++ind)
				if (DeleteItemVector::operator[](ind).batch < 0 )	// not deleted
					--index;
			_prev_phys = --ind;
		}
		return index >=0 ? -1 : ind;
	}
	//int _RealIndexFor(int index, int realFrom = 0)		  
	//{
	//	return /*const_cast<DeletableItemList*>(this)->*/_RealIndexFor(index, realFrom);
	//}
public:
	DeletableItemList() { _dummy.batch = -100;  }	// invalidate dummy
	DeletableItemList(const DeletableItemList &dtl)	: DeletableItemList()
	{
		_lastBatchUsed = dtl._lastBatchUsed;
		_size = dtl._size;
		QVector<DeletableItem<T> >::operator=(dtl);
	}

	int size() const { return _size; }

	// return index-th not deleted element
	DeletableItem<T> &operator[](int index) 
	{ 
		int i = _RealIndexFor(index);
		return  i >= 0 ? DeleteItemVector::operator[](i) : _dummy;
	}
	//DeletableItem<T> operator[](int index)
	//{
	//	int i = _RealIndexFor(index);
	//	const DeletableItem<T> t = DeleteItemVector::operator[](i);
	//	return  i >= 0 ? t : _dummy;
	//}

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
		if (!DeleteItemVector::size() || _lastBatchUsed < 0)
			return;
		for (int i = 0; i < DeleteItemVector::size(); ++i)
			if (DeleteItemVector::operator[](i).batch == _lastBatchUsed)
			{
				DeleteItemVector::operator[](i).batch = -1;
				++_size;
			}
		--_lastBatchUsed;
		--_changed;
	}

	void push_back(const T&t)
	{
		const DeletableItem<T> dti(t);
		DeleteItemVector::push_back(dti);
		++_size;
		++_changed;
	}

	bool NothingToUndo() const { return _size == 0; }
	bool Changed() const { return _changed; }
};