#ifndef MODEL_H
#define MODEL_H

#include <QString>
#include <QVector>
#include <QObject>

#include <math.h>

#include "Vector3.h"

class Model : public QObject
{
  Q_OBJECT

public:
  Model() {}
  Model(const QString &filePath);

  void render(bool wireframe = false, bool normals = false) const;

  QString fileName() const { return m_fileName; }
  int faces() const { return m_pointIndices.size() / 3; }
  int edges() const { return m_edgeIndices.size() / 2; }
  int points() const { return m_points.size(); }

  Vector3 minPoint() { return boundsMin_; }
  Vector3 maxPoint() { return boundsMax_; }

signals:
  void geometryChanged();

private:
  QString m_fileName;
  QVector<Vector3> m_points;
  QVector<Vector3> m_normals;
  QVector<int> m_edgeIndices;
  QVector<int> m_pointIndices;

  Vector3 boundsMin_, boundsMax_;
};

#endif
