
// wGYPExplorerView.h : CwGYPExplorerView 类的接口
//

#pragma once


class CwGYPExplorerView : public CListView
{
protected: // 仅从序列化创建
	CwGYPExplorerView();
	DECLARE_DYNCREATE(CwGYPExplorerView)

// 特性
public:
	CwGYPExplorerDoc* GetDocument() const;

// 操作
public:

// 重写
public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual void OnInitialUpdate(); // 构造后第一次调用

// 实现
public:
	virtual ~CwGYPExplorerView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	afx_msg void OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct);
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // wGYPExplorerView.cpp 中的调试版本
inline CwGYPExplorerDoc* CwGYPExplorerView::GetDocument() const
   { return reinterpret_cast<CwGYPExplorerDoc*>(m_pDocument); }
#endif

