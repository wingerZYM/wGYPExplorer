
// wGYPExplorerView.cpp : CwGYPExplorerView 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "wGYPExplorer.h"
#endif

#include "wGYPExplorerDoc.h"
#include "wGYPExplorerView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CwGYPExplorerView

IMPLEMENT_DYNCREATE(CwGYPExplorerView, CListView)

BEGIN_MESSAGE_MAP(CwGYPExplorerView, CListView)
	ON_WM_STYLECHANGED()
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CwGYPExplorerView 构造/析构

CwGYPExplorerView::CwGYPExplorerView()
{
	// TODO:  在此处添加构造代码

}

CwGYPExplorerView::~CwGYPExplorerView()
{
}

BOOL CwGYPExplorerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CListView::PreCreateWindow(cs);
}

void CwGYPExplorerView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();


	// TODO:  调用 GetListCtrl() 直接访问 ListView 的列表控件，
	//  从而可以用项填充 ListView。
	auto &listCtrl = GetListCtrl();
}

void CwGYPExplorerView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CwGYPExplorerView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CwGYPExplorerView 诊断

#ifdef _DEBUG
void CwGYPExplorerView::AssertValid() const
{
	CListView::AssertValid();
}

void CwGYPExplorerView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CwGYPExplorerDoc* CwGYPExplorerView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CwGYPExplorerDoc)));
	return (CwGYPExplorerDoc*)m_pDocument;
}
#endif //_DEBUG


// CwGYPExplorerView 消息处理程序
void CwGYPExplorerView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	//TODO:  添加代码以响应用户对窗口视图样式的更改	
	CListView::OnStyleChanged(nStyleType,lpStyleStruct);	
}
