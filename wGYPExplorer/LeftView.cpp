
// LeftView.cpp : CLeftView ���ʵ��
//

#include "stdafx.h"
#include "wGYPExplorer.h"

#include "wGYPExplorerDoc.h"
#include "LeftView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLeftView

IMPLEMENT_DYNCREATE(CLeftView, CTreeView)

BEGIN_MESSAGE_MAP(CLeftView, CTreeView)
//	ON_WM_CREATE()
END_MESSAGE_MAP()


// CLeftView ����/����

CLeftView::CLeftView()
{
	// TODO:  �ڴ˴���ӹ������
}

CLeftView::~CLeftView()
{
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  �ڴ˴�ͨ���޸� CREATESTRUCT cs ���޸Ĵ��������ʽ
	cs.style |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	return CTreeView::PreCreateWindow(cs);
}

void CLeftView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	TRACE("tree is init update!\n");
	// TODO:  ���� GetTreeCtrl() ֱ�ӷ��� TreeView �����ؼ���
	//  �Ӷ������������ TreeView��
	auto pDoc = GetDocument();
	const auto &root = *pDoc->GetRoot();

	if (root.empty()) return;

	auto &treeCtrl = GetTreeCtrl();

	treeCtrl.DeleteAllItems();

	HTREEITEM hItem;
	if (pDoc->IsGypi())
		hItem = treeCtrl.InsertItem(_T("GYPI�ļ�"));
	else
		hItem = treeCtrl.InsertItem(_T("GYP�ļ�"));

	insertItem(treeCtrl, hItem, root);
}


// CLeftView ���

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CLeftView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CwGYPExplorerDoc* CLeftView::GetDocument() // �ǵ��԰汾��������
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CwGYPExplorerDoc)));
	return (CwGYPExplorerDoc*)m_pDocument;
}
#endif //_DEBUG


// CLeftView ��Ϣ�������
void CLeftView::insertItem(CTreeCtrl& treeCtrl, HTREEITEM hParent, const gyp::Value& value)
{
	for (auto iter = value.begin(); iter != value.end(); ++iter)
	{
		CString strName(iter.memberName());

		if (strName.IsEmpty()) continue;
		auto hItem = treeCtrl.InsertItem(strName, hParent);

		insertItem(treeCtrl, hItem, *iter);
	}
}
