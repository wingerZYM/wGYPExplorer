
// LeftView.cpp : CLeftView 类的实现
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


// CLeftView 构造/析构

CLeftView::CLeftView()
{
	// TODO:  在此处添加构造代码
}

CLeftView::~CLeftView()
{
}

BOOL CLeftView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  在此处通过修改 CREATESTRUCT cs 来修改窗口类或样式
	cs.style |= TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;

	return CTreeView::PreCreateWindow(cs);
}

void CLeftView::OnInitialUpdate()
{
	CTreeView::OnInitialUpdate();

	TRACE("tree is init update!\n");
	// TODO:  调用 GetTreeCtrl() 直接访问 TreeView 的树控件，
	//  从而可以用项填充 TreeView。
	auto pDoc = GetDocument();
	const auto &root = *pDoc->GetRoot();

	if (root.empty()) return;

	auto &treeCtrl = GetTreeCtrl();

	treeCtrl.DeleteAllItems();

	HTREEITEM hItem;
	if (pDoc->IsGypi())
		hItem = treeCtrl.InsertItem(_T("GYPI文件"));
	else
		hItem = treeCtrl.InsertItem(_T("GYP文件"));

	insertItem(treeCtrl, hItem, root);
}


// CLeftView 诊断

#ifdef _DEBUG
void CLeftView::AssertValid() const
{
	CTreeView::AssertValid();
}

void CLeftView::Dump(CDumpContext& dc) const
{
	CTreeView::Dump(dc);
}

CwGYPExplorerDoc* CLeftView::GetDocument() // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CwGYPExplorerDoc)));
	return (CwGYPExplorerDoc*)m_pDocument;
}
#endif //_DEBUG


// CLeftView 消息处理程序
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
