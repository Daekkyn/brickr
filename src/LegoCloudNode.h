#ifndef LEGO_CLOUD_NODE_H
#define LEGO_CLOUD_NODE_H

#include "LegoGraph.h"
#include "LegoCloud.h"

#include "Vector3.h"
#include <QObject>


class LegoBrick;

class QGraphicsScene;

class LegoCloudNode : public QObject
{
  Q_OBJECT

public:
  enum ColorRendering {RealColor, Random, ConnectedComp, BiconnectedComp};

  explicit LegoCloudNode();
  ~LegoCloudNode();


  //Inherited from Data
  int getType() const;
  std::string getTypeName() const;

  //Inherited from Node
  void recomputeAABB();//Override

  void render();

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

  void nodeUpdated() { drawDirty_ = true; recomputeAABB(); }

  void drawInstructions(QGraphicsScene* scene, bool hintLayerBelow);
  void exportToObj(QString filename);

  Vector3 minPoint() { return boundsMin_; }
  Vector3 maxPoint() { return boundsMax_; }

private:
  void drawLegoBrick(const LegoBrick& brick) const;
  void drawBox(const Vector3& p1, const Vector3& p2) const;
  void drawBrickOutline(const LegoBrick &brick) const;
  void drawKnobs(const LegoBrick& brick, const Vector3 &p1) const;
  void drawNeighbourhood(const LegoBrick& brick, const QSet<LegoBrick*>& neighbours) const;
  void drawLegoGraph(const LegoGraph& graph) const;
  void setColor(const LegoGraph::vertex_descriptor &vertex) const;

  Vector3 boundsMin_, boundsMax_;

  LegoCloud* legoCloud_;
  bool renderLayerByLayer_;
  int renderLayer_;
  uint32_t knobList_;
  bool renderBricks_;
  bool renderGraph_;
  ColorRendering colorRendering_;
  bool drawDirty_;
};

#endif
