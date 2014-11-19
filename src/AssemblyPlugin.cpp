#include "AssemblyPlugin.h"

#include "AssemblyWidget.h"
#include "LegoCloudNode.h"
#include "LegoCloud.h"
#include "LegoBrick.h"

#include <fstream>
#include <limits.h>
#include <QSet>
#include <QFileInfo>
#include <QTime>

#define AUTO_OPTIMIZE_MAX_STEPS 50

AssemblyPlugin::AssemblyPlugin()
  : assemblyWidget_(0) {
}

AssemblyPlugin::~AssemblyPlugin()
{
}

//Button slot
void AssemblyPlugin::test(int x, int y, int z)
{

  if(x < 0 || y < 0 || z < 0)
  {
    std::cerr << "The test values must all be positive" << std::endl;
    return;
  }

  legoCloudNode_ = std::make_shared<LegoCloudNode>();

  int height = y;
  int width = x;
  int depth = z;

  legoCloudNode_->getLegoCloud()->setVoxelGridDimmension(height, width, depth);
  for(int level=0; level < height; level++)
  {
    for(int x = 0; x < width; ++x)
    {
      for(int y = 0; y < depth; ++y)
      {
        LegoBrick* brick = legoCloudNode_->getLegoCloud()->addBrick(level, x, y);
        legoCloudNode_->getLegoCloud()->addVoxel(level, x, y, brick);
      }
    }
  }

  legoCloudNode_->getLegoCloud()->buildNeighbourhood();

  legoCloudNode_->nodeUpdated();

  emit geometryChanged();

  if (assemblyWidget_)
    assemblyWidget_->setMaxLayerSpinBox( legoCloudNode_->getLegoCloud()->getLevelNumber());
}

void AssemblyPlugin::loadVoxelization(QString filename)
{
  //No file selected
  if(filename == NULL)
    return;

  legoCloudNode_ = std::make_shared<LegoCloudNode>();


  std::cout << "Opening file: " << qPrintable(filename) << std::endl;
  parseBinvox(filename.toStdString(), legoCloudNode_.get());
  legoCloudNode_->getLegoCloud()->buildNeighbourhood();

  legoCloudNode_->nodeUpdated();

  emit geometryChanged();

  if (assemblyWidget_)
    assemblyWidget_->setMaxLayerSpinBox(legoCloudNode_->getLegoCloud()->getLevelNumber());
}

/*
void AssemblyPlugin::loadObj(QString fileName)
{
  //add to scenegraph
  OpenMeshNode *openMeshNode = new OpenMeshNode(&getScenegraph(), mesh);
  openMeshNode->setName(fileinfo.baseName());
  openMeshNode->setVisible(false,false);
  openMeshNode->setSelected(false,false);

  //*** Scale the mesh to the same size as the legoCloud
  typedef Dolphin::defines::Vector3 Vector3;
  Vector3 minLego = legoCloudNode->getAABB()->getMin();
  Vector3 maxLego = legoCloudNode->getAABB()->getMax();

  Vector3 minMesh = openMeshNode->getAABB()->getMin();
  Vector3 maxMesh = openMeshNode->getAABB()->getMax();

  OMesh::VertexIter v_it, v_end(mesh->vertices_end());
  for (v_it=mesh->vertices_begin(); v_it!=v_end; ++v_it)
  {
    //mesh->set_point(v_it, (mesh->point(v_it) + translation) * ((maxLego.x()-minLego.x())/(maxMesh.x()-minMesh.x())));
    mesh->set_point(v_it,
                    (mesh->point(v_it) - (minMesh+maxMesh)/2.0) * //Center around 0
                    ((maxLego.x()-minLego.x())/(maxMesh.x()-minMesh.x())) //Scale
                    + (minLego+maxLego)/2.0); //Translate on Legos
  }
  //***

  mesh->update_face_normals();

  openMeshNode->getDrawModes()->setMode(DrawModes::FACE_SHADING_FLAT);
  openMeshNode->getDrawModes()->setMode(DrawModes::HALFEDGE_TEXCOORDS);

  openMeshNode->setGeometryChanged();
}

void AssemblyPlugin::loadTexture(QString fileName)
{
  QFileInfo fileinfo(fileName);

  if(!fileinfo.isFile() || ! fileinfo.isReadable())
  {
    std::cerr << "Assembly Plugin > Unable to open file: " << fileName.toStdString().c_str() << std::endl;
    return;
  }

  OpenMeshNode* oMeshNode = getScenegraph().getFirstDescendant<OpenMeshNode*>(false);
  if( !oMeshNode )
  {
    std::cerr << "AssemblyPlugin: There is no OpenMeshNode in the scenegraph: aborting.\n";
    return;
  }

  Dolphin::scenegraph::Texture& texture = *(oMeshNode->getTexture());
  Dolphin::scenegraph::TextureProps& texProps = texture.getTextureProps();
  texProps.textureLoadType_ = Dolphin::scenegraph::TextureProps::SOIL_TYPE_MESH_TEX_COORD;
  texProps.textureLayoutFlags_ |= TextureProps::SOIL_FLAG_INVERT_Y;//Not sure why this is needed...
  texProps.textureLayoutFlags_ |= TextureProps::SOIL_FLAG_POWER_OF_TWO;//Force the texture to be power of 2 (e.g. 512x512) (I don't know why it is needed, but otherwise the mesh does not display properly)
  texProps.textureFileName = fileName.toStdString();

  OpenMeshNode* opmn=NULL;
  Dolphin::gui::OpenMeshNodeTextureDialog::loadTextureToObject(oMeshNode, opmn);


  LegoCloudNode* legoCloudNode = getLegoCloudNode(true);
  legoCloudNode->getLegoCloud()->loadColors(oMeshNode);
}

void AssemblyPlugin::removeAllMeshes()
{
  QList<OpenMeshNode*> meshList;
  getScenegraph().getDescendants<OpenMeshNode*>(meshList, false);

  foreach (OpenMeshNode* mesh, meshList)
  {
    mesh->deleteLater();
    mesh->setParent(NULL);
  }
}
*/

bool AssemblyPlugin::parseBinvox(const std::string& filename, LegoCloudNode* legoCloudNode)
{
  //Credit: http://www.google.com/search?q=binvox
  typedef unsigned char byte;

  int version;
  int size;
  float tx, ty, tz;
  float scale;
  int depth, width, height;

  std::ifstream input;
  input.open(filename.c_str());
  if (!input.is_open())
    return false;

  // read header
  std::string line;
  input >> line;  // #binvox
  if (line.compare("#binvox") != 0) {
    std::cerr << "Error: first line reads [" << line.c_str() << "] instead of [#binvox]" << std::endl;
    return false;
  }

  input >> version;
  //cout << "reading binvox version " << version << endl;

  depth = -1;
  int done = 0;
  while(input.good() && !done) {
    input >> line;
    if (line.compare("data") == 0) done = 1;
    else if (line.compare("dim") == 0) {
      input >> depth >> height >> width;
    }
    else if (line.compare("translate") == 0) {
      input >> tx >> ty >> tz;
    }
    else if (line.compare("scale") == 0) {
      input >> scale;
    }
    else {
      std::cerr << "  unrecognized keyword [" << line.c_str() << "], skipping" << std::endl;
      char c;
      do {  // skip until end of line
        c = input.get();
      } while(input.good() && (c != '\n'));

    }
  }
  if (!done) {
    std::cerr << "  error reading header" << std::endl;
    return false;
  }
  if (depth == -1) {
    std::cerr << "  missing dimensions in header" << std::endl;
    return false;
  }

  size = width * height * depth;
  legoCloudNode->getLegoCloud()->setVoxelGridDimmension(height, width, depth);

  //
  // read voxel data
  //
  byte value;
  byte count;
  int index = 0;
  int end_index = 0;
  int nr_voxels = 0;

  input.unsetf(std::ios::skipws);  // need to read every byte now (!)
  input >> value;  // read the linefeed char

  while((end_index < size) && input.good()) {
    input >> value >> count;

    if (input.good()) {
      end_index = index + count;
      if (end_index > size) return false;
      for(int i=index; i < end_index; i++) {
        if(value == 1)
        {
          int level = (i%(width*height))%height;
          int y = ((i-level)%(width*height))/height;
          int x = (i - level - y*height)/(width*height);
          LegoBrick* brick = legoCloudNode->getLegoCloud()->addBrick(level, x, y);
          legoCloudNode->getLegoCloud()->addVoxel(level, x, y, brick);
        } else {
          //do nothing
        }
      }

      if (value) nr_voxels += count;
      index = end_index;
    }  // if file still ok

  }  // while

  input.close();
  std::cout << "  read " << nr_voxels << " voxels" << std::endl;

  return true;
}

QPair<float, QPair<int, int> > AssemblyPlugin::autoOptimize()
{
  if(!legoCloudNode_)
    return QPair<float, QPair<int, int> >();

  LegoCloud* legoCloud = legoCloudNode_->getLegoCloud();

//  progress::setNumberOfSteps(AUTO_OPTIMIZE_MAX_STEPS, "Optimizing...");
//  progress::setProgress(0);

  std::cout << "Optimization started, please wait..." << std::endl;
  QTime time;
  time.start();

  //Step1: merge
  legoCloud->merge();

  int conCompNumber = legoCloud->getConCompNumber();
  int minConCompNumber = conCompNumber;
  int iterationCon = 0;
  int totalConCompIter = 0;
  int totalArtPointIter = 0;

  //Step2: find the minimum number of connected components
  while(iterationCon < AUTO_OPTIMIZE_MAX_STEPS*2 && (conCompNumber > 1 || conCompNumber > minConCompNumber))
  {
    legoCloud->splitConComp();
    legoCloud->merge();
    conCompNumber = legoCloud->getConCompNumber();

    if(conCompNumber < minConCompNumber)
      minConCompNumber = conCompNumber;

    iterationCon++;
    totalConCompIter++;
  }

  //std::cout << minConCompNumber << " " << legoCloud->getConCompNumber() << std::endl;

  //Step3: reduce the bad articulation points then if the connected component number increase try to reduce it
  int badArtPointNumber = legoCloud->getBadArtPointNumber();
  int iterationBicon = 0;
  while(badArtPointNumber > 0 && iterationBicon < AUTO_OPTIMIZE_MAX_STEPS)
  {
    legoCloud->splitBiconComp();
    legoCloud->merge();
    conCompNumber = legoCloud->getConCompNumber();
    badArtPointNumber = legoCloud->getBadArtPointNumber();

    if(conCompNumber < minConCompNumber)
      minConCompNumber = conCompNumber;

    iterationCon = 0;
    while(conCompNumber > minConCompNumber && iterationCon < AUTO_OPTIMIZE_MAX_STEPS*2)
    {
      legoCloud->splitConComp();
      legoCloud->merge();
      conCompNumber = legoCloud->getConCompNumber();
      badArtPointNumber = legoCloud->getBadArtPointNumber();

      iterationCon++;
      totalConCompIter++;
    }

    iterationBicon++;
    totalArtPointIter++;
//    progress::setProgress(iterationBicon);
  }

  //int end = QTime::currentTime().msec();

  //std::cout << "conIter: " << totalConCompIter << ", artIter: " << totalArtPointIter << std::endl;

  badArtPointNumber = legoCloud->getBadArtPointNumber();
//  progress::finish();

  legoCloudNode_->nodeUpdated();

  std::cout << "Optimization ended; results:" << std::endl;

  std::cout << "Brick count: " << legoCloud->getBrickNumber() << std::endl;

  if(conCompNumber == 1)
  {
    std::cout << "This model is fully connected." << std::endl;
  }
  else
  {
    std::cout << "   There are " << conCompNumber << " connected components." << std::endl;
    std::cout << "   Warning, there are more than 1 connected component, this model will have disconnected pieces. Consider doing another optimization." << std::endl;
  }

  if(badArtPointNumber == 0)
  {
    std::cout << "There are no weak articulation points." << std::endl;
  }
  else
  {
    std::cout << "   There are " << badArtPointNumber << " weak articulation points." << std::endl;
    std::cout << "   Warning, there are some weak articulation points in this model. Consider doing another optimization." << std::endl;
  }

  std::cout << "Time: " << time.elapsed()/1000.0 << " seconds." << std::endl;

  return QPair<float, QPair<int, int> >(time.elapsed()/1000.0, QPair<int, int>(totalConCompIter, totalArtPointIter));
}

void AssemblyPlugin::draw()
{
  if (legoCloudNode_) {
//    std::cout << legoCloudNode_->minPoint() << "; " << legoCloudNode_->maxPoint() << std::endl;
    legoCloudNode_->render();
  }
}

