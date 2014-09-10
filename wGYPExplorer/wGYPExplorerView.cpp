
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
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CwGYPExplorerView::OnNMDblclk)
	ON_NOTIFY_REFLECT(LVN_ENDLABELEDIT, &CwGYPExplorerView::OnLvnEndlabeledit)
END_MESSAGE_MAP()

// CwGYPExplorerView ����/����

CwGYPExplorerView::CwGYPExplorerView()
	: m_pParent(nullptr)
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
//	cs.dwExStyle |= LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES;
	cs.style |= LVS_REPORT | LVS_EDITLABELS;

	return CListView::PreCreateWindow(cs);
}

void CwGYPExplorerView::OnInitialUpdate()
{
	CListView::OnInitialUpdate();


	TRACE("view is init update!\n");
	// TODO:  ���� GetListCtrl() ֱ�ӷ��� ListView ���б�ؼ���
	//  �Ӷ������������ ListView��
	auto &listCtrl = GetListCtrl();

	listCtrl.SetExtendedStyle(listCtrl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	if (!listCtrl.GetHeaderCtrl()->GetItemCount())
	{
		CRect rc;
		listCtrl.GetWindowRect(&rc);

		listCtrl.InsertColumn(0, _T("����"), LVCFMT_LEFT, rc.Width() - 20);
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
		return;// ע��
	case gyp::objectValue:// �������˵�������۵����ôչʾ��û��á���
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

// CwGYPExplorerView ��Ϣ�������
void CwGYPExplorerView::OnStyleChanged(int nStyleType, LPSTYLESTRUCT lpStyleStruct)
{
	//TODO:  ��Ӵ�������Ӧ�û��Դ�����ͼ��ʽ�ĸ���	
	CListView::OnStyleChanged(nStyleType,lpStyleStruct);	
}

void CwGYPExplorerView::OnSize(UINT nType, int cx, int cy)
{
	CListView::OnSize(nType, cx, cy);

	// TODO:  �ڴ˴������Ϣ����������
	auto &listCtrl = GetListCtrl();

	if (listCtrl.GetHeaderCtrl()->GetItemCount())
	{
		listCtrl.SetColumnWidth(0, cx - 20);
	}
}

void CwGYPExplorerView::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);

	// TODO:  �ڴ���ӿؼ�֪ͨ����������

	if (m_pParent)
	{
		if (pNMItemActivate->iItem < 0)// �Ƿ�ѡ��������
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

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	auto &listCtrl = GetListCtrl();
	if (pDispInfo->item.pszText && _tcslen(pDispInfo->item.pszText))// �������������
	{
		if (pDispInfo->item.lParam)// �༩����
		{
			auto pValue = reinterpret_cast<gyp::Value*>(pDispInfo->item.lParam);

			*pValue = pDispInfo->item.pszText;
		}
		else// �������
		{
			gyp::Value value(pDispInfo->item.pszText);

			pDispInfo->item.lParam = reinterpret_cast<LPARAM>(&m_pParent->append(value));
		}
		listCtrl.SetItem(&pDispInfo->item);
		GetDocument()->SetModifiedFlag(true);
	}
	else// �����������
	{
		if (listCtrl.GetItemText(pDispInfo->item.iItem, 0).IsEmpty())
		{
			listCtrl.DeleteItem(pDispInfo->item.iItem);
		}
	}

	*pResult = 0;
}
