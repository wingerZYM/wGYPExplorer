
// wGYPExplorerView.h : CwGYPExplorerView ��Ľӿ�
//

#pragma once


class CwGYPExplorerView : public CListView
{
protected: // �������л�����
	CwGYPExplorerView();
	DECLARE_DYNCREATE(CwGYPExplorerView)

// ����
public:
	CwGYPExplorerDoc* GetDocument() const;

// ����
public:

// ��д
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // ������һ�ε���

// ʵ��
public:
	virtual ~CwGYPExplorerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // wGYPExplorerView.cpp �еĵ��԰汾
inline CwGYPExplorerDoc* CwGYPExplorerView::GetDocument() const
   { return reinterpret_cast<CwGYPExplorerDoc*>(m_pDocument); }
#endif

