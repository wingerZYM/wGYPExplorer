
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
	auto &root = *pDoc->GetRoot();

	if (root.empty()) return;

	auto &treeCtrl = GetTreeCtrl();

	treeCtrl.DeleteAllItems();

	auto hItem = treeCtrl.InsertItem(pDoc->GetFileName());

	insertItem(treeCtrl, hItem, root);

	treeCtrl.Expand(hItem, TVE_EXPAND);
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
void CLeftView::insertItem(CTreeCtrl& treeCtrl, HTREEITEM hParent, gyp::Value& value)
{
	TVINSERTSTRUCT tvInsert;
	ZeroMemory(&tvInsert, sizeof(TVINSERTSTRUCT));

	tvInsert.hParent = hParent;
	tvInsert.hInsertAfter = TVI_LAST;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;

	for (auto iter = value.begin(); iter != value.end(); ++iter)
	{
		CString strName(iter.memberName());

		if (strName.IsEmpty()) continue;

		tvInsert.item.pszText = strName.GetBuffer();
		tvInsert.item.lParam = reinterpret_cast<LPARAM>(&*iter);

		auto hItem = treeCtrl.InsertItem(&tvInsert);

		if (_T("includes") == strName) continue;

		if (_T("conditions") == strName)
			insertConditions(treeCtrl, hItem, *iter);
		else if (_T("targets") == strName)
			insertTargets(treeCtrl, hItem, *iter);
		else
			insertItem(treeCtrl, hItem, *iter);
	}
}

void CLeftView::insertConditions(CTreeCtrl& treeCtrl, HTREEITEM hParent, gyp::Value& value)
{
	TVINSERTSTRUCT tvInsert;
	ZeroMemory(&tvInsert, sizeof(TVINSERTSTRUCT));

	tvInsert.hParent = hParent;
	tvInsert.hInsertAfter = TVI_LAST;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;

	for (auto iter = value.begin(); iter != value.end(); ++iter)
	{
		auto &node = *iter;
		ASSERT(2 == node.size() || 3 == node.size());

		auto &cond1 = node[static_cast<gyp::UInt>(1)];
		CString strName(node[static_cast<gyp::UInt>(0)].asString().c_str());

		tvInsert.item.pszText = strName.GetBuffer();
		tvInsert.item.lParam = reinterpret_cast<LPARAM>(&cond1);

		auto hItem = treeCtrl.InsertItem(&tvInsert);

		insertItem(treeCtrl, hItem, cond1);

		if (2 == node.size()) continue;// 是否是相反条件

		// 相反条件
		cutoverConditions(strName);
		auto &cond2 = node[static_cast<gyp::UInt>(2)];

		tvInsert.item.pszText = strName.GetBuffer();// 防止CString重新分配过内存。
		tvInsert.item.lParam = reinterpret_cast<LPARAM>(&cond2);

		hItem = treeCtrl.InsertItem(&tvInsert);

		insertItem(treeCtrl, hItem, cond2);
	}
}

void CLeftView::insertTargets(CTreeCtrl& treeCtrl, HTREEITEM hParent, gyp::Value& value)
{
	TVINSERTSTRUCT tvInsert;
	ZeroMemory(&tvInsert, sizeof(TVINSERTSTRUCT));

	tvInsert.hParent = hParent;
	tvInsert.hInsertAfter = TVI_LAST;
	tvInsert.item.mask = TVIF_TEXT | TVIF_PARAM;

	for (auto iter = value.begin(); iter != value.end(); ++iter)
	{
		CString strName(iter.memberName());

		tvInsert.item.pszText = strName.GetBuffer();
		tvInsert.item.lParam = reinterpret_cast<LPARAM>(&*iter);

		auto hItem = treeCtrl.InsertItem(&tvInsert);
	}
}

void CLeftView::cutoverConditions(CString &strCond)
{
	auto pStr = strCond.GetBuffer();

	auto c = pStr + strCond.GetLength() - 1;
	if ('1' == *c)
	{
		*c = '0';
	}
	else if ('0' == *c)
	{
		*c = '1';
	}
	else
	{
		int nFind;
		if (-1 != (nFind = strCond.Find('!')))
		{
			pStr[nFind] = '=';
		}
		else if (-1 != (nFind = strCond.Find('=')))
		{
			pStr[nFind] = '!';
		}
	}

	strCond.ReleaseBuffer();
}
