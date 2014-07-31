
// LeftView.h : CLeftView ��Ľӿ�
//


#pragma once

namespace gyp
{
class Value;
}

class CwGYPExplorerDoc;

class CLeftView : public CTreeView
{
protected: // �������л�����
	CLeftView();
	DECLARE_DYNCREATE(CLeftView)

// ����
public:
	CwGYPExplorerDoc* GetDocument();

// ����
public:

// ��д
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // ������һ�ε���

// ʵ��
public:
	virtual ~CLeftView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()
private:
	void insertItem(CTreeCtrl& treeCtrl, HTREEITEM hParent, gyp::Value& value);
	void insertConditions(CTreeCtrl& treeCtrl, HTREEITEM hParent, gyp::Value& value);
	void insertTargets(CTreeCtrl& treeCtrl, HTREEITEM hParent, gyp::Value& value);
	void cutoverConditions(CString &strCond);// �л�����

	afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // LeftView.cpp �еĵ��԰汾
inline CwGYPExplorerDoc* CLeftView::GetDocument()
   { return reinterpret_cast<CwGYPExplorerDoc*>(m_pDocument); }
#endif

