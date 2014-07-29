
// LeftView.h : CLeftView ��Ľӿ�
//


#pragma once

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
	void insertItem(CTreeCtrl& treeCtrl, HTREEITEM hParent, const gyp::Value& value);
};

#ifndef _DEBUG  // LeftView.cpp �еĵ��԰汾
inline CwGYPExplorerDoc* CLeftView::GetDocument()
   { return reinterpret_cast<CwGYPExplorerDoc*>(m_pDocument); }
#endif

