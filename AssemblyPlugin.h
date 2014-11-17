#ifndef ASSEMBLY_PLUGIN_H
#define ASSEMBLY_PLUGIN_H

#include <Memory>
#include <string>
#include <QSet>

#include "LegoBrick.h"
#include "LegoCloudNode.h"


class AssemblyWidget;


class AssemblyPlugin : public QObject {
  Q_OBJECT

public:

  explicit AssemblyPlugin();
  virtual ~AssemblyPlugin();

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


#endif
