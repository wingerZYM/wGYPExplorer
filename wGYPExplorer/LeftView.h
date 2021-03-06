
// LeftView.h : CLeftView 类的接口
//


#pragma once

namespace gyp
{
class Value;
}

class CwGYPExplorerDoc;

class CLeftView : public CTreeView
{
protected: // 仅从序列化创建
	CLeftView();
	DECLARE_DYNCREATE(CLeftView)

// 特性
public:
	CwGYPExplorerDoc* GetDocument();

// 操作
public:

// 重写
	public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	protected:
	virtual void OnInitialUpdate(); // 构造后第一次调用

// 实现
public:
	virtual ~CLeftView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// 生成的消息映射函数
protected:
	DECLARE_MESSAGE_MAP()
private:
	void insertItem(CTreeCtrl& treeCtrl, HTREEITEM hParent, gyp::Value& value);
	void insertConditions(CTreeCtrl& treeCtrl, HTREEITEM hParent, gyp::Value& value);
	void insertTargets(CTreeCtrl& treeCtrl, HTREEITEM hParent, gyp::Value& value);
	void cutoverConditions(CString &strCond);// 切换条件

	afx_msg void OnTvnSelchanged(NMHDR *pNMHDR, LRESULT *pResult);
};

#ifndef _DEBUG  // LeftView.cpp 中的调试版本
inline CwGYPExplorerDoc* CLeftView::GetDocument()
   { return reinterpret_cast<CwGYPExplorerDoc*>(m_pDocument); }
#endif

