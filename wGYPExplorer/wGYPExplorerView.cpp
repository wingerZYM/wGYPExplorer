
// wGYPExplorerView.cpp : CwGYPExplorerView ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
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

// CwGYPExplorerView ����/����

CwGYPExplorerView::CwGYPExplorerView()
{
	// TODO:  �ڴ˴���ӹ������

}

CwGYPExplorerView::~CwGYPExplorerView()
{
}

BOOL CwGYPExplorerView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  �ڴ˴�ͨ���޸�
	//  CREATESTRUCT cs ���޸Ĵ��������ʽ

	return CListView::PreCreateWindow(cs);
}

void CwGYPExplorerView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();


	// TODO:  ���� GetListCtrl() ֱ�ӷ��� ListView ���б�ؼ���
	//  �Ӷ������������ ListView��
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


// CwGYPExplorerView ���

#ifdef _DEBUG
void CwGYPExplorerView::AssertValid() const
{
	CListView::AssertValid();
}

void CwGYPExplorerView::Dump(CDumpContext& dc) const
{
	CListView::Dump(dc);
}

CwGYPExplorerDoc* CwGYPExplorerView::GetDocument() const // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CwGYPExplorerDoc)));
	return (CwGYPExplorerDoc*)m_pDocument;
}
#endif //_DEBUG


// CwGYPExplorerView ��Ϣ�������
void CwGYPExplorerView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	//TODO:  ��Ӵ�������Ӧ�û��Դ�����ͼ��ʽ�ĸ���	
	CListView::OnStyleChanged(nStyleType,lpStyleStruct);	
}
