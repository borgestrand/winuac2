/*
   This file is part of the EMU CA0189 USB Audio Driver.

   Copyright (C) 2008 EMU Systems/Creative Technology Ltd. 

   This driver is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This driver is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library.   If not, a copy of the GNU Lesser General Public 
   License can be found at <http://www.gnu.org/licenses/>.
*/
/*
 *****************************************************************************
 *//*!
 * @file	   CList.h
 * @brief	   List template definition.
 * @copyright  E-MU Systems, 2004.
 * @author     hyhuang\@atc.creative.com.
 * @changelog  03-07-2005 1.00 Created.\n
 *//*
 *****************************************************************************
 */
#ifndef __CLIST_H__
#define __CLIST_H__

/*****************************************************************************
 * Templates
 */
/*****************************************************************************
 *//*! @class CList<_item>
 *****************************************************************************
 * @brief
 * List template.
 */
template<class _item>
class CList
{
private:
    _item *     m_Head;         /*!< @brief Head of the list. */
    _item *     m_Tail;         /*!< @brief Tail of the list. */
    ULONG       m_NumItems;     /*!< @brief Number of items in the list. */
    //KSPIN_LOCK  m_ListLock;     /*!< @brief List lock. */
    //KIRQL       m_Irql;         /*!< @brief Lock IRQL. */

public:
    /*! @brief Constructor. */
    CList()     { m_Head = m_Tail = NULL; m_NumItems = 0; /*KeInitializeSpinLock(&m_ListLock);*/}

    /*! @brief Destructor. */
	~CList()    { DeleteAllItems(); }

	//@{
    /*! @brief Methods for creating & destroying list item. */
    void DeleteAllItems()	{ _item * next, * current = m_Head;
							  while (current) {next = current->m_Next; current->Destruct(); current = next; }
							  m_Head = m_Tail = NULL; m_NumItems = 0; }
	//@}

	//@{
    /*! @brief Methods to lock/unlock the list for safe manipulation. */
    //void Lock()     { KeAcquireSpinLock(&m_ListLock, &m_Irql); }
    //void Unlock()   { KeReleaseSpinLock(&m_ListLock, m_Irql); }
	//@}

	//@{
    /*! @brief Methods for manipulation of the list. */
    inline
    BOOLEAN IsItemInList(_item * Item)   {
                                           #if (DBG)
                                           _item * current = m_Head;
                                           while (current) {if (current==Item) {ASSERT(Item->m_Owner==this); return TRUE;} current = current->m_Next;}
                                           ASSERT(Item->m_Owner!=this);
                                           return FALSE;
                                           #else
                                           return BOOLEAN(Item->m_Owner==this);
                                           #endif
                                         }

    BOOLEAN IsEmpty()                    { return (m_Head == NULL); }

    ULONG Put(_item * Item)              { ASSERT(!IsItemInList(Item));
                                           Item->m_Next = NULL;
                                           Item->m_Prev = NULL;
                                           Item->m_Owner = this;
                                           if (m_Head) {m_Head->m_Prev = Item; Item->m_Next = m_Head; m_Head = Item;}
                                           else {m_Head = m_Tail = Item;}
                                           m_NumItems++;
                                           return m_NumItems; }

    ULONG Remove(_item * Item)           { if (!IsItemInList(Item)) {ASSERT(0); return m_NumItems;}
                                           if (Item->m_Prev) {Item->m_Prev->m_Next = Item->m_Next;}
                                           if (Item->m_Next) {Item->m_Next->m_Prev = Item->m_Prev;}
                                           if (Item==m_Head) {m_Head = Item->m_Next;}
                                           if (Item==m_Tail) {m_Tail=Item->m_Prev;}
                                           Item->m_Next = NULL;
                                           Item->m_Prev = NULL;
                                           Item->m_Owner = NULL;
                                           m_NumItems--;
                                           return m_NumItems; }

    _item * Pop()                        { if (m_Tail) {_item * Item = m_Tail; Remove(Item); return Item;}
                                           return NULL; }

    _item * First()                      { return m_Tail; }
    _item * Last()                       { return m_Head; }

    _item * Next(_item * Item)           { ASSERT(IsItemInList(Item));
                                           if (!IsItemInList(Item)) return NULL;
                                           return Item->m_Prev; }

    _item * Prev(_item * Item)			 { ASSERT(IsItemInList(Item));
 										   if (!IsItemInList(Item)) return NULL;
										   return Item->m_Next; }

    ULONG Count()                        { return m_NumItems; }
	//@}
};

#endif // __CLIST_H__
