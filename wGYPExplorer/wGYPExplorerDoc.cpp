
// wGYPExplorerDoc.cpp : CwGYPExplorerDoc 类的实现
//

#include "stdafx.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
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


// CwGYPExplorerDoc 构造/析构

CwGYPExplorerDoc::CwGYPExplorerDoc()
{
	// TODO:  在此添加一次性构造代码

}

CwGYPExplorerDoc::~CwGYPExplorerDoc()
{
}



// CwGYPExplorerDoc 序列化

void CwGYPExplorerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO:  在此添加存储代码
	}
	else
	{
		// TODO:  在此添加加载代码
		auto nLen = static_cast<UINT>(ar.GetFile()->GetLength());

		char *pBuff = new char[nLen];

		ar.Read(pBuff, nLen);

		gyp::Reader reader;

		if (reader.parse(pBuff, pBuff + nLen, m_Root))
		{

		}
		else
		{
			CString strMessage("读取gyp文件错误：");

			strMessage += reader.getFormatedErrorMessages().c_str();
			::AfxMessageBox(strMessage);
		}

		delete[] pBuff;
	}
}

#ifdef SHARED_HANDLERS

// 缩略图的支持
void CwGYPExplorerDoc::OnDrawThumbnail(CDC& dc, LPRECT lprcBounds)
{
	// 修改此代码以绘制文档数据
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

// 搜索处理程序的支持
void CwGYPExplorerDoc::InitializeSearchContent()
{
	CString strSearchContent;
	// 从文档数据设置搜索内容。
	// 内容部分应由“;”分隔

	// 例如:     strSearchContent = _T("point;rectangle;circle;ole object;")；
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

// CwGYPExplorerDoc 诊断

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


// CwGYPExplorerDoc 命令
