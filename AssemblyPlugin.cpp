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

#include "AssemblyPlugin.h"


#include "AssemblyWidget.h"
#include "LegoCloudNode.h"
#include "LegoCloud.h"
#include "LegoBrick.h"

#include <Dolphin/Core/Scenegraph/Scenegraph.h>

// progress & stream
#include <Dolphin/Core/Utilities/dolphinstream.h>
#include <Dolphin/Core/Utilities/dolphinprogress.h>

#include <Utilities/Geometry/IO/IOUtility.h>
#include <Utilities/Geometry/IO/ReadOptions.h>
#include <Dolphin/Core/Scenegraph/Geometry/OpenMeshNode.h>
#include <Dolphin/GUI/Qt4/Widgets/NodeDialogs/OpenMeshNodeDialog/OpenMeshNodeTextureDialog.h>
#include <Dolphin/Core/DataStructures/AABB.h>

#include <fstream>
#include <limits.h>
#include <QSet>
#include <QFileInfo>
#include <QTime>

#define AUTO_OPTIMIZE_MAX_STEPS 50


namespace Dolphin { namespace plugins {

  AssemblyPlugin::AssemblyPlugin()
                 : assemblyWidget_(new AssemblyWidget(*this)){
  }

  void AssemblyPlugin::init(MainApplication *_mainApplication)
  {
    Plugin_interface::init(_mainApplication);

    //maxi widget
    //AssemblyWidget_->readSettings(getSettings());
    setMaxiWidget(assemblyWidget_.get());
   }


  AssemblyPlugin::~AssemblyPlugin()
  {
  }

  void AssemblyPlugin::onUnload()
  {
    //AssemblyWidget_->writeSettings(getSettings());
    removeAllActions();
  }

  //Button slot
  void AssemblyPlugin::test(int x, int y, int z)
  {

    if(x < 0 || y < 0 || z < 0)
    {
      dolphinErr() << "The test values must all be positive" << std::endl;
      return;
    }

    LegoCloudNode* oldLegoCloudNode = getFirstSelectedLegoCloudNode(false);
    if(oldLegoCloudNode)
    {
      oldLegoCloudNode->deleteLater();
      oldLegoCloudNode->setParent(NULL);
    }

    LegoCloudNode* legoCloudNode = new LegoCloudNode(&getScenegraph());
    legoCloudNode->setName(QString("Test"));

    int height = y;
    int width = x;
    int depth = z;

    legoCloudNode->getLegoCloud()->setVoxelGridDimmension(height, width, depth);
    for(int level=0; level < height; level++)
    {
      for(int x = 0; x < width; ++x)
      {
        for(int y = 0; y <depth; ++y)
        {
          LegoBrick* brick = legoCloudNode->getLegoCloud()->addBrick(level, x, y);
          legoCloudNode->getLegoCloud()->addVoxel(level, x, y, brick);
        }
      }
    }

    legoCloudNode->getLegoCloud()->buildNeighbourhood();

    //legoCloudNode->nodeUpdated();
    legoCloudNode->setGeometryChanged();
    legoCloudNode->setSelected(true, false);

    resetExaminerToBoundingSphere();

    assemblyWidget_->setMaxLayerSpinBox( legoCloudNode->getLegoCloud()->getLevelNumber());
  }

  void AssemblyPlugin::loadVoxelization(QString filename)
  {
    //No file selected
    if(filename == NULL)
      return;

    LegoCloudNode* oldLegoCloudNode = getFirstSelectedLegoCloudNode(false);
    if(oldLegoCloudNode)
    {
      oldLegoCloudNode->deleteLater();
      oldLegoCloudNode->setParent(NULL);
    }

    LegoCloudNode* legoCloudNode = new LegoCloudNode(&getScenegraph());
    legoCloudNode->setSelected(true);

    QFileInfo fileInfo(filename);
    legoCloudNode->setName(fileInfo.baseName());


    dolphinOut() << "Openning file: " << qPrintable(filename) << std::endl;
    parseBinvox(filename.toStdString(), legoCloudNode);
    legoCloudNode->getLegoCloud()->buildNeighbourhood();

    legoCloudNode->setGeometryChanged();
    resetExaminerToBoundingSphere();

    assemblyWidget_->setMaxLayerSpinBox(legoCloudNode->getLegoCloud()->getLevelNumber());
  }

  void AssemblyPlugin::loadObj(QString fileName)
  {
    QFileInfo fileinfo(fileName);

    if(!fileinfo.isFile() || ! fileinfo.isReadable())
    {
      dolphinErr() << "Assembly Plugin > Unable to open file: " << fileName.toStdString().c_str() << std::endl;
      return;
    }

    LegoCloudNode* legoCloudNode = getFirstSelectedLegoCloudNode();
    if(!legoCloudNode)
      return;

    std::tr1::shared_ptr<OMesh> mesh(new OMesh);

    typedef Dolphin::defines::Vector2 Vector2;
    typedef Dolphin::defines::Vector3 Vector3;

    //*** Parse obj (code adapted from Mario Deuss)
    std::vector<Vector3> vertices;
    std::vector<Vector2> txCoords;
    //std::vector<Vector3> normals;
    std::vector<OpenMesh::VectorT<OpenMesh::VectorT<int,3>,3 > > faces;

    string line;
    ifstream myfile(fileName.toStdString().c_str());
    if(!myfile.is_open())
    {
      dolphinErr() << "Assembly Plugin > Unable to open file: " << fileName.toStdString().c_str() << std::endl;
      return;
    }

    bool hasNormal = false;

    while ( myfile.good() )
    {
      std::getline (myfile,line);
      if(line.empty()) continue;
      if( !(line[0]=='v' || line[0]=='f')) continue;

      string token;
      stringstream sl(line);
      sl >> token;

      if(!token.compare("v")){
        Vector3 v;
        for(int i=0;i<3;i++) sl >> v[i];
        vertices.push_back(v);
      }

      if(!token.compare("vn")){
        hasNormal = true;
        /*Vector3 n;
        for(int i=0;i<3;i++) sl >> n[i];
        normals.push_back(n);*/
      }

      if(!token.compare("vt")){
        Vector2 t;
        for(int i=0;i<2;i++) sl >> t[i];
        txCoords.push_back(t);
      }

      if(!token.compare("f")){
        OpenMesh::VectorT<OpenMesh::VectorT<int,3>,3 > f;

        int i=0;
        for(int c=0;c<3;c++){
          OpenMesh::VectorT<int,3> v;
          string vertex;
          getline(sl,vertex,'/');
          stringstream ss(vertex);
          ss >> v[0];

          if(hasNormal)
          {
            getline(sl,vertex,'/');
            stringstream ss2(vertex);
            ss2 >> v[1];

            getline(sl,vertex,' ');
            stringstream ss3(vertex);
            ss3 >> v[2];
          }
          else
          {
            getline(sl,vertex,' ');
            stringstream ss3(vertex);
            ss3 >> v[1];
          }

          f[i]=v;
          i++;
        }
        faces.push_back(f);
      }
    }
    myfile.close();


    //Now we construct the mesh:
    mesh->request_halfedge_texcoords2D();
    //mesh->request_vertex_normals();

    assert(mesh->n_vertices()==0);
    for(std::vector<Vector3>::iterator it=vertices.begin();it!=vertices.end();++it)
      mesh->add_vertex(*it);


    for(std::vector<OpenMesh::VectorT<OpenMesh::VectorT<int, 3>, 3 > >::iterator it=faces.begin();it!=faces.end();++it){
      OpenMesh::VectorT<OpenMesh::VectorT<int, 3>, 3 >& f = (*it);
      OMesh::VertexHandle v0,v1,v2;
      v0 = OMesh::VertexHandle(f[0][0] - 1);
      v1 = OMesh::VertexHandle(f[1][0] - 1);
      v2 = OMesh::VertexHandle(f[2][0] - 1);
      OMesh::FaceHandle fh=mesh->add_face(v0,v1,v2);

      if(!fh.is_valid())
      {
        dolphinErr() << "AssemblyPlugin: Invalid face handle (the mesh might be non-manifold or not triangulated)." << std::endl;
        assert(false && "AssemblyPlugin: Invalid face handle (the mesh might be non-manifold or not triangulated).");
      }

      //mesh->set_normal(v0,normals[f[0][2]-1]);
      //mesh->set_normal(v1,normals[f[1][2]-1]);
      //mesh->set_normal(v2,normals[f[2][2]-1]);

      unsigned int i = 0;
      for(OMesh::FaceHalfedgeIter fhI=mesh->fh_iter(fh); fhI; ++fhI,++i){
        unsigned int itxc = f[i][1] - 1;
        if(itxc >= txCoords.size()){
          dolphinErr() << "AssemblyPlugin: Number of texture coordinates smaller than biggest halfedge index.";
          assert(false && "AssemblyPlugin: Number of texture coordinates smaller than biggest halfedge index.");
          return;
        }
        mesh->set_texcoord2D(fhI, txCoords[itxc]);
      }
    }

    //mesh->update_vertex_normals();

    dolphinOut() << "Loaded a mesh with: " << mesh->n_vertices() << " vertices, " << mesh->n_edges() << " edges, " << mesh->n_faces() << " faces." << std::endl;

    //***

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
      dolphinErr() << "Assembly Plugin > Unable to open file: " << fileName.toStdString().c_str() << std::endl;
      return;
    }

    OpenMeshNode* oMeshNode = getScenegraph().getFirstDescendant<OpenMeshNode*>(false);
    if( !oMeshNode )
    {
      dolphinErr() << "AssemblyPlugin: There is no OpenMeshNode in the scenegraph: aborting.\n";
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


    LegoCloudNode* legoCloudNode = getFirstSelectedLegoCloudNode(true);
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
    string line;
    input >> line;  // #binvox
    if (line.compare("#binvox") != 0) {
      dolphinErr() << "Error: first line reads [" << line.c_str() << "] instead of [#binvox]" << std::endl;
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
        dolphinErr() << "  unrecognized keyword [" << line.c_str() << "], skipping" << endl;
        char c;
        do {  // skip until end of line
          c = input.get();
        } while(input.good() && (c != '\n'));

      }
    }
    if (!done) {
      dolphinErr() << "  error reading header" << endl;
      return false;
    }
    if (depth == -1) {
      dolphinErr() << "  missing dimensions in header" << endl;
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

    input.unsetf(ios::skipws);  // need to read every byte now (!)
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
    dolphinOut() << "  read " << nr_voxels << " voxels" << endl;

    return true;
  }

  LegoCloudNode *AssemblyPlugin::getFirstSelectedLegoCloudNode(bool debug)
  {
    LegoCloudNode *legoCloudNode = getScenegraph().getFirstDescendant<LegoCloudNode*>(true);
    if( !legoCloudNode )
    {
      if(debug)
        dolphinErr() << "AssemblyPlugin: No lego cloud selected.\n";
      return NULL;
    }
    return legoCloudNode;
  }

  QPair<float, QPair<int, int> > AssemblyPlugin::autoOptimize()
  {
    LegoCloudNode* legoCloudNode = getFirstSelectedLegoCloudNode();
    if(!legoCloudNode)
      return QPair<float, QPair<int, int> >();

    LegoCloud* legoCloud = legoCloudNode->getLegoCloud();

    progress::setNumberOfSteps(AUTO_OPTIMIZE_MAX_STEPS, "Optimizing...");
    progress::setProgress(0);

    dolphinOut() << "Optimization started, please wait..." << std::endl;
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

    //dolphinOut() << minConCompNumber << " " << legoCloud->getConCompNumber() << std::endl;

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
      progress::setProgress(iterationBicon);
    }

    //int end = QTime::currentTime().msec();

    //dolphinOut() << "conIter: " << totalConCompIter << ", artIter: " << totalArtPointIter << std::endl;

    badArtPointNumber = legoCloud->getBadArtPointNumber();
    progress::finish();

    legoCloudNode->nodeUpdated();

    dolphinOut() << "Optimization ended; results:" << std::endl;

    dolphinOut() << "Brick count: " << legoCloud->getBrickNumber() << std::endl;

    if(conCompNumber == 1)
    {
      dolphinOut() << "This model is fully connected." << std::endl;
    }
    else
    {
      dolphinOut() << "   There are " << conCompNumber << " connected components." << std::endl;
      dolphinOut() << "   Warning, there are more than 1 connected component, this model will have disconnected pieces. Consider doing another optimization." << std::endl;
    }

    if(badArtPointNumber == 0)
    {
      dolphinOut() << "There are no weak articulation points." << std::endl;
    }
    else
    {
      dolphinOut() << "   There are " << badArtPointNumber << " weak articulation points." << std::endl;
      dolphinOut() << "   Warning, there are some weak articulation points in this model. Consider doing another optimization." << std::endl;
    }

    dolphinOut() << "Time: " << time.elapsed()/1000.0 << " seconds." << std::endl;

    return QPair<float, QPair<int, int> >(time.elapsed()/1000.0, QPair<int, int>(totalConCompIter, totalArtPointIter));
  }

  Q_EXPORT_PLUGIN2(AssemblyPlugin, AssemblyPlugin)
}}//namespaces


