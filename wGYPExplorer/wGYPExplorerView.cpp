
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
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CwGYPExplorerView::OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, &CwGYPExplorerView::OnLvnEndlabeledit)
END_MESSAGE_MAP()

// CwGYPExplorerView 构造/析构

CwGYPExplorerView::CwGYPExplorerView()
	: m_pParent(nullptr)
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
//	cs.dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	cs.style |= LVS_REPORT | LVS_EDITLABELS;

	return CListView::PreCreateWindow(cs);
}

void CwGYPExplorerView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();


	TRACE("view is init update!\n");
	// TODO:  调用 GetListCtrl() 直接访问 ListView 的列表控件，
	//  从而可以用项填充 ListView。
	auto &listCtrl = GetListCtrl();

	listCtrl.SetExtendedStyle(listCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	if (!listCtrl.GetHeaderCtrl()->GetItemCount())
	{
		CRect rc;
		listCtrl.GetWindowRect(&rc);

		listCtrl.InsertColumn(0, _T("描述"), LVCFMT_LEFT, rc.Width() - 20);
	}
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


void CwGYPExplorerView::UpdateList(gyp::Value *pValue)
{
	m_pParent = pValue;
	auto &listCtrl = GetListCtrl();

	listCtrl.DeleteAllItems();

	LVITEM lvItem;
	ZeroMemory(&lvItem, sizeof(LVITEM));

	lvItem.mask = LVIF_TEXT | LVIF_PARAM;

	CString strValue;
	switch (pValue->type())
	{
	case gyp::intValue:
		strValue.Format(_T("%d"), pValue->asInt());
		break;
	case gyp::uintValue:
		strValue.Format(_T("%u"), pValue->asUInt());
		break;
	case gyp::realValue:
		strValue.Format(_T("%f"), pValue->asDouble());
		break;
	case gyp::stringValue:
		strValue = pValue->asCString();
		break;
	case gyp::booleanValue:
		strValue = pValue->asBool() ? "true" : "false";
		break;
	case gyp::arrayValue:
		for (auto iter = pValue->begin(); iter != pValue->end(); ++iter)
		{
			auto &node = *iter;

			if (!node.isString())
			{
				TRACE("found no string type item at arrayValue. type is:%d\n", node.type());
				continue;
			}

			strValue = node.asCString();

			lvItem.pszText = strValue.GetBuffer();
			lvItem.lParam = reinterpret_cast<LPARAM>(&node);

			listCtrl.InsertItem(&lvItem);
		}
		return;// 注意
	case gyp::objectValue:// 这个类型说明是有折叠项，怎么展示还没想好……
		return;
	default:
		return;
	}

	lvItem.pszText = strValue.GetBuffer();
	lvItem.lParam = reinterpret_cast<LPARAM>(pValue);

	listCtrl.InsertItem(&lvItem);
}

void CwGYPExplorerView::addNewItem()
{
	auto &listCtrl = GetListCtrl();
	int nItem = listCtrl.InsertItem(INT_MAX, _T(""));
	auto pEdit = listCtrl.EditLabel(nItem);
}

// CwGYPExplorerView 消息处理程序
void CwGYPExplorerView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	//TODO:  添加代码以响应用户对窗口视图样式的更改	
	CListView::OnStyleChanged(nStyleType,lpStyleStruct);	
}

void CwGYPExplorerView::OnSize(UINT nType, int cx, int cy)
{
	CListView::OnSize(nType, cx, cy);

	// TODO:  在此处添加消息处理程序代码
	auto &listCtrl = GetListCtrl();

	if (listCtrl.GetHeaderCtrl()->GetItemCount())
	{
		listCtrl.SetColumnWidth(0, cx - 20);
	}
}

void CwGYPExplorerView::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// TODO:  在此添加控件通知处理程序代码

	if (m_pParent)
	{
		if (pNMItemActivate->iItem < 0)// 是否选中现有项
		{
			addNewItem();
		}
		else
		{
			auto &listCtrl = GetListCtrl();
			listCtrl.EditLabel(pNMItemActivate->iItem);
		}
	}

	*pResult = 0;
}

void CwGYPExplorerView::OnLvnEndlabeledit(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);

	// TODO:  在此添加控件通知处理程序代码
	auto &listCtrl = GetListCtrl();
	if (pDispInfo->item.pszText && _tcslen(pDispInfo->item.pszText))// 编译框内有文字
	{
		if (pDispInfo->item.lParam)// 编缉旧项
		{
			auto pValue = reinterpret_cast<gyp::Value*>(pDispInfo->item.lParam);

			*pValue = pDispInfo->item.pszText;
		}
		else// 新添加项
		{
			gyp::Value value(pDispInfo->item.pszText);

			pDispInfo->item.lParam = reinterpret_cast<LPARAM>(&m_pParent->append(value));
		}
		listCtrl.SetItem(&pDispInfo->item);
		GetDocument()->SetModifiedFlag(true);
	}
	else// 编译框无文字
	{
		if (listCtrl.GetItemText(pDispInfo->item.iItem, 0).IsEmpty())
		{
			listCtrl.DeleteItem(pDispInfo->item.iItem);
		}
	}

	*pResult = 0;
}
