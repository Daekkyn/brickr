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

#ifndef LEGO_CLOUD_NODE_H
#define LEGO_CLOUD_NODE_H

#include <Dolphin/Core/Scenegraph/Core/GeometryNode.h>
#include <Dolphin/Core/DolphinDefines.h>

#include "LegoGraph.h"
#include "LegoCloud.h"

namespace Dolphin {
  class LegoBrick;
}

class QGraphicsScene;

namespace Dolphin {
namespace scenegraph {

class LegoCloudNode : public GeometryNode
{
  Q_OBJECT

public:
  typedef Dolphin::defines::Vector3 Point;
  typedef Dolphin::defines::Vector4 Color;
  enum ColorRendering {RealColor, Random, ConnectedComp, BiconnectedComp};

  explicit LegoCloudNode(Node* _parent);
  ~LegoCloudNode();


  //Inherited from Data
  int getType() const;
  std::string getTypeName() const;

  //Inherited from Node
  void accept(Visitor & _visitor);
  void recomputeBoundingSphere();//Override
  void recomputeAABB();//Override

  //Inherited from GeometryNode
  void resetSelection(const bool _notifyObservers = true);
  void fillDrawData(DrawModes* _drawmodes);
  void render(DrawModes* _drawmodes);//Override

  //New
  inline LegoCloud* getLegoCloud(){ return legoCloud_;}
  inline void setRenderLayerByLayer(bool v){renderLayerByLayer_ = v;}
  inline void setRenderLayer(int layer){
    if(layer >= legoCloud_->getLevelNumber())
    {
      renderLayer_ = legoCloud_->getLevelNumber()-1;
    }
    else if(layer < 0)
    {
      renderLayer_ = 0;
    }
    else
    {
      renderLayer_ = layer;
    }
  }
  inline void setRenderBricks(bool v){renderBricks_ = v;}
  inline void setRenderGraph(bool v){renderGraph_ = v;}
  inline void setColorRendering(ColorRendering col){colorRendering_ = col;}

  void drawInstructions(QGraphicsScene* scene, bool hintLayerBelow);
  void exportToObj(QString filename);


private:
  void drawLegoBrick(const LegoBrick& brick) const;
  void drawBox(const Point& p1, const Point& p2) const;
  void drawBrickOutline(const LegoBrick &brick) const;
  void drawKnobs(const LegoBrick& brick, const Point &p1) const;
  void drawNeighbourhood(const LegoBrick& brick, const QSet<LegoBrick*>& neighbours) const;
  void drawLegoGraph(const LegoGraph& graph) const;
  void setColor(const LegoGraph::vertex_descriptor &vertex) const;


  LegoCloud* legoCloud_;
  bool renderLayerByLayer_;
  int renderLayer_;
  GLuint knobList_;
  bool renderBricks_;
  bool renderGraph_;
  ColorRendering colorRendering_;


};

}}//namespaces

#endif
