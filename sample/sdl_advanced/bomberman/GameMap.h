#ifndef GameMap_H
#define GameMap_H

#include <vector>
#include <string>
#include <list>
#include "SparseGraph.h"
#include "Wall2D.h"
#include "Trigger.h"
#include "GameObject.h"
#include "GraphEdgeTypes.h"
#include "GraphNodeTypes.h"
#include "CellSpacePartition.h"
#include "TriggerSystem.h"

class GameObject;

class GameMap
{
public:

	//typedef NavGraphNode<Trigger<GameObject>*>         GraphNode;
	//typedef SparseGraph<GraphNode, BombermanGraphEdge>      NavGraph;
	typedef CellSpacePartition<SparseGraph<NavGraphNode<Trigger<GameObject>*>, BombermanGraphEdge>::NodeType*>   CellSpace;
  
private:

  //this map's accompanying navigation graph
	SparseGraph<NavGraphNode<Trigger<GameObject>*>, BombermanGraphEdge>*                          m_pNavGraph;

  //the graph nodes will be partitioned enabling fast lookup
  CellSpace*                        m_pSpacePartition;

  //the size of the search radius the cellspace partition uses when looking for 
  //neighbors 
  double                             m_dCellSpaceNeighborhoodRange;

  int m_iSizeX;
  int m_iSizeY;
  
  void  PartitionNavGraph();

  //this will hold a pre-calculated lookup table of the cost to travel from
  //one node to any other.
  std::vector<std::vector<double> >  m_PathCosts;

  void Clear();
  
public:
  
	GameMap();
	~GameMap();

  //loads an environment from a file
  bool CreateNaviGraph(std::vector<Vector2D> vecNavi);
  bool AddNavigationNode(int tileRow, int tileColumn);

  double   CalculateCostToTravelBetweenNodes(int nd1, int nd2)const;

  //returns the position of a graph node selected at random
  Vector2D GetRandomNodeLocation()const;
  
  
  SparseGraph<NavGraphNode<Trigger<GameObject>*>, BombermanGraphEdge>*                          GetNavGraph()const{ return m_pNavGraph; }
 
  CellSpace* const                   GetCellSpace()const{return m_pSpacePartition;}
  
  int                                GetSizeX()const{return m_iSizeX;}
  int                                GetSizeY()const{return m_iSizeY;}
  int                                GetMaxDimension()const{return Maximum(m_iSizeX, m_iSizeY);}
  double                             GetCellSpaceNeighborhoodRange()const{return m_dCellSpaceNeighborhoodRange;}

};



#endif