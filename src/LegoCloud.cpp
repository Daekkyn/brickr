#include "LegoCloud.h"
#include "LegoCloudNode.h"

#include <boost/graph/connected_components.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <boost/tuple/tuple.hpp>
#include <QTime>

#define DEFAULT_COLOR_ID 2

LegoCloud::LegoCloud()
{
  levelNumber_ = 0;
  merged_ = false;
  brickLimitConstraint_ = false;

  QVector<char> legalLengths;
  legalLengths.push_back(1);
  legalLengths.push_back(2);
  legalLengths.push_back(3);
  legalLengths.push_back(4);
  //legalLengths.push_back(5);
  legalLengths.push_back(6);
  //legalLengths.push_back(7);
  legalLengths.push_back(8);

  foreach(char l, legalLengths)
  {
    //legalBricks_.insert(BrickSize(1, l));
    legalBricks_.insert(BrickSize(1, l));
    brickLimitation_[BrickSize(1, l)] = -1;

    //legalBricks_.insert(BrickSize(2, l));
    if(l>1)//Prevent inserting (2,1) because (1,2) is already there
    {
      legalBricks_.insert(BrickSize(2, l));
      brickLimitation_[BrickSize(2, l)] = -1;
    }
  }

  /*legalColors_.push_back(LegoBrick::Color4(242/255.0, 243/255.0, 242/255.0, 1));//White
  legalColors_.push_back(LegoBrick::Color4(27/255.0, 42/255.0, 52/255.0, 1));//Black
  legalColors_.push_back(LegoBrick::Color4(196/255.0, 40/255.0, 27/255.0, 1));//Bright red
  legalColors_.push_back(LegoBrick::Color4(13/255.0, 105/255.0, 171/255.0, 1));//Bright blue
  legalColors_.push_back(LegoBrick::Color4(40/255.0, 127/255.0, 70/255.0, 1));//Dark green
  legalColors_.push_back(LegoBrick::Color4(164/255.0, 189/255.0, 70/255.0, 1));//Lime (Yellowish green)
  legalColors_.push_back(LegoBrick::Color4(245/255.0, 205/255.0, 47/255.0, 1));//Bright yellow
  legalColors_.push_back(LegoBrick::Color4(218/255.0, 133/255.0, 64/255.0, 1));//Bright orange
  legalColors_.push_back(LegoBrick::Color4(105/255.0, 64/255.0, 39/255.0, 1));//Redish brown*/

  legalColors_.push_back(Color3(0.95f, 0.95f, 0.95f));//White
  legalColors_.push_back(Color3(0.15f, 0.15f, 0.15f));//Black
  legalColors_.push_back(Color3(1.0f, 0.0f, .00f));//Bright red
  legalColors_.push_back(Color3(0.0f, 0.0f, 1.0f));//Bright blue
  legalColors_.push_back(Color3(14.0f/255.0f, 132.0f/255.0f, 36.0f/255.0f));//Dark green
  legalColors_.push_back(Color3(180.0f/255.0f, 1.0f, 0.0f));//Lime (Yellowish green)
  legalColors_.push_back(Color3(1.0f, 1.0f, 0.0f));//Bright yellow
  legalColors_.push_back(Color3(1.0f, 110.0f/255.0f, 0.0f));//Bright orange
  legalColors_.push_back(Color3(118.0f/255.0f, 45.0f/255.0f, 28.0f/255.0f));//Redish brown

  assert(DEFAULT_COLOR_ID < legalColors_.size());
}

LegoCloud::~LegoCloud()
{
  removeAllBricks();
  //std::cout << "Cloud destroyed" << std::endl;
}

int LegoCloud::getBrickNumber() const
{
  int brickNumber = 0;
  for(int level=0; level < levelNumber_; level++)
  {
    brickNumber += bricks_[level].size();
  }
  return brickNumber;
}

LegoBrick *LegoCloud::addBrick(int level, int posX, int posY)
{
  //If level is higher than the curent max level, all the levels between must be added
  if(level+1 > levelNumber_)
  {
    for(int i = 0; i < level+1 - levelNumber_; i++)
    {
      QList<LegoBrick> levelBricks;
      bricks_.append(levelBricks);
    }
    levelNumber_ = level+1;
  }
  assert(bricks_.size() > level);

  LegoBrick brick(level, posX, posY, 1, 1);
  assert(!bricks_[level].contains(brick));

  brickNumber_[BrickSize(1,1)]++;

  bricks_[level].push_back(brick);
  LegoBrick* brickPointer = &(bricks_[level].last());

  LegoGraph::vertex_descriptor vertex = boost::add_vertex(graph_);
  graph_[vertex].brick = brickPointer;
  brickToVertex_.insert(brickPointer, vertex);

  assert(brickPointer != NULL);
  return brickPointer;
}

void LegoCloud::removeAllBricks()
{
  graph_.clear();
  bricks_.clear();
  neighbourhood_.clear();
  levelNumber_ = 0;
  width_ = 0;
  depth_ = 0;
  voxelGrid_.clear();
  brickLimitation_.clear();
  brickNumber_.clear();
}

void LegoCloud::setVoxelGridDimmension(int height, int width, int depth)
{
  height_ = height;
  width_ = width;
  depth_ = depth;

  voxelGrid_.clear();

  //Resize the grid
  voxelGrid_.resize(height);
  for(int level = 0; level < height; level++)
  {
    voxelGrid_[level].resize(width);
    for(int x = 0; x < width; x++)
    {
      voxelGrid_[level][x].resize(depth);
      for(int y = 0; y < depth; y++)
      {
        voxelGrid_[level][x][y] = NULL;
      }
    }
  }
}

void LegoCloud::addVoxel(int level, int posX, int posY, LegoBrick* brick)
{
  assert(voxelGrid_.size() >= level);
  assert(voxelGrid_[0].size() >= posX);
  assert(voxelGrid_[0][0].size() >= posY);

  voxelGrid_[level][posX][posY] = brick;
}



void LegoCloud::buildNeighbourhood()//Assumption: there is only 1x1 bricks
{
  QSet<LegoBrick*> neighbours;
  QSet<LegoBrick*> toRemove;

  int width = 0;
  int depth = 0;

  for(int level = 0; level < levelNumber_; level++)
  {
    for(QList<LegoBrick>::iterator brickIt = bricks_[level].begin(); brickIt != bricks_[level].end(); brickIt++)
    {
      assert(brickIt->getKnobNumber() == 1);
      neighbours.clear();
      int neighbourNumber = 0;

      if(brickIt->getPosX()+1 > width)
        width = brickIt->getPosX()+1;
      if(brickIt->getPosY()+1 > depth)
        depth = brickIt->getPosY()+1;

      //Search for the left neighbour
      if(brickIt->getPosX() > 0 )
      {
        LegoBrick* neighbour = voxelGrid_[level][brickIt->getPosX()-1][brickIt->getPosY()];
        if(neighbour != NULL)
        {
          neighbours.insert(neighbour);
          neighbourNumber++;
        }
      }

      //Search for the right neighbour
      if(brickIt->getPosX() < width_-1 )
      {
        LegoBrick* neighbour = voxelGrid_[level][brickIt->getPosX()+1][brickIt->getPosY()];
        if(neighbour != NULL)
        {
          neighbours.insert(neighbour);
          neighbourNumber++;
        }
      }

      //Search for the back neighbour
      if(brickIt->getPosY() > 0 )
      {
        LegoBrick* neighbour = voxelGrid_[level][brickIt->getPosX()][brickIt->getPosY()-1];
        if(neighbour != NULL)
        {
          neighbours.insert(neighbour);
          neighbourNumber++;
        }
      }

      //Search for the front neighbour
      if(brickIt->getPosY() < depth_-1 )
      {
        LegoBrick* neighbour = voxelGrid_[level][brickIt->getPosX()][brickIt->getPosY()+1];
        if(neighbour != NULL)
        {
          neighbours.insert(neighbour);
          neighbourNumber++;
        }
      }

      neighbourhood_.insert(&(*brickIt), neighbours);

      //GRAPH
      if(level < levelNumber_-1)
      {
        LegoBrick* aboveNeighbour = voxelGrid_[level+1][brickIt->getPosX()][brickIt->getPosY()];
        if(aboveNeighbour != NULL)
        {
          boost::add_edge(brickToVertex_[&(*brickIt)], brickToVertex_[aboveNeighbour],graph_);
          neighbourNumber++;
        }
      }

      if(level > 0)
      {
        LegoBrick* aboveNeighbour = voxelGrid_[level-1][brickIt->getPosX()][brickIt->getPosY()];
        if(aboveNeighbour != NULL)
        {
          //Must not add this edge to prevent multigraph
          //boost::add_edge(brickToVertex_[&(*brickIt)], brickToVertex_[aboveNeighbour],graph_);
          neighbourNumber++;
        }
      }

      if(neighbourNumber < 6)//If one 1x1 brick has less than 4 neighbours, it must be on the outside
      {
        brickIt->setIsOuter(true);
        outerBricks_.append(&(*brickIt));
      }
      else
      {
        brickIt->setIsOuter(false);
        innerBricks_.append(&(*brickIt));
      }

      brickIt->setColorId(DEFAULT_COLOR_ID);

      if(neighbourNumber == 0)
      {
        //std::cout << "One brick was removed because it had zero neighbours: ";
        //brickIt->print();
        toRemove.insert(&(*brickIt));
        //brickIt->setColorId(8);
        //removeBrick(&(*brickIt));

      }

    }
  }

  foreach(LegoBrick* brick, toRemove)
  {
    std::cout << "One brick was removed because it had zero neighbours: "; brick->print();
    removeBrick(brick);
    //brick->setColorId(1);
  }

  width_ = width;
  depth_ = depth;

  assert(brickNumber_[BrickSize(1,1)] == getBrickNumber());
  connectedComponents();
  biconnectedComponents();
}

void LegoCloud::preHollow(int shellThickness)
{
  if(shellThickness < 1)
  {
    std::cerr << "The shell thickness should be greater than 0" << std::endl;
  }

  QList<LegoBrick*> toDelete;
  for(int level = 0; level < levelNumber_; level++)
  {
    //for(QSet<LegoBrick>::iterator brickIt = bricks_[level].begin(); brickIt != bricks_[level].end(); brickIt++)
    for(QList<LegoBrick>::iterator brickIt = bricks_[level].begin(); brickIt != bricks_[level].end(); brickIt++)//QTL
    {
      LegoBrick* brick = &(*brickIt);

      if(brick->getLevel() - shellThickness < 0 || brick->getLevel() + shellThickness >= height_ ||
         brick->getPosX() - shellThickness < 0 || brick->getPosX() + shellThickness >= width_ ||
         brick->getPosY() - shellThickness < 0 || brick->getPosY() + shellThickness >= depth_
         )
      {
        //The brick is on the border of the domain, it must not be removed
        continue;
      }

      bool remove = true;
      for(int l = brick->getLevel() - shellThickness; l <= brick->getLevel() + shellThickness; l++)
      {
        for(int x = brick->getPosX() - shellThickness; x <= brick->getPosX() + shellThickness; x++)
        {
          for(int y = brick->getPosY() - shellThickness; y <= brick->getPosY() + shellThickness; y++)
          {
            if(voxelGrid_[l][x][y] == NULL)
            {
              remove = false;//The brick is within the border of the object and must not be removed
            }
          }
        }
      }
      if(remove)
        toDelete.append(brick);//The brick is not on the border and can be removed
    }
  }

  for(QList<LegoBrick*>::iterator brickIt = toDelete.begin(); brickIt != toDelete.end(); brickIt++)
  {
    LegoBrick* brick = *brickIt;
    voxelGrid_[brick->getLevel()][brick->getPosX()][brick->getPosY()] = NULL;
    removeBrick(brick);
  }

  voxelGrid_.clear();
}

QSet<LegoBrick *>& LegoCloud::getNeighbours(LegoBrick *brick)
{
  return neighbourhood_[brick];
}

void LegoCloud::merge()
{
  //progress::setNumberOfSteps(levelNumber_, "merging...");
  //progress::setProgress(0);

  QSet<LegoBrick*> toMerge;

  int noSuccessNumber = 0;

  //First merge the outside bricks
  while(noSuccessNumber < outerBricks_.size())
  {
    toMerge.clear();

    LegoBrick* brickToMerge = outerBricks_[rand()%outerBricks_.size()];
    LegoBrick* neighbourToMerge;

    if(merged_)//True only after the first merge
      neighbourToMerge = findBestNeighbour(brickToMerge, Random);
    else
      neighbourToMerge = findBestNeighbour(brickToMerge, MaxConnectivity);

    while(neighbourToMerge != NULL)
    {
      toMerge.clear();
      toMerge.insert(neighbourToMerge);
      toMerge.insert(brickToMerge);

      brickToMerge = mergeBricks(toMerge);
      assert(brickToMerge != NULL);

      if(merged_)//True only after the first merge
        neighbourToMerge = findBestNeighbour(brickToMerge, Random);
      else
        neighbourToMerge = findBestNeighbour(brickToMerge, MaxConnectivity);

      noSuccessNumber = 0;
    }

    noSuccessNumber++;
  }
  //std::cout  << "Outer finished" << std::endl;

  //Then merge the inside bricks
  noSuccessNumber = 0;
  while(noSuccessNumber < innerBricks_.size())
  {
    toMerge.clear();

    LegoBrick* brickToMerge = innerBricks_[rand()%innerBricks_.size()];
    LegoBrick* neighbourToMerge;

    if(merged_)//True only after the first merge
      neighbourToMerge = findBestNeighbour(brickToMerge, Random);
    else
      neighbourToMerge = findBestNeighbour(brickToMerge, MaxConnectivity);

    while(neighbourToMerge != NULL)
    {
      toMerge.clear();
      toMerge.insert(neighbourToMerge);
      toMerge.insert(brickToMerge);

      brickToMerge = mergeBricks(toMerge);
      assert(brickToMerge != NULL);
      if(merged_)//True only after the first merge
        neighbourToMerge = findBestNeighbour(brickToMerge, Random);
      else
        neighbourToMerge = findBestNeighbour(brickToMerge, MaxConnectivity);
      noSuccessNumber = 0;
    }
    noSuccessNumber++;
  }

  connectedComponents();
  biconnectedComponents();

  merged_ = true;
  //progress::finish();
}

//This method should be the last call before saving instructions
void LegoCloud::solveBrickNumberLimitation()
{
  for(int level = 0; level < levelNumber_; level++)
  {
    QList<LegoBrick>::iterator brickIt;
    for(brickIt = bricks_[level].begin(); brickIt!=bricks_[level].end(); brickIt++)
    {
      int limit = brickLimitation_[brickIt->getSize()];
      if(limit != -1 && brickNumber_[brickIt->getSize()] > limit)
      {
        QVector<QPair<LegoBrick, LegoBrick> > cuts = possibleCuts(&*brickIt);
        int bestCutIndex = findBestCut(&*brickIt, cuts);
        if(bestCutIndex != -1)
        {
          cutBrick(&*brickIt, cuts[bestCutIndex]);
          brickIt = bricks_[level].begin();
        }

      }
    }
  }

  //Print if all the constrainsts are satisfied
  bool noProblem = true;
  foreach(const BrickSize& size, brickLimitation_.keys())
  {
    int limit = brickLimitation_[size];
    if(limit != -1 && brickNumber_[size] > limit)
    {
      std::cout << "The number of " << size.first <<"x" << size.second << " bricks ("<< brickNumber_[size]
                   <<") cannot be reduced to " << limit << std::endl;
      noProblem = false;

    }
  }

  if(noProblem)
    std::cout << "Solving number constraints terminated, all constraints are satisfied." << std::endl;

  connectedComponents();
  biconnectedComponents();
  brickLimitConstraint_ = true;
}

void LegoCloud::setBrickLimit(BrickSize size, int value)
{
  brickLimitation_[size] = value;
}

/*
void LegoCloud::loadColors(scenegraph::OpenMeshNode* meshNode)
{
  typedef Dolphin::defines::Vector3 Vec3;
  typedef Dolphin::defines::Vector2 Vec2;

  utilities::NearestTriangleSearch nts;
  OMesh* mesh = meshNode->getMesh().get();
  nts.initializeKDTreeBasedSearchStructure(mesh, 10, 30);

  meshNode->getTexture()->enableTextureing();

  GLint textureWidth, textureHeight;// internalFormat;
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
  glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &textureHeight);
  //glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_COMPONENTS, &internalFormat);

  if(textureWidth != textureHeight)
  {
    std::cout << "Warning: the texture is not squared, some problems might occur." << std::endl;
  }

  GLint numBytes = 0;

  numBytes = textureWidth * textureHeight * 3;
  /*switch(internalFormat) // determine what type GL texture has...
  {
    case GL_RED:
      std::cout << "Red" << std::endl;
      numBytes = textureWidth * textureHeight * 1;
      break;
    case GL_RGB:
      std::cout << "RGB" << std::endl;
      numBytes = textureWidth * textureHeight * 3;
      break;
    case GL_RGBA:
      std::cout << "RGBA" << std::endl;
      numBytes = textureWidth * textureHeight * 4;
      break;
    default: // unsupported type (or you can put some code to support more formats if you need)
      std::cerr << "Unsupported image type: " << internalFormat << std::endl;
      return;
      break;
  }* /


  GLubyte* buffer = new GLubyte[numBytes];

  //Save the texture to buffer
  glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
  meshNode->getTexture()->disableTextureing();

  for(int level = 0; level < levelNumber_; level++)
  {
    QList<LegoBrick>::iterator brickIt;
    for(brickIt = bricks_[level].begin(); brickIt!=bricks_[level].end(); brickIt++)
    {
      //if(brickIt->isOuter())
      {
        Vec3 brickCenter;//Center of a brick
        brickCenter[0] = brickIt->getPosX()*LEGO_KNOB_DISTANCE + LEGO_KNOB_DISTANCE*0.5;
        brickCenter[1] = brickIt->getLevel()*LEGO_HEIGHT + LEGO_HEIGHT*0.5;
        brickCenter[2] = brickIt->getPosY()*LEGO_KNOB_DISTANCE + LEGO_KNOB_DISTANCE*0.5;

        //Find the closest triangle from brickCenter
        OMesh::FaceHandle fh;
        nts.computeClosestTriangleOfPoint(brickCenter, mesh, fh);


        //OMesh::ConstFaceVertexIter fvI = mesh->fv_iter(fh);
        OMesh::FaceHalfedgeIter fheIt = mesh->fh_iter(fh);

        //Compute the 3 texture coordinates of the 3 triangle vertices
        Vec2 texA = mesh->texcoord2D(fheIt);
        //std::cout << texA << std::endl;
        texA.minimize(Vec2(1.0, 1.0));
        texA.maximize(Vec2(0.0, 0.0));
        //std::cout << texA << std::endl << std::endl;

        Vec3 pointA = mesh->point(mesh->to_vertex_handle(fheIt));
        ++fheIt;

        Vec2 texB = mesh->texcoord2D(fheIt);
        texB.minimize(Vec2(1.0, 1.0));
        texB.maximize(Vec2(0.0, 0.0));
        Vec3 pointB = mesh->point(mesh->to_vertex_handle(fheIt));
        ++fheIt;

        Vec2 texC = mesh->texcoord2D(fheIt);
        texC.minimize(Vec2(1.0, 1.0));
        texC.maximize(Vec2(0.0, 0.0));
        Vec3 pointC = mesh->point(mesh->to_vertex_handle(fheIt));


        //*** Compute the barycenters of the projection of brickCenter on the triangle
        Vec3 ab = (pointB - pointA);
        Vec3 ac = (pointC - pointA);

        double area = cross(ab, ac).norm()/2.0;//Area of the triangle

        Vec3 normal = mesh->normal(fh);
        Vec3 p = brickCenter - normal*((brickCenter - pointA).dot(normal));//brickCenter projected on the triangle

        Vec3 pc = pointC - p;
        Vec3 pb = pointB - p;
        double areaA = cross(pc, pb).norm()/2.0;//Area of triangle pcb

        Vec3 pa = pointA - p;
        double areaB = cross(pc, pa).norm()/2.0;//Area of triangle pca

        double baryA = areaA/area;
        double baryB = areaB/area;
        double baryC = 1.0 - (baryA + baryB);
        //***

        //Clamp to vertex if outside triangle (could also clamp to edge)
        if(baryA > 1.0)
        {
          baryA = 1.0;
          baryB = 0.0;
          baryC = 0.0;
        }
        else if(baryB > 1.0)
        {
          baryA = 0.0;
          baryB = 1.0;
          baryC = 0.0;
        }
        else if(baryC > 1.0)
        {
          baryA = 0.0;
          baryB = 0.0;
          baryC = 1.0;
        }

        if(baryA < 0.0)
        {
          baryA = 0.0;
          baryB = 0.5;
          baryC = 0.5;
        }
        else if(baryB < 0.0)
        {
          baryA = 0.5;
          baryB = 0.0;
          baryC = 0.5;
        }
        else if(baryC < 0.0)
        {
          baryA = 0.5;
          baryB = 0.5;
          baryC = 0.0;
        }



        //Compute the texture coordinates using the barycentric coordinates
        Vec2 baryTex = baryA*texA + baryB*texB + baryC*texC;

        //Translate baryTex into the index of "buffer"
        int startIndexOfPixel = ((((int)(baryTex.y()*(textureHeight))) * (textureWidth)) + (int)(baryTex.x()*(textureWidth-1))) * 3;
        startIndexOfPixel = min(startIndexOfPixel, numBytes-3);

        assert(startIndexOfPixel+2 < numBytes);
        Vec3 color(buffer[startIndexOfPixel]/255.0, buffer[startIndexOfPixel+1]/255.0, buffer[startIndexOfPixel+2]/255.0);

        //*** From this color, find the closest LEGO legal color
        int closestColorIndex = 0;
        float min = 10000.0;
        for(int i = 0; i < legalColors_.size(); i++)
        {
          if((color - legalColors_[i]).sqrnorm() < min)
          {
            min = (color - legalColors_[i]).sqrnorm();
            closestColorIndex = i;
          }
        }
        //***

        brickIt->setColorId(closestColorIndex);
      }
    }
  }

  delete[] buffer;
}
*/


void LegoCloud::printBrickTypes()
{
  //Print the brick type by color and by type
  QHash<int, QHash<BrickSize, int> > bricksByColorByType;

  for(int colorId = 0; colorId < legalColors_.size(); colorId++)
  {
    bricksByColorByType[colorId] = QHash<BrickSize, int>();
    foreach(const BrickSize& brickSize, legalBricks_)
    {
      bricksByColorByType[colorId][brickSize] = 0;
    }
  }

  const int brick0ColorID = (*(innerBricks_.begin()))->getColorId();//We take from innerBricks_ because bricks_[0] could be empty
  bool isSingleColor = true;

  for(int level = 0; level < levelNumber_; level++)
  {
    QList<LegoBrick>::iterator brickIt;
    for(brickIt = bricks_[level].begin(); brickIt!=bricks_[level].end(); brickIt++)
    {
      int colorID = brickIt->getColorId();
      bricksByColorByType[colorID][brickIt->getSize()]++;

      if(brick0ColorID != colorID)
      {
        isSingleColor = false;
      }
    }
  }

  //We only display the by color brick types if there is more than 1 color
  if(!isSingleColor)
  {
    for(int colorId = 0; colorId < legalColors_.size(); colorId++)
    {

      std::cout << "Color: (" << legalColors_[colorId] << "): " << std::endl;

      foreach(const BrickSize& brickSize, legalBricks_)
      {
        if(bricksByColorByType[colorId][brickSize] > 0)
          std::cout << "\t" << brickSize.first << "x" << brickSize.second  << ": "<< bricksByColorByType[colorId][brickSize] << " bricks" << std::endl;
      }
    }
    std::cout << std::endl;
  }


  foreach(const BrickSize& brickSize, brickNumber_.keys())
  {
    int size = brickNumber_[brickSize];
    std::cout << brickSize.first << "x" << brickSize.second << ": " << size << " bricks" << std::endl;
  }

}

void LegoCloud::printStats()
{
  std::cout << "Stats:" << std::endl;
  printBrickTypes();
  std::cout << "Number of bricks: " << getBrickNumber() << std::endl;
  std::cout << "Height: " << levelNumber_ << " levels " << "(" <<levelNumber_*LEGO_HEIGHT*100.0 << "cm)" << std::endl;
  std::cout << "Number of connected components: "<< conCompNumber_ << std::endl;
  std::cout << "Number of weak articulation points: " << badArtPointNumber_ << std::endl;
}

float LegoCloud::postHollow()
{
  std::cout << "Begin hollow..." << std::endl;
//  progress::setNumberOfSteps(levelNumber_, "hollowing...");
//  progress::setProgress(0);


  QTime time;
  time.start();

  //progress::setNumberOfSteps(levelNumber_, "hollowing out...");
  //progress::setProgress(0);

  int noSuccessNumber = 0;
  while(noSuccessNumber < innerBricks_.size())
  {

    LegoBrick* randomInnerBrick = innerBricks_[rand()%innerBricks_.size()];
    if(canRemoveBrick(randomInnerBrick))
    {
      removeBrick(randomInnerBrick);
      noSuccessNumber = 0;
    }
    else
    {
      noSuccessNumber++;
    }
  }

  /*for(QList<LegoBrick*>::Iterator innerIt = innerBricks_.begin(); innerIt != innerBricks_.end(); innerIt++)
  {
    if(canRemoveBrick(*innerIt))
    {
      removeBrick(*innerIt);
      innerIt = innerBricks_.begin();//The iterator has been invalidated
    }
  }*/

  //std::cout << "Time hollow: " << time.elapsed()/1000.0 << std::endl;

  connectedComponents();
  biconnectedComponents();
//  progress::finish();

  std::cout << "End hollow" << std::endl;
  //progress::finish();

  return time.elapsed()/1000.0;
}

void LegoCloud::connectedComponents()
{
  //Vertex index map (input)
  typedef std::map<LegoGraph::vertex_descriptor, int> VertexIndexMap;
  VertexIndexMap vertex_id_map;

  typedef boost::associative_property_map<VertexIndexMap> VertexIdPropertyMap;
  VertexIdPropertyMap vertex_index_pmap(vertex_id_map);

  int index = 0;
  LegoGraph::vertex_iterator vertexIt, vertexItEnd;
  for (boost::tie(vertexIt, vertexItEnd) = boost::vertices(graph_); vertexIt != vertexItEnd; ++vertexIt)
  {
    vertex_index_pmap[*vertexIt] = index;
    index++;
  }

  //Component map (output)
  boost::property_map < LegoGraph, int LegoVertex::* >::type component_map = get(&LegoVertex::connected_comp, graph_);

  conCompNumber_ = boost::connected_components(graph_, component_map, boost::vertex_index_map(vertex_index_pmap));
}

void LegoCloud::splitConComp()
{
  QSet<LegoBrick*> toSplit;

  for(int level = 0; level < levelNumber_; level++)
  {
    QList<LegoBrick>& bricks = bricks_[level];

    QList<LegoBrick>::iterator brickIt;
    for(brickIt = bricks.begin(); brickIt!=bricks.end(); brickIt++)
    {
      int connected_comp = graph_[brickToVertex_[&(*brickIt)]].connected_comp;

      const QSet<LegoBrick*>& neighbours = neighbourhood_.value(&(*brickIt));
      foreach(LegoBrick* neighbour, neighbours)
      {
        if(connected_comp != graph_[brickToVertex_[neighbour]].connected_comp)
        {
          toSplit.insert(&(*brickIt));
          toSplit.insert(neighbour);
        }
      }
    }
  }

  foreach(LegoBrick* brickToSplit, toSplit)
  {
    splitBrick(brickToSplit);
  }

  connectedComponents();
  biconnectedComponents();
}

void LegoCloud::loopConComp()
{
  for(int i=0; i<10; i++)
  {
    splitConComp();
    merge();
  }

  connectedComponents();
  biconnectedComponents();
}

void LegoCloud::biconnectedComponents()
{
  //Vertex index map (input)
  typedef std::map<LegoGraph::vertex_descriptor, int> VertexIndexMap;
  VertexIndexMap vertex_id_map;

  typedef boost::associative_property_map<VertexIndexMap> VertexIdPropertyMap;
  VertexIdPropertyMap vertex_index_pmap(vertex_id_map);

  int index = 0;
  LegoGraph::vertex_iterator vertexIt, vertexItEnd;
  for (boost::tie(vertexIt, vertexItEnd) = boost::vertices(graph_); vertexIt != vertexItEnd; ++vertexIt)
  {
    vertex_index_pmap[*vertexIt] = index;
    index++;
  }

  size_t num_biconnected_components = 0;
  std::vector<LegoGraph::vertex_descriptor> art_points;

  boost::property_map < LegoGraph, int LegoEdge::* >::type bicomponent_map = get(&LegoEdge::biconnected_comp, graph_);

  boost::tie(num_biconnected_components, boost::tuples::ignore) = boost::biconnected_components(graph_, bicomponent_map, std::back_inserter(art_points), boost::vertex_index_map(vertex_index_pmap));

  //Reset the articulation point property to false
  for (boost::tie(vertexIt, vertexItEnd) = boost::vertices(graph_); vertexIt != vertexItEnd; ++vertexIt)
  {
    graph_[*vertexIt].articulationPoint = false;
    graph_[*vertexIt].badArticulationPoint = false;
  }

  //Set the articulation point and badArtPoint properties
  badArtPointNumber_ = 0;
  LegoGraph::adjacency_iterator neighbourIt, neighbourItEnd;
  for(std::vector<LegoGraph::vertex_descriptor>::iterator it = art_points.begin(); it != art_points.end(); it++)
  {
    graph_[*it].articulationPoint = true;


    int firstBigBiconComp = -1;//The value of one of the biconComp of incident edge from *it which target vertex is not alone
    for (boost::tie(neighbourIt, neighbourItEnd) = boost::adjacent_vertices(*it, graph_); neighbourIt != neighbourItEnd; ++neighbourIt)
    {
      if(boost::out_degree(*neighbourIt, graph_) > 1)
      {
        LegoGraph::edge_descriptor edge;
        boost::tie(edge, boost::tuples::ignore) = boost::edge(*it, *neighbourIt, graph_);

        if(firstBigBiconComp == -1)
        {
          firstBigBiconComp = graph_[edge].biconnected_comp;
        }
        else if(firstBigBiconComp != graph_[edge].biconnected_comp)
        {
          graph_[*it].badArticulationPoint = true;
          badArtPointNumber_++;
          break;
        }
      }
    }

  }
}

void LegoCloud::splitBiconComp()
{
  QSet<LegoBrick*> toSplit;

  LegoGraph::vertex_iterator vertexIt, vertexItEnd;
  for (boost::tie(vertexIt, vertexItEnd) = boost::vertices(graph_); vertexIt != vertexItEnd; ++vertexIt)
  {
    if(graph_[*vertexIt].badArticulationPoint)
    {
      toSplit += neighbourhood_[graph_[*vertexIt].brick];
      toSplit.insert(graph_[*vertexIt].brick);

      LegoGraph::adjacency_iterator neighbourIt, neighbourItEnd;
      for (boost::tie(neighbourIt, neighbourItEnd) = boost::adjacent_vertices(*vertexIt, graph_); neighbourIt != neighbourItEnd; ++neighbourIt)
      {
        toSplit.insert(graph_[*neighbourIt].brick);
      }
    }
  }


  foreach(LegoBrick* brickToSplit, toSplit)
  {
    splitBrick(brickToSplit);
  }

  connectedComponents();
  biconnectedComponents();
}

void LegoCloud::loopBiconComp()
{
  for(int i=0; i<10; i++)
  {
    splitBiconComp();
    merge();
  }

  connectedComponents();
  biconnectedComponents();
}

LegoBrick *LegoCloud::addBrick(int level, int posX, int posY, int sizeX, int sizeY)
{
  assert(level < levelNumber_);

  LegoBrick brick(level, posX, posY, sizeX, sizeY);
  assert(!bricks_[level].contains(brick));

  //Insert the brick and recuperate the address of the inserted brick
  bricks_[level].push_back(brick);
  LegoBrick* newBrickPointer = &(bricks_[level].back());
  //assert(newBrickIt != bricks_[level].constEnd());

  brickNumber_[brick.getSize()]++;

  //GRAPH
  assert(!brickToVertex_.contains(newBrickPointer));
  LegoGraph::vertex_descriptor vertex = boost::add_vertex(graph_);
  graph_[vertex].brick = newBrickPointer;
  brickToVertex_.insert(newBrickPointer, vertex);

  return newBrickPointer;
}

bool LegoCloud::removeBrick(LegoBrick *brick)
{
  const QSet<LegoBrick*>& neighbours = getNeighbours(brick);//Save the neighbourhood

  //For each of the neighbours, we must remove the pointer to the old brick
  foreach(LegoBrick *neighbour, neighbours)
  {
    neighbourhood_[neighbour].remove(brick);
  }

  neighbourhood_.remove(brick);//Remove the neighbourhood of the brick

  assert(brickToVertex_.contains(brick));
  boost::clear_vertex(brickToVertex_[brick], graph_);
  boost::remove_vertex(brickToVertex_[brick], graph_);
  brickToVertex_.remove(brick);

  if(brick->isOuter())
    outerBricks_.removeOne(brick);
  else
    innerBricks_.removeOne(brick);

  //Decrement the number of this brick type
  brickNumber_[brick->getSize()]--;

  bool removeOk = bricks_[brick->getLevel()].removeOne(*brick);//Remove the brick
  assert(removeOk);

  return removeOk;
}

LegoBrick* LegoCloud::mergeBricks(const QSet<LegoBrick *> brickToMerge)//The signature of this method should be changed to (LegoBrick * a, LegoBrick * b)
{
  assert(brickToMerge.size() == 2);

  const int LEVEL = (*(brickToMerge.begin()))->getLevel();
  assert(LEVEL < levelNumber_);


#ifdef DEBUG
  Q_FOREACH(LegoBrick* brick, brickToMerge)
  {
    assert(bricks_[LEVEL].contains(*brick));//Trying to merge bricks on different levels or unexisting bricks
  }
#endif

  int minX = INT_MAX;
  int maxX = INT_MIN;

  int minY = INT_MAX;
  int maxY = INT_MIN;

  int totalKnobNumber = 0;
  foreach(LegoBrick* brick, brickToMerge)
  {
    if(brick->getPosX() < minX)
      minX = brick->getPosX();
    if(brick->getPosX()+brick->getSizeX() > maxX)
      maxX = brick->getPosX()+brick->getSizeX();

    if(brick->getPosY() < minY)
      minY = brick->getPosY();
    if(brick->getPosY()+brick->getSizeY() > maxY)
      maxY = brick->getPosY()+brick->getSizeY();

    totalKnobNumber += brick->getSizeX()*brick->getSizeY();
  }

  int newBrickSizeX = maxX - minX;
  int newBrickSizeY = maxY - minY;
  assert(newBrickSizeX > 0);
  assert(newBrickSizeY > 0);

  //Check for missing bricks:
  assert(totalKnobNumber == newBrickSizeX*newBrickSizeY);//Trying to merge uncompatible bricks(missing knobs)


  //Check that this brick exists:
  assert(legalBricks_.contains(BrickSize(newBrickSizeX, newBrickSizeY)) ||
         legalBricks_.contains(BrickSize(newBrickSizeY, newBrickSizeX)));//Trying to merge uncompatible bricks

  //****OK, merge can begin****

  LegoBrick* newBrick = addBrick(LEVEL, minX, minY, newBrickSizeX, newBrickSizeY);

  QSet<LegoBrick*> newNeighbours;

  bool isOuter = false;
  int newColorId = -1;

  foreach(LegoBrick *brick, brickToMerge)
  {
    newNeighbours.unite(neighbourhood_.value(brick));//The new neighbours are the neighbours of all bricks composing the new brick...

    if(brick->isOuter())
    {
      assert(newColorId == -1 || newColorId == brick->getColorId());//All the outer brick should have the same color
      newColorId = brick->getColorId();
    }

    isOuter = isOuter || brick->isOuter();//If one brick is outside, the new brick is also outside
  }

  if(newColorId != -1)
    newBrick->setColorId(newColorId);
  else
    newBrick->setColorId((*(brickToMerge.begin()))->getColorId());//If both bricks are inner bricks, we just pick one color

  newBrick->setIsOuter(isOuter);

  if(isOuter)
    outerBricks_.push_back(newBrick);
  else
    innerBricks_.push_back(newBrick);

  newNeighbours.subtract(brickToMerge);//... minus the composing bricks themselves

  //Not anymore:For each of the new neighbours, we must remove the dangling pointer to the deleted bricks and add the new brick as a neighbour
  foreach(LegoBrick *neighbour, newNeighbours)
  {
    //***neighbourhood_[neighbour].subtract(brickToMerge);
    neighbourhood_[neighbour].insert(newBrick);
  }

  neighbourhood_.insert(newBrick, newNeighbours);


  //GRAPH
  QSet<LegoGraph::vertex_descriptor> graphNewNeighbours;

  LegoGraph::adjacency_iterator neighbourIt, neighbourItEnd;
  foreach(LegoBrick *brick, brickToMerge)
  {
    for (boost::tie(neighbourIt, neighbourItEnd) = boost::adjacent_vertices(brickToVertex_[brick], graph_); neighbourIt != neighbourItEnd; ++neighbourIt)
    {
      graphNewNeighbours.insert(*neighbourIt);
    }
  }

  foreach(const LegoGraph::vertex_descriptor& newNeighbour, graphNewNeighbours)
  {
    boost::add_edge(brickToVertex_[newBrick], newNeighbour, graph_);
  }

  foreach(LegoBrick *brick, brickToMerge)
  {
    removeBrick(brick);
  }

  return newBrick;
}



bool LegoCloud::splitBrick(LegoBrick *brick)
{
  if(brick->getKnobNumber() == 1)
  {
    return false;//Brick of size 1x1 connot be split
  }

  int level = brick->getLevel();
  int oldBrickPosX = brick->getPosX();
  int oldBrickPosY = brick->getPosY();
  int oldBrickSizeX = brick->getSizeX();
  int oldBrickSizeY = brick->getSizeY();

  const QSet<LegoBrick*>& neighbours = getNeighbours(brick);//Save the neighbourhood


  QSet<LegoBrick*> newBricks;
  for(int x = oldBrickPosX; x < oldBrickPosX + oldBrickSizeX; x++)
  {
    for(int y = oldBrickPosY; y < oldBrickPosY + oldBrickSizeY; y++)
    {
      LegoBrick* newBrick = addBrick(level, x, y, 1, 1);
      newBrick->setColorId(brick->getColorId());
      newBricks.insert(newBrick);
      neighbourhood_.insert(newBrick, QSet<LegoBrick*>());//Add an empty set of neighbours for each new brick
    }
  }

  //This is all the bricks for which the neighbourhood must be created or completed
  QSet<LegoBrick*> potentialNeighbours = newBricks + neighbours;

  foreach(LegoBrick *newBrick, newBricks)
  {
    foreach(LegoBrick *potentialNeighbour, potentialNeighbours)
    {
      if(areNeighbours(newBrick, potentialNeighbour))
      {
        neighbourhood_[newBrick].insert(potentialNeighbour);
        neighbourhood_[potentialNeighbour].insert(newBrick);
      }
    }

    //TODO this is buggy when the model is hollow
    if(neighbourhood_[newBrick].size() < 4)
    {
      newBrick->setIsOuter(true);//The brick is outside if it has less than 4 neighbours
    }
  }

  assert(newBricks.size() == brick->getKnobNumber());


  //GRAPH

  LegoGraph::adjacency_iterator neighbourIt, neighbourItEnd;
  QSet<LegoGraph::vertex_descriptor> graphNeighbours;

  foreach(LegoBrick *newBrick, newBricks)
  {
    assert(boost::out_degree(brickToVertex_[newBrick], graph_) == 0);
    graphNeighbours.clear();

    for (boost::tie(neighbourIt, neighbourItEnd) = boost::adjacent_vertices(brickToVertex_[brick], graph_); neighbourIt != neighbourItEnd; ++neighbourIt)
    {
      if(areConnected(newBrick, graph_[*neighbourIt].brick))
        graphNeighbours.insert(*neighbourIt);
    }

    assert(graphNeighbours.size() <= 2);//a 1x1 brick can have maximum 2 connections

    if(graphNeighbours.size() < 2)
    {
      newBrick->setIsOuter(true);//The brick is outside if it has less 2 connections
    }

    if(newBrick->isOuter())
      outerBricks_.push_back(newBrick);
    else
      innerBricks_.push_back(newBrick);

    foreach(const LegoGraph::vertex_descriptor& neighbour, graphNeighbours)
    {
      boost::add_edge(brickToVertex_[newBrick], neighbour, graph_);
    }

  }

  removeBrick(brick);

  return true;
}

bool LegoCloud::areNeighbours(LegoBrick *brick1, LegoBrick *brick2)
{
  if(brick1->getLevel() != brick2->getLevel())//Must be on same level
    return false;

  if(brick1 == brick2)
    return false;

  for(int x1 = brick1->getPosX(); x1 < brick1->getPosX() + brick1->getSizeX(); x1++)
  {
    for(int y1 = brick1->getPosY(); y1 < brick1->getPosY() + brick1->getSizeY(); y1++)
    {
      for(int x2 = brick2->getPosX(); x2 < brick2->getPosX() + brick2->getSizeX(); x2++)
      {
        for(int y2 = brick2->getPosY(); y2 < brick2->getPosY() + brick2->getSizeY(); y2++)
        {
          if(
             (x1 == x2+1 && y1 == y2) ||
             (x1 == x2-1 && y1 == y2) ||
             (x1 == x2 && y1+1 == y2) ||
             (x1 == x2 && y1-1 == y2)
             )
            return true;
        }
      }
    }
  }

  return false;
}

bool LegoCloud::areConnected(const LegoBrick *brick1, const LegoBrick *brick2)
{
  if(brick1 == brick2)
    return false;

  if(brick1->getLevel() == brick2->getLevel())//Must be on different level
    return false;

  if(brick1->getLevel()-1 != brick2->getLevel() && brick1->getLevel()+1 != brick2->getLevel() )//Must be on adjacent level
    return false;


  for(int x1 = brick1->getPosX(); x1 < brick1->getPosX() + brick1->getSizeX(); x1++)
  {
    for(int y1 = brick1->getPosY(); y1 < brick1->getPosY() + brick1->getSizeY(); y1++)
    {
      for(int x2 = brick2->getPosX(); x2 < brick2->getPosX() + brick2->getSizeX(); x2++)
      {
        for(int y2 = brick2->getPosY(); y2 < brick2->getPosY() + brick2->getSizeY(); y2++)
        {
          if(x1 == x2 && y1 == y2)
            return true;
        }
      }
    }
  }

  return false;
}

bool LegoCloud::canMerge(LegoBrick *brick1, LegoBrick *brick2)
{
  assert(bricks_[brick1->getLevel()].contains(*brick1));
  assert(bricks_[brick2->getLevel()].contains(*brick2));

  if(brick1->getLevel() != brick1->getLevel())
  {
    std::cerr << "Trying to merge bricks on different levels" << std::endl;
    return false;
  }

  int minX = (brick1->getPosX() < brick2->getPosX()) ? brick1->getPosX() : brick2->getPosX();
  int maxX = (brick1->getPosX()+brick1->getSizeX() > brick2->getPosX()+brick2->getSizeX() ) ?
        brick1->getPosX()+brick1->getSizeX() : brick2->getPosX()+brick2->getSizeX();

  int minY = (brick1->getPosY() < brick2->getPosY()) ? brick1->getPosY() : brick2->getPosY();
  int maxY = (brick1->getPosY()+brick1->getSizeY() > brick2->getPosY()+brick2->getSizeY() ) ?
        brick1->getPosY()+brick1->getSizeY() : brick2->getPosY()+brick2->getSizeY();

  int totalKnobNumber = brick1->getSizeX()*brick1->getSizeY() + brick2->getSizeX()*brick2->getSizeY();

  int newBrickSizeX = maxX - minX;
  int newBrickSizeY = maxY - minY;
  assert(newBrickSizeX > 0);
  assert(newBrickSizeY > 0);

  //Check for missing bricks:
  if(totalKnobNumber < newBrickSizeX*newBrickSizeY)
  {
    //std::cerr << "Trying to merge uncompatible bricks(missing knobs)" << std::endl;
    return false;
  }

  //Swap if necessary
  if(newBrickSizeX > newBrickSizeY)
  {
    int temp = newBrickSizeX;
    newBrickSizeX = newBrickSizeY;
    newBrickSizeY = temp;
  }

  //Check that this brick exists:
  if(!legalBricks_.contains(BrickSize(newBrickSizeX, newBrickSizeY)))
  {
    //std::cerr << "Trying to merge uncompatible bricks" << std::endl;
    return false;
  }

  if(brick1->isOuter() && brick2->isOuter() && brick1->getColorId() != brick2->getColorId())
    return false;

  if(brickLimitConstraint_)
  {
    int limit = brickLimitation_[BrickSize(newBrickSizeX, newBrickSizeY)];
    if(limit != -1 && brickNumber_[BrickSize(newBrickSizeX, newBrickSizeY)] >= limit)
      return false;

  }

  return true;
}

//Returns the number of connections that merge of brick1 and brick2 will have
//First check that the two bricks can be merged (if not, return -1) and then return the number of connections that this brick will have.
int LegoCloud::connectionNumber(LegoBrick *brick1, LegoBrick *brick2)
{
  if(!canMerge(brick1, brick2))
    return -1;

  QSet<LegoGraph::vertex_descriptor> potentialNewNeighbour;
  LegoGraph::adjacency_iterator neighbourIt, neighbourItEnd;
  for (boost::tie(neighbourIt, neighbourItEnd) = boost::adjacent_vertices(brickToVertex_[brick1], graph_); neighbourIt != neighbourItEnd; ++neighbourIt)
  {
    potentialNewNeighbour.insert(*neighbourIt);
  }

  for (boost::tie(neighbourIt, neighbourItEnd) = boost::adjacent_vertices(brickToVertex_[brick2], graph_); neighbourIt != neighbourItEnd; ++neighbourIt)
  {
    potentialNewNeighbour.insert(*neighbourIt);
  }

  return potentialNewNeighbour.size();
}

LegoBrick *LegoCloud::findBestNeighbour(LegoBrick *brick, MergeStrategy strategy)
{
  const QSet<LegoBrick*>& neighbours = neighbourhood_.value(brick);
  if(neighbours.size() == 0)
  {
    return NULL;
  }

  if(strategy == Random)//Random neighbour
  {
    QList<LegoBrick*> possibleNeighbours;
    foreach(LegoBrick* neighbour, neighbours)
    {
      if(canMerge(brick, neighbour))
      {
        possibleNeighbours.append(neighbour);//Instead of randomly chosing one, we should consider brick type limit constraints
      }
    }

    if(possibleNeighbours.size() == 0)
      return NULL;
    else
      return possibleNeighbours.at(rand() % possibleNeighbours.size());
  }
  else if(strategy == MaxConnectivity)//Most connections after merge first
  {
    int bestConnectionNumber = -1;

    QList<LegoBrick*> bestNeighbours;
    foreach(LegoBrick* neighbour, neighbours)
    {
      int currentConNumber = connectionNumber(brick, neighbour);
      if(currentConNumber > bestConnectionNumber)
      {
        bestNeighbours.clear();
        bestNeighbours.append(neighbour);
        bestConnectionNumber = currentConNumber;
      }
      else if(bestConnectionNumber != -1 && currentConNumber == bestConnectionNumber)
      {
        bestNeighbours.append(neighbour);
      }
    }

    if(bestNeighbours.size() == 0)
      return NULL;
    else
      return bestNeighbours.at(rand() % bestNeighbours.size());//Instead of randomly chosing one, we should consider brick type limit constraints

  }

  return NULL;
}

bool LegoCloud::cutBrick(LegoBrick *oldBrick, QPair<LegoBrick, LegoBrick> newBricks)
{

  //add the 2 new bricks
  LegoBrick* newBrick1 = addBrick(newBricks.first.getLevel(), newBricks.first.getPosX(), newBricks.first.getPosY(), newBricks.first.getSizeX(), newBricks.first.getSizeY());
  LegoBrick* newBrick2 = addBrick(newBricks.second.getLevel(), newBricks.second.getPosX(), newBricks.second.getPosY(), newBricks.second.getSizeX(), newBricks.second.getSizeY());

  //Set the color of the new bricks to the color of the old brick
  newBrick1->setColorId(oldBrick->getColorId());
  newBrick2->setColorId(oldBrick->getColorId());

  //Create an empty neighbourhood for both
  neighbourhood_.insert(newBrick1, QSet<LegoBrick*>());
  neighbourhood_.insert(newBrick2, QSet<LegoBrick*>());

  //Save the neighbourhood of the old brick
  const QSet<LegoBrick*>& neighbours = getNeighbours(oldBrick);

  //Build there neighbourhood for both
  neighbourhood_[newBrick1].insert(newBrick2);
  neighbourhood_[newBrick2].insert(newBrick1);

  foreach(LegoBrick *potentialNeighbour, neighbours)
  {
    if(areNeighbours(newBrick1, potentialNeighbour))
    {
      neighbourhood_[newBrick1].insert(potentialNeighbour);
      neighbourhood_[potentialNeighbour].insert(newBrick1);
    }

    if(areNeighbours(newBrick2, potentialNeighbour))
    {
      neighbourhood_[newBrick2].insert(potentialNeighbour);
      neighbourhood_[potentialNeighbour].insert(newBrick2);
    }
  }

  if(oldBrick->isOuter())
  {
    //TODO this is wrong (one of the two might be an inner brick), but I don't know how to make it better
    newBrick1->setIsOuter(true);
    newBrick2->setIsOuter(true);
    outerBricks_.push_back(newBrick1);
    outerBricks_.push_back(newBrick2);
  }
  else
  {
    newBrick1->setIsOuter(false);
    newBrick2->setIsOuter(false);
    innerBricks_.push_back(newBrick1);
    innerBricks_.push_back(newBrick2);
  }


  //GRAPH:
  LegoGraph::adjacency_iterator neighbourIt, neighbourItEnd;
  for (boost::tie(neighbourIt, neighbourItEnd) = boost::adjacent_vertices(brickToVertex_[oldBrick], graph_); neighbourIt != neighbourItEnd; ++neighbourIt)
  {
    if(areConnected(newBrick1, graph_[*neighbourIt].brick))
      boost::add_edge(brickToVertex_[newBrick1], *neighbourIt, graph_);

    if(areConnected(newBrick2, graph_[*neighbourIt].brick))
      boost::add_edge(brickToVertex_[newBrick2], *neighbourIt, graph_);
  }

  removeBrick(oldBrick);

  return true;
}

QVector<QPair<LegoBrick, LegoBrick> > LegoCloud::possibleCuts(LegoBrick *brick)
{
  typedef QPair<LegoBrick, LegoBrick> Cut;
  QVector<Cut> cuts;

  for(int x = 1; x < brick->getSizeX(); x++)
  {
    LegoBrick brick1(brick->getLevel(), brick->getPosX(), brick->getPosY(), x, brick->getSizeY());
    LegoBrick brick2(brick->getLevel(), brick->getPosX() + x, brick->getPosY(), brick->getSizeX() - x, brick->getSizeY());

    //We must check that the cut is legal:
    if(legalBricks_.contains(brick1.getSize()) && legalBricks_.contains(brick2.getSize()))
      cuts.append(Cut(brick1, brick2));
  }

  for(int y = 1; y < brick->getSizeY(); y++)
  {
    LegoBrick brick1(brick->getLevel(), brick->getPosX(), brick->getPosY(), brick->getSizeX(), y);
    LegoBrick brick2(brick->getLevel(), brick->getPosX(), brick->getPosY() + y, brick->getSizeX(), brick->getSizeY() - y);

    //We must check that the cut is legal:
    if(legalBricks_.contains(brick1.getSize()) && legalBricks_.contains(brick2.getSize()))
      cuts.append(Cut(brick1, brick2));
  }

  return cuts;
}

int LegoCloud::findBestCut(LegoBrick *brick, const QVector<QPair<LegoBrick, LegoBrick> >& cuts)
{
  typedef QPair<LegoBrick, LegoBrick> Cut;
  typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS > Subgraph;

  Subgraph subgraph;
  QMap<LegoGraph::vertex_descriptor, Subgraph::vertex_descriptor> globalToLocal;
  QVector<LegoGraph::vertex_descriptor> oneRingNeighbours;

  //*** First, create an exact 2-ring subgraph around "brick"
  //Add the center vertex
  Subgraph::vertex_descriptor v0 = boost::add_vertex(subgraph);
  globalToLocal[brickToVertex_[brick]] = v0;

  //1-Ring
  LegoGraph::adjacency_iterator neighbourIt1, neighbourItEnd1;
  for (boost::tie(neighbourIt1, neighbourItEnd1) = boost::adjacent_vertices(brickToVertex_[brick], graph_); neighbourIt1 != neighbourItEnd1; ++neighbourIt1)
  {
    Subgraph::vertex_descriptor v1 = boost::add_vertex(subgraph);
    boost::add_edge(v0, v1, subgraph);

    oneRingNeighbours.append(*neighbourIt1);
    globalToLocal[*neighbourIt1] = v1;

    //2-Ring
    LegoGraph::adjacency_iterator neighbourIt2, neighbourItEnd2;
    for (boost::tie(neighbourIt2, neighbourItEnd2) = boost::adjacent_vertices(*neighbourIt1, graph_); neighbourIt2 != neighbourItEnd2; ++neighbourIt2)
    {

      if(!globalToLocal.contains(*neighbourIt2))//This 2-ring neighbour has not yet been encountered
      {
        Subgraph::vertex_descriptor v2 = boost::add_vertex(subgraph);
        globalToLocal[*neighbourIt2] = v2;
        boost::add_edge(v1, v2, subgraph);

        //3-Ring
        LegoGraph::adjacency_iterator neighbourIt3, neighbourItEnd3;
        for (boost::tie(neighbourIt3, neighbourItEnd3) = boost::adjacent_vertices(*neighbourIt2, graph_); neighbourIt3 != neighbourItEnd3; ++neighbourIt3)
        {
          if(!globalToLocal.contains(*neighbourIt3))//This 2-ring neighbour has not yet been encountered
          {
            Subgraph::vertex_descriptor v3 = boost::add_vertex(subgraph);
            globalToLocal[*neighbourIt3] = v3;
            boost::add_edge(v2, v3, subgraph);
          }
          else
          {
            boost::add_edge(v2, globalToLocal[*neighbourIt3], subgraph);
          }

        }
      }
      else
      {
        boost::add_edge(v1, globalToLocal[*neighbourIt2], subgraph);
      }



    }
  }

  //Done ***

  //*** Then create the vertex index map needed by the connected_components and biconnected_components algo

  typedef std::map<Subgraph::vertex_descriptor, int> VertexIndexMap;
  VertexIndexMap vertex_id_map;
  typedef boost::associative_property_map<VertexIndexMap> VertexIdPropertyMap;
  VertexIdPropertyMap vertex_index_pmap(vertex_id_map);
  int index = 0;
  Subgraph::vertex_iterator vertexIt, vertexItEnd;
  for (boost::tie(vertexIt, vertexItEnd) = boost::vertices(subgraph); vertexIt != vertexItEnd; ++vertexIt)
  {
    vertex_index_pmap[*vertexIt] = index;
    index++;
  }
  //Done ***

  //Compute the 2 values
  int mainConCompNumber = boost::connected_components(subgraph, boost::dummy_property_map(), boost::vertex_index_map(vertex_index_pmap));
  size_t mainBiconCompNumber = boost::biconnected_components(subgraph, boost::dummy_property_map(), boost::vertex_index_map(vertex_index_pmap));

  //Now remove the center vertex
  boost::clear_vertex(v0, subgraph);
  boost::remove_vertex(v0, subgraph);

  int bestCutConCompNumber = mainConCompNumber;
  size_t bestCutBiconCompNumber = mainBiconCompNumber;
  int bestCutIndex = -1;

  //Add 2 new vertices corresponding to the 2 bricks in a Cut
  Subgraph::vertex_descriptor v1 = boost::add_vertex(subgraph);//cut.first
  Subgraph::vertex_descriptor v2 = boost::add_vertex(subgraph);//cut.second

  //Rebuild the vertex index map that will be used later
  index = 0;
  vertex_id_map.clear();
  for (boost::tie(vertexIt, vertexItEnd) = boost::vertices(subgraph); vertexIt != vertexItEnd; ++vertexIt)
  {
    vertex_index_pmap[*vertexIt] = index;
    index++;
  }

  //Iterate over all possible cuts
  for(int cutIndex = 0; cutIndex < cuts.size(); cutIndex++)
  {
    //For both new vertices v1 and v2, build their connectivity
    foreach(LegoGraph::vertex_descriptor neighbour, oneRingNeighbours)
    {
      if(areConnected(&(cuts[cutIndex].first), graph_[neighbour].brick))
      {
        boost::add_edge(v1, globalToLocal[neighbour], subgraph);
      }

      if(areConnected(&(cuts[cutIndex].second), graph_[neighbour].brick))
      {
        boost::add_edge(v2, globalToLocal[neighbour], subgraph);
      }
    }



    //Compute the 2 values for this graph configuration
    int currentConCompNumber = boost::connected_components(subgraph, boost::dummy_property_map(), boost::vertex_index_map(vertex_index_pmap));
    size_t currentBiconCompNumber = boost::biconnected_components(subgraph, boost::dummy_property_map(), boost::vertex_index_map(vertex_index_pmap));

    //We are finished with this cut: clear their edges to prepare for the next
    boost::clear_vertex(v1, subgraph);
    boost::clear_vertex(v2, subgraph);

    //Remember the best cut
    if(currentConCompNumber <= bestCutConCompNumber && currentBiconCompNumber <= bestCutBiconCompNumber)
    {
      bestCutConCompNumber = currentConCompNumber;
      bestCutBiconCompNumber = currentBiconCompNumber;
      bestCutIndex = cutIndex;
    }

  }

  return bestCutIndex;
}

bool LegoCloud::canRemoveBrick(LegoBrick *brick)
{
  typedef boost::adjacency_list<boost::listS, boost::listS, boost::undirectedS > Subgraph;

  Subgraph subgraph;
  QMap<LegoGraph::vertex_descriptor, Subgraph::vertex_descriptor> globalToLocal;

  //*** First, create an exact 2-ring subgraph around "brick"
  //Add the center vertex
  Subgraph::vertex_descriptor v0 = boost::add_vertex(subgraph);
  globalToLocal[brickToVertex_[brick]] = v0;

  //1-Ring
  LegoGraph::adjacency_iterator neighbourIt1, neighbourItEnd1;
  for (boost::tie(neighbourIt1, neighbourItEnd1) = boost::adjacent_vertices(brickToVertex_[brick], graph_); neighbourIt1 != neighbourItEnd1; ++neighbourIt1)
  {
    Subgraph::vertex_descriptor v1 = boost::add_vertex(subgraph);
    boost::add_edge(v0, v1, subgraph);
    globalToLocal[*neighbourIt1] = v1;

    //2-Ring
    LegoGraph::adjacency_iterator neighbourIt2, neighbourItEnd2;
    for (boost::tie(neighbourIt2, neighbourItEnd2) = boost::adjacent_vertices(*neighbourIt1, graph_); neighbourIt2 != neighbourItEnd2; ++neighbourIt2)
    {
      //if(*neighbourIt2 != brickToVertex_[brick])//We must discard the center vertex
      {
        if(!globalToLocal.contains(*neighbourIt2))//This 2-ring neighbour has not yet been encountered
        {
          Subgraph::vertex_descriptor v2 = boost::add_vertex(subgraph);
          globalToLocal[*neighbourIt2] = v2;
          boost::add_edge(v1, v2, subgraph);

          //3-Ring
          LegoGraph::adjacency_iterator neighbourIt3, neighbourItEnd3;
          for (boost::tie(neighbourIt3, neighbourItEnd3) = boost::adjacent_vertices(*neighbourIt2, graph_); neighbourIt3 != neighbourItEnd3; ++neighbourIt3)
          {
            if(!globalToLocal.contains(*neighbourIt3))//This 3-ring neighbour has not yet been encountered
            {
              Subgraph::vertex_descriptor v3 = boost::add_vertex(subgraph);
              globalToLocal[*neighbourIt3] = v3;
              boost::add_edge(v2, v3, subgraph);
            }
            else
            {
              boost::add_edge(v2, globalToLocal[*neighbourIt3], subgraph);
            }

          }
        }
        else
        {
          boost::add_edge(v1, globalToLocal[*neighbourIt2], subgraph);
        }
      }


    }
  }

  //Done ***

  //*** Then create the vertex index map needed by the connected_components and biconnected_components algo

  typedef std::map<Subgraph::vertex_descriptor, int> VertexIndexMap;
  VertexIndexMap vertex_id_map;
  typedef boost::associative_property_map<VertexIndexMap> VertexIdPropertyMap;
  VertexIdPropertyMap vertex_index_pmap(vertex_id_map);
  int index = 0;
  Subgraph::vertex_iterator vertexIt, vertexItEnd;
  for (boost::tie(vertexIt, vertexItEnd) = boost::vertices(subgraph); vertexIt != vertexItEnd; ++vertexIt)
  {
    vertex_index_pmap[*vertexIt] = index;
    index++;
  }
  //Done ***

  //Compute the 2 values
  int beforeConCompNumber = boost::connected_components(subgraph, boost::dummy_property_map(), boost::vertex_index_map(vertex_index_pmap));
  size_t beforeBiconCompNumber = boost::biconnected_components(subgraph, boost::dummy_property_map(), boost::vertex_index_map(vertex_index_pmap));

  //Now remove the center vertex
  boost::clear_vertex(v0, subgraph);
  boost::remove_vertex(v0, subgraph);

  //Rebuild the vertex index map that will be used later
  index = 0;
  vertex_id_map.clear();
  for (boost::tie(vertexIt, vertexItEnd) = boost::vertices(subgraph); vertexIt != vertexItEnd; ++vertexIt)
  {
    vertex_index_pmap[*vertexIt] = index;
    index++;
  }

  //Compute the 2 values for this graph without the center brick
  int afterConCompNumber = boost::connected_components(subgraph, boost::dummy_property_map(), boost::vertex_index_map(vertex_index_pmap));
  size_t afterBiconCompNumber = boost::biconnected_components(subgraph, boost::dummy_property_map(), boost::vertex_index_map(vertex_index_pmap));

  if(afterConCompNumber <= beforeConCompNumber && afterBiconCompNumber <= beforeBiconCompNumber)
    return true;
  else
    return false;
}
