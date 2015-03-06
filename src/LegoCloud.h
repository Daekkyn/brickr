#ifndef LEGO_CLOUD_H
#define LEGO_CLOUD_H

#include <qvector>
#include <qset>
#include <QMultiHash>
#include <QPair>
#include <QMap>

#include "LegoBrick.h"
#include "LegoGraph.h"

class LegoCloud
{
public:

  enum MergeStrategy{Random, MaxConnectivity};
  LegoCloud();
  ~LegoCloud();

  int getBrickNumber() const;
  inline int getLevelNumber() const {return levelNumber_;}
  inline int getConCompNumber() const {return conCompNumber_;}
  inline int getBadArtPointNumber() const {return badArtPointNumber_;}
  inline const QList<LegoBrick>& getBricks(int level) const {return bricks_[level];}
  inline const QList<LegoBrick*>& getOuterBricks() const {return outerBricks_;}

  LegoBrick* addBrick(int level, int posX, int posY);//Add a 1 by 1 brick

  void removeAllBricks();

  void setVoxelGridDimmension(int height, int width, int depth);//Before adding voxels
  void addVoxel(int level, int posX, int posY, LegoBrick * brick);

  void buildNeighbourhood();//Must be called just after having added all the 1x1 bricks
  void preHollow(int shellThickness);//Should be called right after buildNeighbourhood
  QSet<LegoBrick*>& getNeighbours(LegoBrick *brick);
  void merge();

  void solveBrickNumberLimitation();
  void setBrickLimit(BrickSize size, int value);

//  void loadColors(scenegraph::OpenMeshNode *meshNode);


  void printBrickTypes();
  void printStats();

  float postHollow();

  inline int getWidth() const {return width_;}
  inline int getDepth() const {return depth_;}

  //GRAPH
  inline const LegoGraph& getLegoGraph() const {return graph_;}

  void connectedComponents();
  void splitConComp();
  void loopConComp();

  void biconnectedComponents();
  void splitBiconComp();
  void loopBiconComp();

  inline const QVector<Color3>& getLegalColor() {return legalColors_;}


private:
  LegoBrick *addBrick(int level, int posX, int posY, int sizeX, int sizeY);//Level must already exist
  bool removeBrick(LegoBrick* brick);
  LegoBrick *mergeBricks(const QSet<LegoBrick *> brickToMerge);
  bool splitBrick(LegoBrick* brick);
  bool areNeighbours(LegoBrick* brick1, LegoBrick* brick2);//This is for building the neighbourhood (it does not use neighbourhood_)
  bool areConnected(const LegoBrick *brick1, const LegoBrick *brick2);
  bool canMerge(LegoBrick* brick1, LegoBrick* brick2);
  int connectionNumber(LegoBrick* brick1, LegoBrick* brick2);
  LegoBrick* findBestNeighbour(LegoBrick* brick, MergeStrategy strategy);

  bool cutBrick(LegoBrick *oldBrick, QPair<LegoBrick, LegoBrick> newBricks);
  QVector<QPair<LegoBrick, LegoBrick> > possibleCuts(LegoBrick* brick);
  int findBestCut(LegoBrick *brick, const QVector<QPair<LegoBrick, LegoBrick> >& cuts);//Returns the index of the best cut in "cuts" or -1 if there is no possible cut

  bool canRemoveBrick(LegoBrick *brick);

  QVector<QList<LegoBrick> > bricks_;//by level
  QHash<LegoBrick *, QSet<LegoBrick*> > neighbourhood_;
  QList<LegoBrick*> outerBricks_;
  QList<LegoBrick*> innerBricks_;

  int levelNumber_;

  //The voxelisation dimmensions
  int height_;//y
  int width_;//x
  int depth_;//z
  QVector<QVector<QVector<LegoBrick*> > > voxelGrid_;//This is used in the build neighbourhood method

  //GRAPH
  LegoGraph graph_;

  QHash<LegoBrick *, LegoGraph::vertex_descriptor> brickToVertex_;

  QSet<BrickSize> legalBricks_;
  QVector<Color3> legalColors_;

  QMap<BrickSize, int> brickLimitation_;
  QMap<BrickSize, int> brickNumber_;

  int conCompNumber_;
  int badArtPointNumber_;

  bool merged_;//True after the first merge
  bool brickLimitConstraint_;//If true, then merge will not create more of the bricks that are above the limit

};

#endif
