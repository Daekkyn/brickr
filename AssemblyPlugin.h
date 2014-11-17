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

#ifndef ASSEMBLY_PLUGIN_H
#define ASSEMBLY_PLUGIN_H

#include <Dolphin/Plugins/PluginInterface/Plugin_interface.h>
//#include <Dolphin/Core/DolphinDefines.h>

#include <Memory>
#include <string>
#include<QSet>

#include "LegoBrick.h"
#include "LegoCloudNode.h"


namespace Dolphin { namespace plugins {

class AssemblyWidget;


class AssemblyPlugin : public Plugin_interface {
  Q_OBJECT
  Q_INTERFACES(Dolphin::plugins::Plugin_interface)

public:
  PLUGIN("Assembly","0.1","Lego")

  explicit AssemblyPlugin();
  virtual ~AssemblyPlugin();

  void init(MainApplication* _mainApplication);

  void onUnload();

  void test(int x, int y, int z);
  void loadVoxelization(QString filename);
  void loadObj(QString fileName);
  void loadTexture(QString fileName);
  void removeAllMeshes();

  LegoCloudNode* getFirstSelectedLegoCloudNode(bool debug = true);

  QPair<float, QPair<int, int> > autoOptimize();


private:
  bool parseBinvox(const std::string& filename, LegoCloudNode *legoCloudNode);



  std::auto_ptr<AssemblyWidget> assemblyWidget_;

};

}}//namespaces

#endif
