
// wGYPExplorerDoc.cpp : CwGYPExplorerDoc ���ʵ��
//

#include "stdafx.h"
// SHARED_HANDLERS ������ʵ��Ԥ��������ͼ������ɸѡ�������
// ATL ��Ŀ�н��ж��壬�����������Ŀ�����ĵ����롣
#ifndef SHARED_HANDLERS
#include "wGYPExplorer.h"
#endif

#include "wGYPExplorerDoc.h"

#include <propkey.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CwGYPExplorerDoc

IMPLEMENT_DYNCREATE(CwGYPExplorerDoc, CDocument)

BEGIN_MESSAGE_MAP(CwGYPExplorerDoc, CDocument)
END_MESSAGE_MAP()


// CwGYPExplorerDoc ����/����

CwGYPExplorerDoc::CwGYPExplorerDoc()
{
	// TODO:  �ڴ����һ���Թ������

}

CwGYPExplorerDoc::~CwGYPExplorerDoc()
{
}



// CwGYPExplorerDoc ���л�

void CwGYPExplorerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO:  �ڴ���Ӵ洢����
	}
	else
	{
		// TODO:  �ڴ���Ӽ��ش���
		auto nLen = static_cast<UINT>(ar.GetFile()->GetLength());

		char *pBuff = new char[nLen];

		ar.Read(pBuff, nLen);

		gyp::Reader reader;

		if (reader.parse(pBuff, pBuff + nLen, m_Root))
		{

		}
		else
		{
			CString strMessage("��ȡgyp�ļ�����");

			strMessage += reader.getFormatedErrorMessages().c_str();
			::AfxMessageBox(strMessage);
		}

		delete[] pBuff;
	}
}

#ifdef SHARED_HANDLERS

// ����ͼ��֧��
void CwGYPExplorerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// �޸Ĵ˴����Ի����ĵ�����
	dc.FillSolidRect(lprcBounds, RGB(255, 255, 255));

	CString strText = _T("TODO: implement thumbnail drawing here");
	LOGFONT lf;

	CFont* pDefaultGUIFont = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
	pDefaultGUIFont->GetLogFont(&lf);
	lf.lfHeight = 36;

	CFont fontDraw;
	fontDraw.CreateFontIndirect(&lf);

	CFont* pOldFont = dc.SelectObject(&fontDraw);
	dc.DrawText(strText, lprcBounds, DT_CENTER | DT_WORDBREAK);
	dc.SelectObject(pOldFont);
}

// ������������֧��
void CwGYPExplorerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// ���ĵ����������������ݡ�
	// ���ݲ���Ӧ�ɡ�;���ָ�

	// ����:     strSearchContent = _T("point;rectangle;circle;ole object;")��
	SetSearchContent(strSearchContent);
}

void CwGYPExplorerDoc::SetSearchContent(const CString& value)
{
	if (value.IsEmpty())
	{
		RemoveChunk(PKEY_Search_Contents.fmtid, PKEY_Search_Contents.pid);
	}
	else
	{
		CMFCFilterChunkValueImpl *pChunk = NULL;
		ATLTRY(pChunk = new CMFCFilterChunkValueImpl);
		if (pChunk != NULL)
		{
			pChunk->SetTextValue(PKEY_Search_Contents, value, CHUNK_TEXT);
			SetChunkValue(pChunk);
		}
	}
}

#endif // SHARED_HANDLERS

// CwGYPExplorerDoc ���

#ifdef _DEBUG
void CwGYPExplorerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CwGYPExplorerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CwGYPExplorerDoc ����
