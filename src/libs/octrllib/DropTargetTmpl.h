/*
 * Copyright (c) 2025, Atsushi Ogawa
 * All rights reserved.
 *
 * This software is licensed under the BSD License.
 * See the LICENSE_BSD file for details.
 */

#ifndef _DropTargetTmpl_H_INCLUDED_
#define _DropTargetTmpl_H_INCLUDED_

template <class T>
class DropTargetTmpl : public COleDropTarget
{
// Constructors
public:
	DropTargetTmpl() {};

// Overridables
	virtual DROPEFFECT OnDragEnter(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point)
	{
		T *pedit = (T *)pWnd;
		return pedit->DragEnter(pDataObject, dwKeyState, point);
	}

	virtual DROPEFFECT OnDragOver(CWnd* pWnd, COleDataObject* pDataObject,
		DWORD dwKeyState, CPoint point)
	{
		T *pedit = (T *)pWnd;
		return pedit->DragOver(pDataObject, dwKeyState, point);
	}

	virtual BOOL OnDrop(CWnd* pWnd, COleDataObject* pDataObject,
		DROPEFFECT dropEffect, CPoint point)
	{
		T *pedit = (T *)pWnd;
		return pedit->Drop(pDataObject, dropEffect, point);
	}

//	virtual DROPEFFECT OnDropEx(CWnd* pWnd, COleDataObject* pDataObject,
//		DROPEFFECT dropDefault, DROPEFFECT dropList, CPoint point);
	virtual void OnDragLeave(CWnd* pWnd)
	{
		T *pedit = (T *)pWnd;
		pedit->DragLeave();
	}

	virtual DROPEFFECT OnDragScroll(CWnd* pWnd, DWORD dwKeyState,
		CPoint point)
	{
		return DROPEFFECT_NONE;
	}

// Implementation
public:
	virtual ~DropTargetTmpl() {};

// Interface Maps
//public:
//	BEGIN_INTERFACE_PART(DropTarget, IDropTarget)
//		INIT_INTERFACE_PART(CGrDropTarget, DropTarget)
//		STDMETHOD(DragEnter)(LPDATAOBJECT, DWORD, POINTL, LPDWORD);
//		STDMETHOD(DragOver)(DWORD, POINTL, LPDWORD);
//		STDMETHOD(DragLeave)();
//		STDMETHOD(Drop)(LPDATAOBJECT, DWORD, POINTL pt, LPDWORD);
//	END_INTERFACE_PART(DropTarget)
//
//	DECLARE_INTERFACE_MAP()
};

#endif _DropTargetTmpl_H_INCLUDED_