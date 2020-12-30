#pragma once
#include "common.h"



class CHistoryStack {
public:
	CHistoryStack()
	{
		m_nDepth = 0;
		m_pLast = NULL;
	}
	~CHistoryStack()
	{
		ClearStack();
	}

public:
	void ClearStack()
	{
		if (m_pLast)
		{
			SHistory* pNode = m_pLast;
			SHistory* pTmp;
			do
			{
				pTmp = pNode->pPrev;
				delete pNode;
				pNode = pTmp;
			} while (pNode != NULL);
		}
		m_pLast = NULL;
	}

	void SaveHistory(FILE* fp)
	{
		fwrite(&m_nDepth, 1, sizeof(int), fp);
		if (m_pLast)
		{
			SHistory* pNode = m_pLast;
			do
			{
				fwrite(&pNode->enEatenPiece, 1, sizeof(PIECE), fp);
				fwrite(&pNode->nFromX, 1, sizeof(int), fp);
				fwrite(&pNode->nFromY, 1, sizeof(int), fp);
				fwrite(&pNode->nToX, 1, sizeof(int), fp);
				fwrite(&pNode->nToY, 1, sizeof(int), fp);

				pNode = pNode->pPrev;
			} while (pNode != NULL);
		}
	}
	void LoadHistory(FILE* fp)
	{
		ClearStack();
		SHistory* pNode = NULL, * pPrev = NULL;

		fread(&m_nDepth, 1, sizeof(int), fp);

		for (int i = 0; i < m_nDepth; ++i)
		{
			pNode = new SHistory();
			pNode->pPrev = pPrev;
			pPrev = pNode;
		}
		m_pLast = pPrev;
		if (m_pLast)
		{
			do
			{
				fread(&pNode->enEatenPiece, 1, sizeof(PIECE), fp);
				fread(&pNode->nFromX, 1, sizeof(int), fp);
				fread(&pNode->nFromY, 1, sizeof(int), fp);
				fread(&pNode->nToX, 1, sizeof(int), fp);
				fread(&pNode->nToY, 1, sizeof(int), fp);

				pNode = pNode->pPrev;
			} while (pNode != NULL);
		}
	}
	// 기보에 움직임 저장
	void Push(int nFromX, int nFromY, int nToX, int nToY, PIECE enEatenPiece)
	{
		SHistory* pNewHistory = new SHistory();
		pNewHistory->enEatenPiece = enEatenPiece;
		pNewHistory->nFromX = nFromX;
		pNewHistory->nFromY = nFromY;
		pNewHistory->nToX = nToX;
		pNewHistory->nToY = nToY;

		pNewHistory->pPrev = m_pLast;
		m_pLast = pNewHistory;
		++m_nDepth;
	}

	bool Pop(SHistory& PopStore)
	{
		if (m_pLast == NULL) return false;

		PopStore.enEatenPiece = m_pLast->enEatenPiece;
		PopStore.nFromX = m_pLast->nFromX;
		PopStore.nFromY = m_pLast->nFromY;
		PopStore.nToX = m_pLast->nToX;
		PopStore.nToY = m_pLast->nToY;

		SHistory* pTmp = m_pLast;
		m_pLast = m_pLast->pPrev;
		delete pTmp;
		--m_nDepth;

		return true;
	}

	bool GetLastMove(SHistory& LMStore)
	{
		if (m_pLast == NULL)
			return false;

		LMStore.enEatenPiece = m_pLast->enEatenPiece;
		LMStore.nFromX = m_pLast->nFromX;
		LMStore.nFromY = m_pLast->nFromY;
		LMStore.nToX = m_pLast->nToX;
		LMStore.nToY = m_pLast->nToY;

		return true;
	}

	int GetDepth() { return m_nDepth; }

private:
	int         m_nDepth;
	SHistory* m_pLast;
};