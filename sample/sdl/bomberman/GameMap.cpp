#include "GameMap.h"
#include "HandyGraphFunctions.h"
#include "GameObjectManager.h"
#include "BombermanStructure.h"

//----------------------------- ctor ------------------------------------------
//-----------------------------------------------------------------------------
GameMap::GameMap() :m_pNavGraph(NULL),
                       m_pSpacePartition(NULL),
                       m_iSizeY(0),
                       m_iSizeX(0),
                       m_dCellSpaceNeighborhoodRange(0)
{
}
//------------------------------ dtor -----------------------------------------
//-----------------------------------------------------------------------------
GameMap::~GameMap()
{
  Clear();
}

void GameMap::Clear()
{
  delete m_pNavGraph;   

  delete m_pSpacePartition;
}

bool GameMap::AddNavigationNode(int tileRow, int tileColumn)
{
	int nodeIndex = m_pNavGraph->AddNode(tileRow, tileColumn);

	if (nodeIndex >= 0)
	{
		SparseGraph<NavGraphNode<Trigger<GameObject>*>, BombermanGraphEdge>::NodeIterator NodeItr(*m_pNavGraph);
		for (SparseGraph<NavGraphNode<Trigger<GameObject>*>, BombermanGraphEdge>::NodeType* pN = NodeItr.begin(); !NodeItr.end(); pN = NodeItr.next())
		{
			if (pN->Index() == nodeIndex)
			{
				m_pSpacePartition->AddEntity(pN);
				return true;
			}
			
		}
	}
	
	

	return false;
}

bool GameMap::CreateNaviGraph(std::vector<Vector2D> vecNavi)
{
	Clear();

	m_pNavGraph = new  SparseGraph<NavGraphNode<Trigger<GameObject>*>, BombermanGraphEdge>(false);

	m_pNavGraph->Load(vecNavi);

	m_dCellSpaceNeighborhoodRange = CalculateAverageGraphEdgeLength(*m_pNavGraph);

	PartitionNavGraph();
	
	//calculate the cost lookup table
	//m_PathCosts = CreateAllPairsCostsTable(*m_pNavGraph);

	return true;
}





//------------- CalculateCostToTravelBetweenNodes -----------------------------
//
//  Uses the pre-calculated lookup table to determine the cost of traveling
//  from nd1 to nd2
//-----------------------------------------------------------------------------
double 
GameMap::CalculateCostToTravelBetweenNodes(int nd1, int nd2)const
{
  assert (nd1>=0 && nd1<m_pNavGraph->NumNodes() &&
          nd2>=0 && nd2<m_pNavGraph->NumNodes() &&
          "<Raven_Map::CostBetweenNodes>: invalid index");

  return m_PathCosts[nd1][nd2];
}




//-------------------------- PartitionEnvironment -----------------------------
//-----------------------------------------------------------------------------
void GameMap::PartitionNavGraph()
{
  if (m_pSpacePartition) delete m_pSpacePartition;

  m_iSizeX = 32 * 20;
  m_iSizeY = 32 * 20;
  m_pSpacePartition = new CellSpacePartition< SparseGraph<NavGraphNode<Trigger<GameObject>*>, BombermanGraphEdge>::NodeType*>(m_iSizeX,
                                                                  m_iSizeY,
                                                                  20,
                                                                  20,
                                                                  m_pNavGraph->NumNodes());

  //add the graph nodes to the space partition
  SparseGraph<NavGraphNode<Trigger<GameObject>*>, BombermanGraphEdge>::NodeIterator NodeItr(*m_pNavGraph);
  for (SparseGraph<NavGraphNode<Trigger<GameObject>*>, BombermanGraphEdge>::NodeType* pN = NodeItr.begin(); !NodeItr.end(); pN = NodeItr.next())
  {
    m_pSpacePartition->AddEntity(pN);
  }   
}


//------------------------- GetRandomNodeLocation -----------------------------
//
//  returns the position of a graph node selected at random
//-----------------------------------------------------------------------------
Vector2D GameMap::GetRandomNodeLocation()const
{
	SparseGraph<NavGraphNode<Trigger<GameObject>*>, BombermanGraphEdge>::ConstNodeIterator NodeItr(*m_pNavGraph);
  int RandIndex = RandInt(0, m_pNavGraph->NumActiveNodes()-1);
  const  SparseGraph<NavGraphNode<Trigger<GameObject>*>, BombermanGraphEdge>::NodeType* pN = NodeItr.begin();
  while (--RandIndex > 0)
  {
    pN = NodeItr.next();
  }

  return pN->Pos();
}