#pragma once


#pragma once 
#include "common.h"

class CHistoryStack;

class CMoveData {
public:
	CMoveData()
	{
		m_nIndexCount = 0;
	}
	void SetFromXY(int nX, int nY)
	{
		m_nIndexCount = 0;
		m_nFromX = nX;
		m_nFromY = nY;
	}
	void AddToXY(int nX, int nY)
	{
		m_nToX[m_nIndexCount] = nX;
		m_nToY[m_nIndexCount] = nY;
		++m_nIndexCount;
	}
	int GetIndexCount() { return m_nIndexCount; }
	int GetFromX() { return m_nFromX; }
	int GetFromY() { return m_nFromY; }
	int GetToX(int nIndex) { return m_nToX[nIndex]; }
	int GetToY(int nIndex) { return m_nToY[nIndex]; }
	void Reset() { m_nIndexCount = 0; }

private:
	int m_nFromX;           // X location of a piece to move from
	int m_nFromY;           // Y location of a piece to move from
	int m_nToX[40];         // X location of a piece to move to
	int m_nToY[40];         // Y location of a piece to move to
	int m_nIndexCount;      // Available location count
};



class Search {
public:
	Search();
	int	 getVersion() { return _VERSION_; }
	void initParams(int depth, int hashLevel, int entryLimitTime, int maxLimitTime);
	void print();
	void addPiece(int x, int y, char cp);
	void defaultSetting(void);
	int  isLegalMove(int nFromX, int nFromY, int nToX, int nToY);
	int  makeMove(int from_x, int from_y, int to_x, int to_y);
	bool moveBack();
	MV 	 getBestMove(SIDE sdplayer, int repeatmv);
#ifdef EMSCRIPTEN
	emscripten::val getNextPieceMoves(int from_x, int from_y);
#else
	int getNextPieceMoves(int from_x, int from_y, MV* mvs);
#endif
	int generateMoves(SIDE side, MV* mvs);
	SIDE    GetSideAt(int nX, int nY);
	bool    GetNextPieceLoc(int nFromX, int nFromY, CMoveData* cMD);
	PIECE  GetPiece(int nIndexX, int nIndexY) { return m_enPanel[nIndexY][nIndexX]; }
	int 	getScore(void) { return m_nScore; };

public:
	static  CHistoryStack   sm_cHistoryStack;
	long    zobristKey;
	long    zobristLock;

private:
	PIECE  m_enPanel[DF_PANEL_HEIGHT][DF_PANEL_WIDTH]; // 장기판

	int     m_nScore;
	char    m_bAmIMax;
	PIECE  m_enEatenPiece;  // capture한 기물
	int     m_nFromX;
	int     m_nFromY;
	int     m_nToX;
	int     m_nToY;
	int     m_nNodeCount;  // 리스트에 추가된 노드 갯수

	static  int sm_nTotalNode;   // 전체 총 node수

	// Flags for heuristic
	static int sm_nCutOffDepth;
	static int sm_nFeedOverDepth;

	SIDE    m_enSide;		// 어느 팀을 추론하는가

	// Alphp-Beta pruning의 효율을 높이기위한 child node list
	Search* m_pPrev;
	Search* m_pNext;
	Search* m_pFirst;
	Search* m_pLast;
	Search* m_pParent;

	int		m_nCount;	   // move count

protected:
	Search(Search* pParentSearch);
	void 	Initialize();
	void 	AddPiece(int x, int y, PIECE pc, bool bDel);
	int  	MakeMove(int from_x, int from_y, int to_x, int to_y);
	bool    UndoMove();
	void	changeSide(void);
	bool    IsPassable(int nX, int nY);
	void    GetMovableSaAndJang(int nFromX, int nFromY);
	void    GetMovableJol(int nFromX, int nFromY);
	void    GetMovablePo(int nFromX, int nFromY);
	void    GetMovableCha(int nFromX, int nFromY);
	void    GetMovableMa(int nFromX, int nFromY);
	void    GetMovableSang(int nFromX, int nFromY);
	bool    TryToAddMovableXY(int nFromX, int nFromY, int nToX, int nToY);
	void    ClearList();
	bool    MovePiece(int nFromX, int nFromY, int nToX, int nToY);
	bool    IsMaxTurn() { return m_bAmIMax; }
	int     MaxValue(int nAlpha, int nBeta, int depth);
	int     MinValue(int nAlpha, int nBeta, int depth);
	void    AddNextStageEffectively(Search* pNewStage);
	int     Evaluate();
	bool    CutOffTest(int depth);
	void    GetNextStage();
	void	delrepeatmv(MV repeatmv);
	bool    GetLastMove(SHistory& LMStore);
	void	updateMoveCount(int bInc);
	Search* searchMax(void);
	Search* searchMin(void);
	Search* DeleteNode(Search* pNode);

};
