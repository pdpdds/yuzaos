#include "Search.h"

#include <string.h>
#include <iostream>
#include <time.h>
#include <vector>
#include "HistoryStack.h"

#define SEARCH_MAIN 1
#include <stdbool.h>
#include "search.h"
#include "rc4.hpp"


int SCORE_CHO_CHA = 35100;
int SCORE_HAN_CHA = 35100;

using namespace std;

typedef struct tagHASH_ENTRY {
    int depth;
    int flag;
    int score;
    int from_x;
    int from_y;
    int alpha;
    int beta;
    long zobristLock;
} HASHENT;

const char* row_header = "abcdefghi";
static RC4  rc4;
static long PreGen_zobristKeyPlayer;
static long PreGen_zobristLockPlayer;
static long PreGen_zobristKeyTable[14][256];
static long PreGen_zobristLockTable[14][256];
static int  g_hashMask;
static HASHENT* g_hashTable;

static int  g_nMinimumDepth;
static int  g_entryLimit;
static int  g_maxLimit;
static int  g_gamePhase;
static int	g_jolCnts;		// 게임중후반 판단을 위해

int Search::sm_nTotalNode = 0;
int Search::sm_nCutOffDepth = 0;
int Search::sm_nFeedOverDepth = 0;
CHistoryStack Search::sm_cHistoryStack;

PIECE CHAR_TO_TEAM_PIECE(char c) {
    if (c == 'J') return PS_CHO_JANG;
    else if (c == 'A') return PS_CHO_SA;
    else if (c == 'S') return PS_CHO_SANG;
    else if (c == 'M') return PS_CHO_MA;
    else if (c == 'C') return PS_CHO_CHA;
    else if (c == 'P') return PS_CHO_PO;
    else if (c == 'B') return PS_CHO_JOL;
    else if (c == 'b') return PS_HAN_JOL;
    else if (c == 'a') return PS_HAN_SA;
    else if (c == 's') return PS_HAN_SANG;
    else if (c == 'm') return PS_HAN_MA;
    else if (c == 'c') return PS_HAN_CHA;
    else if (c == 'p') return PS_HAN_PO;
    else if (c == 'j') return PS_HAN_JANG;

    return PS_NOTHING;
}

char PIECE_TO_CHAR(PIECE pc) {
    if (pc == PS_CHO_JANG) return 'J';
    else if (pc == PS_CHO_SA) return 'A';
    else if (pc == PS_CHO_SANG) return 'S';
    else if (pc == PS_CHO_MA) return 'M';
    else if (pc == PS_CHO_CHA) return 'C';
    else if (pc == PS_CHO_PO) return 'P';
    else if (pc == PS_CHO_JOL) return 'B';
    else if (pc == PS_HAN_JANG) return 'j';
    else if (pc == PS_HAN_SA) return 'a';
    else if (pc == PS_HAN_SANG) return 's';
    else if (pc == PS_HAN_MA) return 'm';
    else if (pc == PS_HAN_CHA) return 'c';
    else if (pc == PS_HAN_PO) return 'p';
    else if (pc == PS_HAN_JOL) return 'b';

    return ' ';
}

int probeHash(int zobristKey, int zobristLock, int alpha, int beta, int depth)
{
    int idx = zobristKey & g_hashMask;

    if (g_hashTable[idx].zobristLock != zobristLock)
        return -DF_INFINITY;

    if (g_hashTable[idx].depth < depth)
        return -DF_INFINITY;

    if (g_hashTable[idx].flag == HASH_ALPHA)	// Alpha-cutoff이 발생했던 entry
        return (g_hashTable[idx].score <= alpha) ? g_hashTable[idx].score : -DF_INFINITY;

    if (g_hashTable[idx].flag == HASH_BETA)		// Beta-cutoff이 발생했던 entry
        return (g_hashTable[idx].score >= beta) ? g_hashTable[idx].score : -DF_INFINITY;

    if (g_hashTable[idx].alpha > alpha)
        if (g_hashTable[idx].score <= g_hashTable[idx].alpha)
            return -DF_INFINITY;

    if (g_hashTable[idx].beta < beta)
        if (g_hashTable[idx].score >= g_hashTable[idx].beta)
            return -DF_INFINITY;

    return g_hashTable[idx].score;
}

void recordHash(int zobristKey, int zobristLock, int flag, int score, int depth, int alpha, int beta)
{
    int idx = zobristKey & g_hashMask;

    g_hashTable[idx].flag = flag;
    g_hashTable[idx].depth = depth;
    g_hashTable[idx].zobristLock = zobristLock;
    g_hashTable[idx].score = score;
    g_hashTable[idx].alpha = alpha;
    g_hashTable[idx].beta = beta;
}


Search::Search() {
    Initialize();
}

Search::Search(Search* pParentStage)
{
    m_pParent = pParentStage;
    m_nNodeCount = 0;
    m_pFirst = NULL;
    m_pLast = NULL;
    m_enSide = pParentStage->m_enSide == SD_HAN ? SD_CHO : SD_HAN;
    m_bAmIMax = !pParentStage->IsMaxTurn();
    ::memcpy(m_enPanel, pParentStage->m_enPanel, sizeof(m_enPanel));

    zobristKey = pParentStage->zobristKey ^ PreGen_zobristKeyPlayer;
    zobristLock = pParentStage->zobristLock ^ PreGen_zobristLockPlayer;
}

void Search::Initialize(void) {
    m_pParent = NULL;
    m_nNodeCount = 0;
    m_pFirst = NULL;

    ClearList();

    ::memset(m_enPanel, PS_NOTHING, sizeof(m_enPanel));

    zobristKey = 0;
    zobristLock = 0;
}

void Search::defaultSetting(void) {
    AddPiece(0, 0, PS_HAN_CHA, ADD);
    AddPiece(1, 0, PS_HAN_MA, ADD);
    AddPiece(2, 0, PS_HAN_SANG, ADD);
    AddPiece(3, 0, PS_HAN_SA, ADD);
    AddPiece(5, 0, PS_HAN_SA, ADD);
    AddPiece(6, 0, PS_HAN_SANG, ADD);
    AddPiece(7, 0, PS_HAN_MA, ADD);
    AddPiece(8, 0, PS_HAN_CHA, ADD);
    AddPiece(4, 1, PS_HAN_JANG, ADD);
    AddPiece(1, 2, PS_HAN_PO, ADD);
    AddPiece(7, 2, PS_HAN_PO, ADD);
    AddPiece(0, 3, PS_HAN_JOL, ADD);
    AddPiece(2, 3, PS_HAN_JOL, ADD);
    AddPiece(4, 3, PS_HAN_JOL, ADD);
    AddPiece(6, 3, PS_HAN_JOL, ADD);
    AddPiece(8, 3, PS_HAN_JOL, ADD);

    AddPiece(0, 9, PS_CHO_CHA, ADD);
    AddPiece(1, 9, PS_CHO_SANG, ADD);
    AddPiece(2, 9, PS_CHO_MA, ADD);
    AddPiece(3, 9, PS_CHO_SA, ADD);
    AddPiece(5, 9, PS_CHO_SA, ADD);
    AddPiece(6, 9, PS_CHO_SANG, ADD);
    AddPiece(7, 9, PS_CHO_MA, ADD);
    AddPiece(8, 9, PS_CHO_CHA, ADD);
    AddPiece(4, 8, PS_CHO_JANG, ADD);
    AddPiece(1, 7, PS_CHO_PO, ADD);
    AddPiece(7, 7, PS_CHO_PO, ADD);
    AddPiece(0, 6, PS_CHO_JOL, ADD);
    AddPiece(2, 6, PS_CHO_JOL, ADD);
    AddPiece(4, 6, PS_CHO_JOL, ADD);
    AddPiece(6, 6, PS_CHO_JOL, ADD);
    AddPiece(8, 6, PS_CHO_JOL, ADD);
}

void Search::AddPiece(int x, int y, PIECE pc, bool bDel) { // Internal Function
    int pcAdjust = (pc < PS_BOUNDARY) ? pc - 1 : pc - 2;

    if (bDel)
        m_enPanel[y][x] = PS_NOTHING;
    else
        m_enPanel[y][x] = pc;

    int sq = MAKE_SQ(x, y);
    zobristKey ^= PreGen_zobristKeyTable[pcAdjust][sq];
    zobristLock ^= PreGen_zobristLockTable[pcAdjust][sq];
}

void Search::ClearList()
{
    if (m_pFirst)
    {
        Search* pNode = m_pFirst;
        Search* pTmp;
        do
        {
            pTmp = pNode->m_pNext;
            delete pNode;
            pNode = pTmp;
        } while (pNode != NULL);
        m_pFirst = NULL;
        m_pLast = NULL;
        m_nNodeCount = 0;
    }
}

void Search::print(void) {
    cout << "  ";
    for (int i = 0; i < DF_PANEL_WIDTH; ++i)
        cout << " " << row_header[i];
    cout << "\n\n";

    for (int r = 0; r < DF_PANEL_HEIGHT; ++r) {
        cout << r % 10 << "  ";
        for (int c = 0; c < DF_PANEL_WIDTH; c++) {
            if (m_enPanel[r][c] > 0)
                cout << PIECE_TO_CHAR(m_enPanel[r][c]) << " ";
            else
                cout << ". ";
        }
        cout << "  " << r % 10;
        cout << "\n";
    }

   // cout << "\n";
   // cout << "  ";
   // for (int i = 0; i < DF_PANEL_WIDTH; ++i)
      //  cout << " " << row_header[i];

    //cout << "\n";
}

bool Search::MovePiece(int nFromX, int nFromY, int nToX, int nToY) {
    if (nFromX < 0 || nFromX >= DF_PANEL_WIDTH) return false;
    if (nFromY < 0 || nFromY >= DF_PANEL_HEIGHT) return false;
    if (nToX < 0 || nToX >= DF_PANEL_WIDTH) return false;
    if (nToY < 0 || nToY >= DF_PANEL_HEIGHT) return false;

    if (m_enPanel[nToY][nToX] != PS_NOTHING)
        AddPiece(nToX, nToY, m_enPanel[nToY][nToX], DEL);

    AddPiece(nToX, nToY, m_enPanel[nFromY][nFromX], ADD);
    AddPiece(nFromX, nFromY, m_enPanel[nFromY][nFromX], DEL);

    return true;
}

int Search::MakeMove(int from_x, int from_y, int to_x, int to_y)
{
    PIECE enEatenPiece = m_enPanel[to_y][to_x];  // 목표 경로에 놓여있는 기물

    if (!MovePiece(from_x, from_y, to_x, to_y))
        return -3;

    if (enEatenPiece == PS_CHO_CHA)		// CHA를 하나 잃으면 기물평가값을 높인다.
        SCORE_CHO_CHA = 40000;
    else if (enEatenPiece == PS_HAN_CHA)
        SCORE_HAN_CHA = 40000;

    if (enEatenPiece == PS_CHO_JOL ||
        enEatenPiece == PS_HAN_JOL)
        g_jolCnts--;

    // Save History
    sm_cHistoryStack.Push(from_x, from_y, to_x, to_y, enEatenPiece);

    return 1;
}

void Search::AddNextStageEffectively(Search* pNewStage)
{
    int nNewScore = pNewStage->Evaluate();

    ///////////////////////////////////////////////////////
    // Nodes which are placed in same level must be sorted
    // to make Alpha-Beta algorithm effeciently.
    // When it's in Max turn, it must be sorted ascendently
    // in case of Min,Decendently.
    if (m_bAmIMax)
    { // MAX값을 취해야할 유저인가
        if (m_pFirst)
        { // 리스트에 기존 추가된 노드가 있다면
            // 평가값의 오름차순으로 정렬
            Search* pNode = m_pFirst;
            do
            {
                if (nNewScore == pNode->m_nScore) {  // 국면평가값이 동일하면
                    if (pNewStage->m_enPanel[pNewStage->m_nToY][pNewStage->m_nToX] >
                        pNode->m_enPanel[pNode->m_nToY][pNode->m_nToX])  // 기동성이 높은 기물이 먼저 추가됨
                        break;
                }
                else if (nNewScore > pNode->m_nScore) break;
                pNode = pNode->m_pNext;
            } while (pNode != NULL);

            if (pNode == NULL)  // 노드리스트에서 마지막
            {
                m_pLast->m_pNext = pNewStage;
                pNewStage->m_pPrev = m_pLast;
                pNewStage->m_pNext = NULL;
                m_pLast = pNewStage;
            }
            else              // 노드리스트에서 중간에 끼워넣음
            {
                Search* pTmp = pNode->m_pPrev;
                pNode->m_pPrev = pNewStage;
                pNewStage->m_pNext = pNode;
                pNewStage->m_pPrev = pTmp;
                if (pTmp)
                {
                    pTmp->m_pNext = pNewStage;
                }
                else
                {
                    m_pFirst = pNewStage;
                }
            }
        }
        else
        { // 노드리스트에 노드가 없다면 처음 노드에 추가
            m_pFirst = pNewStage;
            m_pLast = pNewStage;
            pNewStage->m_pPrev = NULL;
            pNewStage->m_pNext = NULL;
        }
    }
    else
    { // MIN값을 취해야할 유저
        if (m_pFirst)
        {
            // 평가값의 내림차순으로 정렬
            Search* pNode = m_pFirst;
            do
            {
                if (nNewScore == pNode->m_nScore) { // 국면평가값이 동일하면
                    if (pNewStage->m_enPanel[pNewStage->m_nToY][pNewStage->m_nToX] >
                        pNode->m_enPanel[pNode->m_nToY][pNode->m_nToX])  // 기동성이 높은 기물이 먼저 추가됨
                        break;
                }
                else if (nNewScore < pNode->m_nScore) break;
                pNode = pNode->m_pNext;
            } while (pNode != NULL);

            if (pNode == NULL)
            {
                m_pLast->m_pNext = pNewStage;
                pNewStage->m_pPrev = m_pLast;
                pNewStage->m_pNext = NULL;
                m_pLast = pNewStage;

            }
            else
            {
                Search* pTmp = pNode->m_pPrev;
                pNode->m_pPrev = pNewStage;
                pNewStage->m_pNext = pNode;
                pNewStage->m_pPrev = pTmp;
                if (pTmp)
                {
                    pTmp->m_pNext = pNewStage;
                }
                else
                {
                    m_pFirst = pNewStage;
                }
            }
        }
        else
        {
            m_pFirst = pNewStage;
            m_pLast = pNewStage;
            pNewStage->m_pPrev = NULL;
            pNewStage->m_pNext = NULL;
        }
    }
    ++m_nNodeCount;
}

bool Search::TryToAddMovableXY(int nFromX, int nFromY, int nToX, int nToY)
{
    if (nToX < 0) return false;
    if (nToY < 0) return false;
    if (nToX >= DF_PANEL_WIDTH) return false;
    if (nToY >= DF_PANEL_HEIGHT) return false;

    // Impossible to move to a location where other piece of
    // same team exist.
    if (GetSideAt(nToX, nToY) == GetSideAt(nFromX, nFromY)) return false;

    Search* pNewStage = new Search(this);
    pNewStage->MovePiece(nFromX, nFromY, nToX, nToY);

    pNewStage->m_nFromX = nFromX;
    pNewStage->m_nFromY = nFromY;
    pNewStage->m_nToX = nToX;
    pNewStage->m_nToY = nToY;
    pNewStage->m_enEatenPiece = m_enPanel[nToY][nToX];

    ++sm_nTotalNode;

    AddNextStageEffectively(pNewStage);

    return true;
}

SIDE Search::GetSideAt(int nX, int nY)
{
    if (m_enPanel[nY][nX] == PS_NOTHING) return SD_NOTHING;
    if (m_enPanel[nY][nX] < PS_BOUNDARY) return SD_CHO;

    return SD_HAN;
}

void Search::GetMovableSaAndJang(int nFromX, int nFromY)
{
    switch (nFromY)
    {
    case 0:
    case 7:
        switch (nFromX)
        {
        case 3:
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY);
            TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY + 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY + 1);
            break;
        case 4:
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY);
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY);
            TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY + 1);
            break;
        case 5:
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY);
            TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY + 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY + 1);
            break;
        }
        break;
    case 1:
    case 8:
        switch (nFromX)
        {
        case 3:
            TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY - 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY + 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY);
            break;
        case 4:
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY);
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY - 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY - 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY - 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY);
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY + 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY + 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY + 1);
            break;
        case 5:
            TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY - 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY + 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY);
            break;
        }
        break;
    case 2:
    case 9:
        switch (nFromX)
        {
        case 3:
            TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY - 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY - 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY);
            break;
        case 4:
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY);
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY);
            TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY - 1);
            break;
        case 5:
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY);
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY - 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY - 1);
            break;
        }
        break;
    }
}

void Search::GetMovableJol(int nFromX, int nFromY)
{
    int nDirection = GetSideAt(nFromX, nFromY) == SD_CHO ? -1 : 1;

    //////////////////////////////////////////////////
    // JOL also can move diagonaly in the area of King
    switch (nFromY)
    {
    case 1:
    case 8:
        switch (nFromX)
        {
        case 4:
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY + nDirection);
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY + nDirection);
            break;
        }
        break;
    case 2:
    case 7:
        switch (nFromX)
        {
        case 3:
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY + nDirection);
            break;
        case 5:
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY + nDirection);
            break;
        }
        break;
    }

    // Tru to move to left
    TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY);
    // Try to move to right
    TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY);
    // Try to move forward
    TryToAddMovableXY(nFromX, nFromY, nFromX, nFromY + nDirection);
}

void Search::GetMovablePo(int nFromX, int nFromY)
{
    bool bMovable = false;
    int	 k;
    // Try Right side line
    for (k = nFromX + 1; k < DF_PANEL_WIDTH; ++k)
    {
        if (bMovable)
        {
            if (m_enPanel[nFromY][k] != PS_CHO_PO &&
                m_enPanel[nFromY][k] != PS_HAN_PO)
            {
                TryToAddMovableXY(nFromX, nFromY, k, nFromY);
                if (m_enPanel[nFromY][k] != PS_NOTHING)
                    break;
            }
            else
            {
                break;
            }
        }
        else
        {
            ////////////////////////////////////////////////////////
            // PO only can move over the other piece except other PO
            if (m_enPanel[nFromY][k] == PS_HAN_PO)
                break;
            if (m_enPanel[nFromY][k] == PS_CHO_PO)
                break;
            if (m_enPanel[nFromY][k] != PS_NOTHING)
                bMovable = true;
        }
    }

    bMovable = false;
    // Try left side line
    for (k = nFromX - 1; k >= 0; --k)
    {
        if (bMovable)
        {
            if (m_enPanel[nFromY][k] != PS_CHO_PO &&
                m_enPanel[nFromY][k] != PS_HAN_PO)
            {
                TryToAddMovableXY(nFromX, nFromY, k, nFromY);
                if (m_enPanel[nFromY][k] != PS_NOTHING)
                    break;
            }
            else
            {
                break;
            }
        }
        else
        {
            ////////////////////////////////////////////////////////
            // PO only can move over the other piece except other PO
            if (m_enPanel[nFromY][k] == PS_HAN_PO)
                break;
            if (m_enPanel[nFromY][k] == PS_CHO_PO)
                break;
            if (m_enPanel[nFromY][k] != PS_NOTHING)
                bMovable = true;
        }
    }
    bMovable = false;
    // Try upper line
    for (k = nFromY - 1; k >= 0; --k)
    {
        if (bMovable)
        {
            if (m_enPanel[k][nFromX] != PS_CHO_PO &&
                m_enPanel[k][nFromX] != PS_HAN_PO)
            {
                TryToAddMovableXY(nFromX, nFromY, nFromX, k);
                if (m_enPanel[k][nFromX] != PS_NOTHING)
                    break;
            }
            else
            {
                break;
            }
        }
        else
        {
            ////////////////////////////////////////////////////////
            // PO only can move over the other piece except other PO
            if (m_enPanel[k][nFromX] == PS_HAN_PO)
                break;
            if (m_enPanel[k][nFromX] == PS_CHO_PO)
                break;
            if (m_enPanel[k][nFromX] != PS_NOTHING)
                bMovable = true;
        }
    }
    bMovable = false;
    // Try under line
    for (k = nFromY + 1; k < DF_PANEL_HEIGHT; ++k)
    {
        if (bMovable)
        {
            if (m_enPanel[k][nFromX] != PS_CHO_PO &&
                m_enPanel[k][nFromX] != PS_HAN_PO)
            {
                TryToAddMovableXY(nFromX, nFromY, nFromX, k);
                if (m_enPanel[k][nFromX] != PS_NOTHING)
                    break;
            }
            else
            {
                break;
            }
        }
        else
        {
            ////////////////////////////////////////////////////////
            // PO only can move over the other piece except other PO
            if (m_enPanel[k][nFromX] == PS_HAN_PO)
                break;
            if (m_enPanel[k][nFromX] == PS_CHO_PO)
                break;
            if (m_enPanel[k][nFromX] != PS_NOTHING)
                bMovable = true;
        }
    }
    bMovable = false;
    // 궁안에 있다면
    switch (nFromX)
    {
    case 3:
        if (nFromY == 2 || nFromY == 9)
        {
            if (m_enPanel[nFromY - 1][nFromX + 1] != PS_CHO_PO &&
                m_enPanel[nFromY - 1][nFromX + 1] != PS_HAN_PO &&
                m_enPanel[nFromY - 1][nFromX + 1] != PS_NOTHING)
            {
                if (m_enPanel[nFromY + 2][nFromX - 2] == PS_HAN_PO ||
                    m_enPanel[nFromY + 2][nFromX - 2] == PS_CHO_PO)
                    ;
                else
                    TryToAddMovableXY(nFromX, nFromY, nFromX + 2, nFromY - 2);
            }
        }
        else if (nFromY == 0 || nFromY == 7)
        {
            if (m_enPanel[nFromY + 1][nFromX + 1] != PS_CHO_PO &&
                m_enPanel[nFromY + 1][nFromX + 1] != PS_HAN_PO &&
                m_enPanel[nFromY + 1][nFromX + 1] != PS_NOTHING)
            {
                if (m_enPanel[nFromY + 2][nFromX + 2] == PS_HAN_PO ||
                    m_enPanel[nFromY + 2][nFromX + 2] == PS_CHO_PO)
                    ;
                else
                    TryToAddMovableXY(nFromX, nFromY, nFromX + 2, nFromY + 2);
            }
        }
        break;

    case 5:
        if (nFromY == 2 || nFromY == 9)
        {
            if (m_enPanel[nFromY - 1][nFromX - 1] != PS_CHO_PO &&
                m_enPanel[nFromY - 1][nFromX - 1] != PS_HAN_PO &&
                m_enPanel[nFromY - 1][nFromX - 1] != PS_NOTHING)
            {
                if (m_enPanel[nFromY - 2][nFromX - 2] == PS_HAN_PO ||
                    m_enPanel[nFromY - 2][nFromX - 2] == PS_CHO_PO)
                    ;
                else
                    TryToAddMovableXY(nFromX, nFromY, nFromX - 2, nFromY - 2);
            }
        }
        else if (nFromY == 0 || nFromY == 7)
        {
            if (m_enPanel[nFromY + 1][nFromX - 1] != PS_CHO_PO &&
                m_enPanel[nFromY + 1][nFromX - 1] != PS_HAN_PO &&
                m_enPanel[nFromY + 1][nFromX - 1] != PS_NOTHING)
            {
                if (m_enPanel[nFromY - 2][nFromX + 2] == PS_HAN_PO ||
                    m_enPanel[nFromY - 2][nFromX + 2] == PS_CHO_PO)
                    ;
                else
                    TryToAddMovableXY(nFromX, nFromY, nFromX - 2, nFromY + 2);
            }
        }
        break;
    }
}

void Search::GetMovableCha(int nFromX, int nFromY)
{
    int k;

    //////////////////////////////////////////////////
    // CHA also can move diagonaly in the area of King
    switch (nFromY)
    {
    case 0:
    case 7:
        switch (nFromX)
        {
        case 3:
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY + 1);
            if (GetSideAt(nFromX + 1, nFromY + 1) == SD_NOTHING)
                TryToAddMovableXY(nFromX, nFromY, nFromX + 2, nFromY + 2);
            break;
        case 5:
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY + 1);
            if (GetSideAt(nFromX - 1, nFromY + 1) == SD_NOTHING)
                TryToAddMovableXY(nFromX, nFromY, nFromX - 2, nFromY + 2);
            break;
        }
        break;
    case 1:
    case 8:
        switch (nFromX)
        {
        case 4:
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY - 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY - 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY + 1);
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY + 1);
            break;
        }
        break;
    case 2:
    case 9:
        switch (nFromX)
        {
        case 3:
            TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY - 1);
            if (GetSideAt(nFromX + 1, nFromY - 1) == SD_NOTHING)
                TryToAddMovableXY(nFromX, nFromY, nFromX + 2, nFromY - 2);
            break;
        case 5:
            TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY - 1);
            if (GetSideAt(nFromX - 1, nFromY - 1) == SD_NOTHING)
                TryToAddMovableXY(nFromX, nFromY, nFromX - 2, nFromY - 2);
            break;
        }
        break;
    }
    // try Right line
    for (k = nFromX + 1; k < DF_PANEL_WIDTH; ++k)
    {
        // Stop if other piece of same team exist
        if (GetSideAt(k, nFromY) == GetSideAt(nFromX, nFromY))
            break;
        TryToAddMovableXY(nFromX, nFromY, k, nFromY);
        if (GetSideAt(k, nFromY) != SD_NOTHING)
            break;
    }
    // try Left line
    for (k = nFromX - 1; k >= 0; --k)
    {
        if (GetSideAt(k, nFromY) == GetSideAt(nFromX, nFromY))
            break;
        TryToAddMovableXY(nFromX, nFromY, k, nFromY);
        if (GetSideAt(k, nFromY) != SD_NOTHING)
            break;
    }

    // try Upper line
    for (k = nFromY - 1; k >= 0; --k)
    {
        if (GetSideAt(nFromX, k) == GetSideAt(nFromX, nFromY))
            break;
        TryToAddMovableXY(nFromX, nFromY, nFromX, k);
        if (GetSideAt(nFromX, k) != SD_NOTHING)
            break;
    }

    // Try under line
    for (k = nFromY + 1; k < DF_PANEL_HEIGHT; ++k)
    {
        if (GetSideAt(nFromX, k) == GetSideAt(nFromX, nFromY))
            break;
        TryToAddMovableXY(nFromX, nFromY, nFromX, k);
        if (GetSideAt(nFromX, k) != SD_NOTHING)
            break;
    }
}

void Search::GetMovableMa(int nFromX, int nFromY)
{
    // Left Upper
    if (IsPassable(nFromX, nFromY - 1))
        TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY - 2);
    // Right Upper
    if (IsPassable(nFromX, nFromY - 1))
        TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY - 2);
    // Left under
    if (IsPassable(nFromX, nFromY + 1))
        TryToAddMovableXY(nFromX, nFromY, nFromX - 1, nFromY + 2);
    // Right under
    if (IsPassable(nFromX, nFromY + 1))
        TryToAddMovableXY(nFromX, nFromY, nFromX + 1, nFromY + 2);
    // Left Upper
    if (IsPassable(nFromX - 1, nFromY))
        TryToAddMovableXY(nFromX, nFromY, nFromX - 2, nFromY - 1);
    // Left under
    if (IsPassable(nFromX - 1, nFromY))
        TryToAddMovableXY(nFromX, nFromY, nFromX - 2, nFromY + 1);
    // Right upper
    if (IsPassable(nFromX + 1, nFromY))
        TryToAddMovableXY(nFromX, nFromY, nFromX + 2, nFromY - 1);
    // right under
    if (IsPassable(nFromX + 1, nFromY))
        TryToAddMovableXY(nFromX, nFromY, nFromX + 2, nFromY + 1);
}

void Search::GetMovableSang(int nFromX, int nFromY)
{
    // Upper & Left
    if (IsPassable(nFromX, nFromY - 1) && IsPassable(nFromX - 1, nFromY - 2))
        TryToAddMovableXY(nFromX, nFromY, nFromX - 2, nFromY - 3);

    // Upper & Right
    if (IsPassable(nFromX, nFromY - 1) && IsPassable(nFromX + 1, nFromY - 2))
        TryToAddMovableXY(nFromX, nFromY, nFromX + 2, nFromY - 3);

    // Under & Left
    if (IsPassable(nFromX, nFromY + 1) && IsPassable(nFromX - 1, nFromY + 2))
        TryToAddMovableXY(nFromX, nFromY, nFromX - 2, nFromY + 3);

    // Under & Right
    if (IsPassable(nFromX, nFromY + 1) && IsPassable(nFromX + 1, nFromY + 2))
        TryToAddMovableXY(nFromX, nFromY, nFromX + 2, nFromY + 3);

    // Left & Upper
    if (IsPassable(nFromX - 1, nFromY) && IsPassable(nFromX - 2, nFromY - 1))
        TryToAddMovableXY(nFromX, nFromY, nFromX - 3, nFromY - 2);

    // Left & Under
    if (IsPassable(nFromX - 1, nFromY) && IsPassable(nFromX - 2, nFromY + 1))
        TryToAddMovableXY(nFromX, nFromY, nFromX - 3, nFromY + 2);

    // Right & Upper
    if (IsPassable(nFromX + 1, nFromY) && IsPassable(nFromX + 2, nFromY - 1))
        TryToAddMovableXY(nFromX, nFromY, nFromX + 3, nFromY - 2);

    // Right & Under
    if (IsPassable(nFromX + 1, nFromY) && IsPassable(nFromX + 2, nFromY + 1))
        TryToAddMovableXY(nFromX, nFromY, nFromX + 3, nFromY + 2);
}

// Check whether piece can pass the place located
bool Search::IsPassable(int nX, int nY)
{
    if (nX < 0) return false;
    if (nY < 0) return false;
    if (nX >= DF_PANEL_WIDTH) return false;
    if (nY >= DF_PANEL_HEIGHT) return false;

    if (m_enPanel[nY][nX] == PS_NOTHING)
        return true;
    else
        return false;
}

int Search::Evaluate()
{
    // 기물들이 어떠한 포진을 하고 있는지에 따른
    // 포지션 평가
    m_nScore = 0;
    for (int i, j = 0; j < DF_PANEL_HEIGHT; ++j)
        for (i = 0; i < DF_PANEL_WIDTH; ++i)
        {
            switch (m_enPanel[j][i])
            {
            case PS_CHO_JANG:
            {
                m_nScore -= DF_SCORE_JANG;
                // An inclination to place PO in front of JANG
                if (g_bPoBeforeJang)
                {
                    if (j == 8 && m_enPanel[j - 1][i] == PS_CHO_PO)
                        m_nScore -= 50;
                }

                if (j == 7)     // JANG의 천궁은 회피하도록
                    m_nScore += 2;

                break;
            }
            case PS_CHO_SA: m_nScore -= DF_SCORE_SA; break;
            case PS_CHO_JOL:
            {
                m_nScore -= DF_SCORE_JOL;
                // Gather JOL to center
                if (g_bGatherJol)
                {
                    if (i > 0 && m_enPanel[j][i - 1] == PS_CHO_JOL)
                        m_nScore -= 1;
                    if (i < DF_PANEL_WIDTH - 1 &&
                        m_enPanel[j][i + 1] == PS_CHO_JOL)
                        m_nScore -= 1;
                }
                // Make JOL aggressive
                if (g_bAggressiveJOL)
                {
                    if (j < 6)
                        m_nScore -= 1;

                    if (j < 4) {
                        if (i > 2 && i < 6)
                            m_nScore -= 2;
                    }
                }

                // 모서리에 있으면 패널티
                if (::g_gamePhase == PHASE_OPENING)
                    if (i == 0 || i == DF_PANEL_WIDTH - 1)
                        m_nScore += 1;

                break;
            }
            case PS_CHO_PO:
            {
                m_nScore -= DF_SCORE_PO;
                if (g_bProtectGung)
                {
                    if (j == 7 &&
                        i >= 3 && i <= 5)
                        m_nScore -= 2;

                    if (j == 8 && i == 4)	// 궁가운데를 차지하면 패널티
                        m_nScore += 10;

                    if (::g_gamePhase == PHASE_OPENING)
                    {
                        if (j < 3)		// 게임초기(openning)에 PO가 나가있으면 패널티
                            m_nScore += 2;
                    }
                }

                break;
            }
            case PS_CHO_CHA:
            {
                m_nScore -= SCORE_CHO_CHA;
                // Make CHA Aggressive
                if (g_bAggressiveCHA)
                {
                    if (j < 6)
                        m_nScore -= 2;
                }
                break;
            }
            case PS_CHO_MA:
            {
                m_nScore -= DF_SCORE_MA;
                // Make MA aggressive
                if (g_bAggressiveMA)
                {
                    if (j < 5)
                        m_nScore -= 1;
                }

                // Positional value
                if (j == DF_PANEL_HEIGHT - 1)
                    m_nScore += 1;
                if (i == 0 || i == DF_PANEL_WIDTH - 1)
                    m_nScore += 1;

                break;
            }
            case PS_CHO_SANG:
            {
                m_nScore -= DF_SCORE_SANG;
                // Make SANG aggressive
                if (g_bAggressiveSANG)
                {
                    if (j < 6)
                        m_nScore -= 1;
                }
                break;
            }
            case PS_HAN_JANG:
            {
                m_nScore += DF_SCORE_JANG;
                // An inclination to place PO in front of JANG
                if (g_bPoBeforeJang)
                {
                    if (j == 1 && m_enPanel[j + 1][i] == PS_HAN_PO)
                        m_nScore += 50;
                }

                if (j == 2)  // JANG의 천궁은 회피하도록
                    m_nScore -= 2;

                break;
            }
            case PS_HAN_SA: m_nScore += DF_SCORE_SA; break;
            case PS_HAN_JOL:
            {
                m_nScore += DF_SCORE_JOL;
                // Gather JOL to center
                if (g_bGatherJol)
                {
                    if (i > 0 && m_enPanel[j][i - 1] == PS_HAN_JOL)
                        m_nScore += 1;
                    if (i < DF_PANEL_WIDTH - 1 && m_enPanel[j][i + 1] == PS_HAN_JOL)
                        m_nScore += 1;
                }
                // Aggressive JOL
                if (g_bAggressiveJOL)
                {
                    if (j > 3)
                        m_nScore += 1;

                    if (j > 5) {
                        if (i > 2 && i < 6)
                            m_nScore += 2;
                    }
                }

                // 모서리에 있으면 패널티
                if (::g_gamePhase == PHASE_OPENING)
                    if (i == 0 || i == DF_PANEL_WIDTH - 1)
                        m_nScore -= 1;

                break;
            }
            case PS_HAN_PO:
            {
                m_nScore += DF_SCORE_PO;
                // Aggressive PO
                if (g_bAggressivePO)
                {
                    if (j > 3)
                        m_nScore += 1;
                }

                if (g_bProtectGung)
                {
                    if (j == 2 &&
                        i >= 3 && i <= 5)
                        m_nScore += 2;
                    if (j == 1 && i == 4)	// 궁가운데를 차지하면 패널티
                        m_nScore -= 10;

                    if (::g_gamePhase == PHASE_OPENING)
                    {
                        if (j > 3)      // 게임초기(openning)에 PO가 나가있으면 패널티
                            m_nScore -= 2;
                    }
                }

                break;
            }
            case PS_HAN_CHA:
            {
                m_nScore += SCORE_HAN_CHA;
                // Aggressive CHA
                if (g_bAggressiveCHA)
                {
                    if (j > 3)
                        m_nScore += 2;
                }
                break;
            }
            case PS_HAN_MA:
            {
                m_nScore += DF_SCORE_MA;
                // Aggressive MA
                if (g_bAggressiveMA)
                {
                    if (j > 4)
                        m_nScore += 1;
                }

                // Positional value
                if (j == 0)
                    m_nScore -= 1;
                if (i == 0 || i == DF_PANEL_WIDTH - 1)
                    m_nScore -= 1;

                break;
            }
            case PS_HAN_SANG:
            {
                m_nScore += DF_SCORE_SANG;
                // Aggressive SANG
                if (g_bAggressiveSANG)
                {
                    if (j > 3)
                        m_nScore += 1;
                }
                break;
            }
            }
        }

    // Block up other pieces except SA and MA to come to the center of the area of JANG
    if (m_enPanel[1][4] != PS_NOTHING && m_enPanel[1][4] != PS_HAN_JANG && m_enPanel[1][4] != PS_HAN_SA)
        m_nScore -= 10;
    if (m_enPanel[8][4] != PS_NOTHING && m_enPanel[8][4] != PS_CHO_JANG && m_enPanel[8][4] != PS_CHO_SA)
        m_nScore += 10;

    return m_nScore;
}

bool Search::GetNextPieceLoc(int nFromX, int nFromY, CMoveData* cMD)
{
    Search cStage;
    ::memcpy(cStage.m_enPanel, m_enPanel, sizeof(m_enPanel));

    /////////////////////////////////////////////////////
    // 각 말 별로 움직일 수 있는 범위를 구한다.
    switch (cStage.m_enPanel[nFromY][nFromX])
    {
    case PS_CHO_JANG:
    case PS_CHO_SA: cStage.GetMovableSaAndJang(nFromX, nFromY); break;
    case PS_CHO_JOL: cStage.GetMovableJol(nFromX, nFromY); break;
    case PS_CHO_PO: cStage.GetMovablePo(nFromX, nFromY); break;
    case PS_CHO_CHA: cStage.GetMovableCha(nFromX, nFromY); break;
    case PS_CHO_MA: cStage.GetMovableMa(nFromX, nFromY); break;
    case PS_CHO_SANG: cStage.GetMovableSang(nFromX, nFromY); break;
    case PS_HAN_JANG:
    case PS_HAN_SA: cStage.GetMovableSaAndJang(nFromX, nFromY); break;
    case PS_HAN_JOL: cStage.GetMovableJol(nFromX, nFromY); break;
    case PS_HAN_PO: cStage.GetMovablePo(nFromX, nFromY); break;
    case PS_HAN_CHA: cStage.GetMovableCha(nFromX, nFromY); break;
    case PS_HAN_MA: cStage.GetMovableMa(nFromX, nFromY); break;
    case PS_HAN_SANG: cStage.GetMovableSang(nFromX, nFromY); break;
    }

    cMD->SetFromXY(nFromX, nFromY);

    // 움직일 수 있는 범위의 노드리스트가 있다면
    if (cStage.m_pFirst)
    {
        Search* pNode = cStage.m_pFirst;
        do
        {
            cMD->AddToXY(pNode->m_nToX, pNode->m_nToY);
            pNode = pNode->m_pNext;
        } while (pNode != NULL);

        return true;
    }

    return true;
}

void Search::GetNextStage()
{
    if (m_enSide == SD_HAN)
    {
        for (int j, i = 0; i < DF_PANEL_HEIGHT; ++i)
        {
            for (j = 0; j < DF_PANEL_WIDTH; ++j)
            {
                switch (m_enPanel[i][j])
                {
                case PS_HAN_JANG:
                case PS_HAN_SA: GetMovableSaAndJang(j, i); break;
                case PS_HAN_JOL: GetMovableJol(j, i); break;
                case PS_HAN_PO: GetMovablePo(j, i); break;
                case PS_HAN_CHA: GetMovableCha(j, i); break;
                case PS_HAN_MA: GetMovableMa(j, i); break;
                case PS_HAN_SANG: GetMovableSang(j, i); break;
                }
            }
        }
    }
    else
    {
        for (int j, i = 0; i < DF_PANEL_HEIGHT; ++i)
        {
            for (j = 0; j < DF_PANEL_WIDTH; ++j)
            {
                switch (m_enPanel[i][j])
                {
                case PS_CHO_JANG:
                case PS_CHO_SA: GetMovableSaAndJang(j, i); break;
                case PS_CHO_JOL: GetMovableJol(j, i); break;
                case PS_CHO_PO: GetMovablePo(j, i); break;
                case PS_CHO_CHA: GetMovableCha(j, i); break;
                case PS_CHO_MA: GetMovableMa(j, i); break;
                case PS_CHO_SANG: GetMovableSang(j, i); break;
                }
            }
        }
    }
}

bool Search::CutOffTest(int depth)
{
    if (m_enEatenPiece == PS_CHO_JANG)
        return true;
    if (m_enEatenPiece == PS_HAN_JANG)
        return true;

    if (depth < sm_nCutOffDepth) return false;

    if (m_enEatenPiece == PS_NOTHING)
        return true;

    if (depth >= sm_nFeedOverDepth) return true;

    return false;
}

int Search::MaxValue(int nAlpha, int nBeta, int depth)
{
    int nValue = 0;
    int oalpha = nAlpha;
    int obeta = nBeta;

    // CUTOFF test
    if (CutOffTest(depth)) return m_nScore;

    // transposition table search
    nValue = ::probeHash(zobristKey, zobristLock, nAlpha, nBeta, sm_nCutOffDepth - depth);
    if (nValue > -DF_INFINITY)
    {
        return nValue;
    }

    updateMoveCount(ADD);
    GetNextStage();
    updateMoveCount(DEL);

    if (m_pFirst)
    {
        Search* pNode = m_pFirst;
        do
        {
            nValue = pNode->MinValue(nAlpha, nBeta, depth + 1);

            if (nValue > nAlpha)
            {
                nAlpha = nValue;
            }
            if (nAlpha >= nBeta)
            {
                ClearList();
                ::recordHash(zobristKey, zobristLock, HASH_BETA, nValue, sm_nCutOffDepth - depth, oalpha, obeta);
                return nBeta;
            }
            pNode = pNode->m_pNext;
        } while (pNode != NULL);
    }
    ClearList();

    ::recordHash(zobristKey, zobristLock, HASH_PV, nAlpha, sm_nCutOffDepth - depth, oalpha, obeta);

    return nAlpha;
}

int Search::MinValue(int nAlpha, int nBeta, int depth)
{
    int nValue = 0;
    int oalpha = nAlpha;
    int obeta = nBeta;
    Search* pSearchNode = NULL;

    // CUTOFF test
    if (CutOffTest(depth)) return m_nScore;

    // transposition table search
    nValue = ::probeHash(zobristKey, zobristLock, nAlpha, nBeta, sm_nCutOffDepth - depth);
    if (nValue > -DF_INFINITY)
    {
        return nValue;
    }

    updateMoveCount(ADD);
    GetNextStage();
    updateMoveCount(DEL);

    if (m_pFirst)
    {
        Search* pNode = m_pFirst;

        do
        {
            nValue = pNode->MaxValue(nAlpha, nBeta, depth + 1);
            if (nValue < nBeta)
            {
                nBeta = nValue;
            }
            if (nBeta <= nAlpha)
            {
                ClearList();
                ::recordHash(zobristKey, zobristLock, HASH_ALPHA, nValue, sm_nCutOffDepth - depth, oalpha, obeta);

                return nAlpha;
            }
            pNode = pNode->m_pNext;
        } while (pNode != NULL);
    }
    ClearList();

    ::recordHash(zobristKey, zobristLock, HASH_PV, nBeta, sm_nCutOffDepth - depth, oalpha, obeta);

    return nBeta;
}

Search* Search::DeleteNode(Search* pNode)
{
    if (pNode)
    {
        if (pNode->m_pPrev)
            pNode->m_pPrev->m_pNext = pNode->m_pNext;
        else
            m_pFirst = pNode->m_pNext;

        if (pNode->m_pNext)
            pNode->m_pNext->m_pPrev = pNode->m_pPrev;
        else
            m_pLast = pNode->m_pPrev;

        --m_nNodeCount;
    }
    return pNode;
}
MV Search::getBestMove(SIDE side, int repeatmv)
{
    m_bAmIMax = (side == SD_HAN);
    m_enSide = side;

    sm_nTotalNode = 0;

    Search* pSelected = NULL;

    ClearList();
    updateMoveCount(ADD);
    GetNextStage();
    updateMoveCount(DEL);

    sm_nCutOffDepth = 6 - (m_nNodeCount / 10);
    sm_nFeedOverDepth = 10 - (m_nNodeCount / 10);

    if (sm_nCutOffDepth < g_nMinimumDepth) sm_nCutOffDepth = g_nMinimumDepth;
    if (sm_nFeedOverDepth < (sm_nCutOffDepth + 5)) sm_nFeedOverDepth = sm_nCutOffDepth + 5;

    if (m_pFirst == NULL) return 0;

    if (repeatmv > 0) delrepeatmv(repeatmv);   // 반복수 제거

    if (m_bAmIMax)
        pSelected = searchMax();
    else
        pSelected = searchMin();

    if (pSelected == NULL) return 0;

    return MAKE_MV(
        MAKE_SQ(pSelected->m_nFromX, pSelected->m_nFromY),
        MAKE_SQ(pSelected->m_nToX, pSelected->m_nToY)
    );
}

Search* Search::searchMax(void)
{
    Search* pSelected = NULL;
    Search* pNode = m_pFirst;
    int alpha = 0, beta = 0;
    int bestidx = 0, bestscore = 0;
    int score = 0;
    int depth = 0;

    //cout << "Start Max value...." << "\n";

    clock_t c1 = clock();
    clock_t dt;

    alpha = pNode->MinValue(-DF_INFINITY, DF_INFINITY, depth);  // PVS
    beta = alpha + 1;			// Null window 설정
    pSelected = pNode;
    pNode = pNode->m_pNext;

    // 1st shallow search
    for (int i = 0; pNode != NULL; i++)
    {
        score = pNode->MinValue(alpha, beta, depth);

        if (score == beta) {	// 더 최적의 값이 나왔으므로 재탐색

            beta = DF_INFINITY;
            score = pNode->MinValue(score, beta, depth);
        }
        if (score > alpha)
        {
            alpha = score;
            pSelected = pNode;
            beta = alpha + 1;		// Null window
            bestidx = i + 1;    // 0번째 node는 for문 전에 평가했으므로
        }
        pNode->m_nScore = score;
        pNode = pNode->m_pNext;
    }

    //cout << "1st find best move " << row_header[pSelected->m_nFromX] << pSelected->m_nFromY
      //  << " -> " << row_header[pSelected->m_nToX] << pSelected->m_nToY << " id=" << bestidx << " alpha=" << alpha << "\n";
    bestscore = alpha;

    // Iterative deepening
    bool stopFlag = false;
    for (int cnt = 0; cnt < 4 && !stopFlag; cnt++)
    {
        clock_t c2 = (clock() - c1) / (CLOCKS_PER_SEC / 1000);
        if (c2 < g_entryLimit) {
            //cout << cnt + 2 << "th search..." << "\n";

            alpha = alpha - 100;

            score = pSelected->MinValue(alpha, DF_INFINITY, depth - (1 + cnt));

            if (score == alpha)		// fail-low 발생
            {						// MAX입장에선 안좋은 값이긴 하지만 초기값을 가져와야 하므로
                score = pSelected->MinValue(-DF_INFINITY, alpha, depth - (1 + cnt));
            }
            alpha = bestscore = score;

            beta = alpha + 1;
            if (bestidx != 0)	// 첫번째 node가 아니라면 앞에 동일값의 node가 있을 수 있으므로
                alpha -= 1;		//  앞의 node가 선택되도록

            pNode = m_pFirst;

            int selectid = bestidx;

            for (int i = 0; pNode != NULL; i++)
            {
                if (i == selectid) {
                    pNode = pNode->m_pNext;
                    continue;
                }

                score = pNode->MinValue(alpha, beta, depth - (1 + cnt));

                if (score == beta) {
                    beta = DF_INFINITY;
                    score = pNode->MinValue(score, beta, depth - (1 + cnt));
                }

                if (score > bestscore ||
                    (score == bestscore && i < bestidx))
                {
                    alpha = bestscore = score;
                    pSelected = pNode;
                    beta = alpha + 1;
                    bestidx = i;
                }
                pNode->m_nScore = score;
                pNode = pNode->m_pNext;

                dt = (clock() - c1) / (CLOCKS_PER_SEC / 1000);
                if (dt >= g_maxLimit) {
                    //cout << "TIME-OUT" << "\n";
                    stopFlag = true;
                    break;
                }
            }

            //cout << cnt + 2 << "th find best move " << row_header[pSelected->m_nFromX] << pSelected->m_nFromY
              //  << " -> " << row_header[pSelected->m_nToX] << pSelected->m_nToY << " id=" << bestidx
                //<< " alpha=" << alpha << "\n";
        }
        else
        {
            break;
        }
    }

    //cout << "Final find best move " << row_header[pSelected->m_nFromX] << pSelected->m_nFromY
      //  << " -> " << row_header[pSelected->m_nToX] << pSelected->m_nToY << " id=" << bestidx
        //<< " alpha=" << alpha << "\n";

    return pSelected;
}

Search* Search::searchMin(void)
{
    Search* pSelected = NULL;
    Search* pNode = m_pFirst;
    int alpha = 0, beta = 0;
    int bestidx = 0;
    int score = 0;
    int depth = 0;

    //cout << "Start Min value...." << "\n";

    clock_t c1 = clock();
    clock_t dt;

    beta = pNode->MaxValue(-DF_INFINITY, DF_INFINITY, depth);		// PVS
    alpha = beta - 1;		// Null window 설정
    pSelected = pNode;
    pNode = pNode->m_pNext;

    // 1st shallow search
    for (int i = 0; pNode != NULL; i++)
    {
        score = pNode->MaxValue(alpha, beta, depth);

        if (score == beta) { 	// 더 최적의 값이 나왔으므로 재탐색
            alpha = -DF_INFINITY;
            score = pNode->MaxValue(alpha, score + 1, depth);
        }
        if (score < beta)
        {
            beta = score;
            pSelected = pNode;
            alpha = beta - 1;	// Null window
            bestidx = i + 1;    // 0번째 node는 for문 전에 평가했으므로
        }
        pNode->m_nScore = score;
        pNode = pNode->m_pNext;
    }

    //cout << "1st find best move " << row_header[pSelected->m_nFromX] << pSelected->m_nFromY
      //  << " -> " << row_header[pSelected->m_nToX] << pSelected->m_nToY << " id=" << bestidx << " beta=" << beta << "\n";

    // Iterative Deepening
    bool stopFlag = false;
    for (int cnt = 0; cnt < 5 && !stopFlag; cnt++)
    {
        clock_t c2 = (clock() - c1) / (CLOCKS_PER_SEC / 1000);
        if (c2 < g_entryLimit) {
            //cout << cnt + 2 << "th search..." << "\n";

            beta = beta + 100;
            score = pSelected->MaxValue(-DF_INFINITY, beta, depth - (1 + cnt));
            if (score == beta)  // fail-high 발생
                score = pSelected->MaxValue(beta - 1, DF_INFINITY, depth - (1 + cnt));

            alpha = beta - 1;       // Null window 설정
            if (bestidx != 0)	// 첫번째 node가 아니라면 앞에 동일값의 node가 있을 수 있으므로
                beta += 1;
            pNode = m_pFirst;

            for (int i = 0; pNode != NULL; i++)
            {
                if (pNode == pSelected) {
                    pNode = pNode->m_pNext;
                    continue;
                }

                score = pNode->MaxValue(alpha, beta, depth - (1 + cnt));

                if (score == alpha) {
                    alpha = -DF_INFINITY;
                    score = pNode->MaxValue(alpha, score + 1, depth - (1 + cnt));
                }

                if (score < beta) {
                    beta = score;
                    pSelected = pNode;
                    alpha = beta - 1;  // Null window 설정
                    bestidx = i + 1;
                }
                pNode->m_nScore = score;
                pNode = pNode->m_pNext;

                dt = (clock() - c1) / (CLOCKS_PER_SEC / 1000);
                if (dt >= g_maxLimit) {
                    //cout << "TIME-OUT" << "\n";
                    stopFlag = true;
                    break;
                }
            }

            //cout << cnt + 2 << "th find best move " << row_header[pSelected->m_nFromX] << pSelected->m_nFromY
              //  << " -> " << row_header[pSelected->m_nToX] << pSelected->m_nToY << " id=" <<
                //bestidx << " beta=" << beta << "\n";
        }
        else
        {
            break;
        }
    }

    //cout << "Final find best move " << row_header[pSelected->m_nFromX] << pSelected->m_nFromY
      //  << " -> " << row_header[pSelected->m_nToX] << pSelected->m_nToY << " id=" << bestidx
        //<< " beta=" << beta << "\n";

    return pSelected;
}

int Search::generateMoves(SIDE side, MV* mvs) {
    m_enSide = side;
    int midx = 0;

    ClearList();
    GetNextStage();

    if (m_pFirst == NULL)
        return -1;

    Search* pNode = m_pFirst;

    do {
        mvs[midx] =
            MAKE_MV(
                MAKE_SQ(pNode->m_nFromX, pNode->m_nFromY),
                MAKE_SQ(pNode->m_nToX, pNode->m_nToY));

        pNode = pNode->m_pNext;
        midx++;
    } while (pNode != NULL);

    return 1;
}

void Search::delrepeatmv(MV repeatmv) {
    int mvSrc;
    Search* pNode = m_pFirst;

    do {
        mvSrc = MAKE_MV(
            MAKE_SQ(pNode->m_nFromX, pNode->m_nFromY),
            MAKE_SQ(pNode->m_nToX, pNode->m_nToY));
        if (repeatmv == mvSrc) {
            DeleteNode(pNode);
            delete pNode;
            break;
        }
        pNode = pNode->m_pNext;
    } while (pNode != NULL);
}

bool Search::UndoMove()
{
    SHistory sH;
    if (sm_cHistoryStack.Pop(sH))
    {
        AddPiece(sH.nFromX, sH.nFromY, m_enPanel[sH.nToY][sH.nToX], ADD);
        AddPiece(sH.nToX, sH.nToY, m_enPanel[sH.nToY][sH.nToX], DEL);

        if (sH.enEatenPiece != PS_NOTHING)
            AddPiece(sH.nToX, sH.nToY, sH.enEatenPiece, ADD);

        changeSide();

        updateMoveCount(DEL);

        return true;
    }

    return false;
}

void Search::changeSide(void)
{
    m_bAmIMax = (m_enSide == SD_HAN);
    m_enSide = (SD_HAN + 1) - m_enSide;
    zobristKey ^= PreGen_zobristKeyPlayer;
    zobristLock ^= PreGen_zobristLockPlayer;
}

//
// INTERFACE
//
void Search::initParams(int depth, int hashLevel, int entryLimitTime, int maxLimitTime) {
    unsigned char keydata[1] = { 0 };
    rc4.init(keydata, 1);

    PreGen_zobristKeyPlayer = rc4.nextLong();
    rc4.nextLong();
    PreGen_zobristLockPlayer = rc4.nextLong();

    for (int i = 0; i < 14; i++) {
        for (int j = 0; j < 256; j++) {
            long key = rc4.nextLong();
            rc4.nextLong();
            long lock = rc4.nextLong();

            PreGen_zobristKeyTable[i][j] = key;
            PreGen_zobristLockTable[i][j] = lock;
        }
    }

    // search engine parameters
    g_nMinimumDepth = (depth == 0) ? DEFAULT_CUTOFF_DEPTH : depth;

    g_entryLimit = entryLimitTime;
    g_maxLimit = maxLimitTime;

    //cout << "MinimumDepth=" << g_nMinimumDepth << " entryLimit=" << g_entryLimit
      //  << " maxLimit=" << g_maxLimit << "\n";

    // transpostion table 초기화
    int hashlevel = (hashLevel <= 0) ? DEFAULT_HASH_LEVEL : hashLevel;

    g_hashMask = (1 << hashlevel) - 1;
    g_hashTable = new HASHENT[(1 << hashlevel)]();
    //cout << "Hash parameter=" << hashLevel << " HashLevel=" << g_hashMask << "\n";

    m_nCount = 0;
    g_jolCnts = 0;
    g_gamePhase = PHASE_OPENING;
}

void Search::addPiece(int x, int y, char cp) {  // Interface Function
    PIECE pc = CHAR_TO_TEAM_PIECE(cp);

    AddPiece(x, y, pc, ADD);

    if (pc == PS_CHO_JOL || pc == PS_HAN_JOL)
        g_jolCnts++;
}

int Search::makeMove(int from_x, int from_y, int to_x, int to_y)
{
    if (m_enPanel[from_y][from_x] == PS_NOTHING)
        return -1;

    if (!isLegalMove(from_x, from_y, to_x, to_y))
        return -2;

    MakeMove(from_x, from_y, to_x, to_y);
    updateMoveCount(ADD);

    changeSide();

    return 1;
}

int Search::isLegalMove(int nFromX, int nFromY, int nToX, int nToY) {
    CMoveData cMD;
    cMD.Reset();

    if (!GetNextPieceLoc(nFromX, nFromY, &cMD))
        return -1;

    for (int i = 0; i < cMD.GetIndexCount(); i++)
    {
        if (nToX == cMD.GetToX(i) && nToY == cMD.GetToY(i))
            return 1;
    }

    return -2;
}

void Search::updateMoveCount(int bDec)
{
    if (bDec) {
        m_nCount--;
        if (g_gamePhase == PHASE_MIDGAME)
        {
            // 한수 무르기 고려
            if (m_nCount <= 18)	// 이동수가 18보다 작으면 OPENING
                g_gamePhase = PHASE_OPENING;

        }
    }
    else {
        m_nCount++;

        if (g_gamePhase == PHASE_OPENING)
        {
            if (m_nCount > 18)	// 이동수가 18보다 크면 게임중반이 시작된다고 본다
            {
                g_gamePhase = PHASE_MIDGAME;
                g_bPoBeforeJang = false;
            }
        }

        if (g_jolCnts <= 4)  // JOL이 4개 이하로 남으면 게임후반이라고 본다
        {
            g_gamePhase = PHASE_ENDGAME;
            // 게임후반이 되면 궁성보호 플래그를 해제하도록
            g_bProtectGung = false;
        }
    }
}

#ifdef EMSCRIPTEN
vector<int> moves;

emscripten::val Search::getNextPieceMoves(int from_x, int from_y) {
#else
int Search::getNextPieceMoves(int from_x, int from_y, MV * mvs) {
#endif
    CMoveData cMD;
    cMD.Reset();
#ifdef EMSCRIPTEN
    MV mv;
    moves.clear();
#endif

    if (!GetNextPieceLoc(from_x, from_y, &cMD))
#ifdef EMSCRIPTEN
        return emscripten::val::null();
#else
        return -1;
#endif
    for (int i = 0; i < cMD.GetIndexCount(); i++)
    {
#ifdef EMSCRIPTEN
        mv =
#else
        mvs[i] =
#endif
            MAKE_MV(
                MAKE_SQ(cMD.GetFromX(), cMD.GetFromY()),
                MAKE_SQ(cMD.GetToX(i), cMD.GetToY(i)));
#ifdef EMSCRIPTEN
        moves.push_back(mv);
#endif
    }

#ifdef EMSCRIPTEN
    return emscripten::val(emscripten::typed_memory_view(moves.size(), moves.data()));
#else
    return 1;
#endif
}

bool Search::GetLastMove(SHistory & LMStore)
{
    return sm_cHistoryStack.GetLastMove(LMStore);
}

bool Search::moveBack()		// 한수 무르기
{
    UndoMove();

    return true;
}

#ifdef EMSCRIPTEN
EMSCRIPTEN_BINDINGS(Search) {
    emscripten::class_<Search>("Search")
        .constructor<>()
        .function("getVersion", &Search::getVersion)
        .function("initParams", &Search::initParams)
        .function("addPiece", &Search::addPiece)
        .function("isLegalMove", &Search::isLegalMove)
        .function("GetSideAt", &Search::GetSideAt)
        .function("makeMove", &Search::makeMove)
        .function("moveBack", &Search::moveBack)
        .function("getNextPieceMoves", &Search::getNextPieceMoves)
        .function("getBestMove", &Search::getBestMove)
        .function("getScore", &Search::getScore);
};
#endif
