
// wGYPExplorerDoc.h : CwGYPExplorerDoc ��Ľӿ�
//


#pragma once

#include "gyp.h"


class CwGYPExplorerDoc : public CDocument
{
protected: // �������л�����
	CwGYPExplorerDoc();
	DECLARE_DYNCREATE(CwGYPExplorerDoc)

// ����
public:

// ����
public:

// ��д
public:
	virtual void Serialize(CArchive& ar);
	virtual BOOL OnNewDocument();
#ifdef SHARED_HANDLERS
	virtual void InitializeSearchContent();
	virtual void OnDrawThumbnail(CDC& dc, LPRECT lprcBounds);
#endif // SHARED_HANDLERS

// ʵ��
public:
	virtual ~CwGYPExplorerDoc();

	gyp::Value *GetRoot() { return &m_Root; }
	bool IsGypi() const { return m_bIsGypi; }
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

// ����
private:
	bool m_bFirst;
	bool m_bIsGypi;
	gyp::Value m_Root;

// ���ɵ���Ϣӳ�亯��
protected:
	DECLARE_MESSAGE_MAP()

#ifdef SHARED_HANDLERS
	// ����Ϊ����������������������ݵ� Helper ����
	void SetSearchContent(const CString& value);
#endif // SHARED_HANDLERS
};
