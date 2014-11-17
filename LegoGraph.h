// Copyright 2012, 2013 Romain Testuz
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef LEGOGRAPH_H
#define LEGOGRAPH_H

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <boost/tuple/tuple.hpp> // to access tie() (not in this file)

#include "LegoBrick.h"

namespace Dolphin {

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

}//namespace

#endif // LEGOGRAPH_H
