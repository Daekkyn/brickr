#ifndef LEGOGRAPH_H
#define LEGOGRAPH_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/tuple/tuple.hpp> // to access tie() (not in this file)

#include "LegoBrick.h"

struct LegoVertex
{
  LegoBrick* brick;
  int connected_comp;
  bool articulationPoint;
  bool badArticulationPoint;
};

struct LegoEdge
{
  int biconnected_comp;
};


typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS, LegoVertex, LegoEdge> LegoGraph;


#endif // LEGOGRAPH_H
