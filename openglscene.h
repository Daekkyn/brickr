#ifndef OPENGLSCENE_H
#define OPENGLSCENE_H

#include "point3d.h"

#include <QGraphicsScene>
#include <QLabel>
#include <QTime>

#ifndef QT_NO_CONCURRENT
#include <QFutureWatcher>
#endif

class Model;

class OpenGLScene : public QGraphicsScene
{
    Q_OBJECT

public:
    OpenGLScene();

    void drawBackground(QPainter *painter, const QRectF &rect);

public slots:
    void enableWireframe(bool enabled);
    void enableNormals(bool enabled);
    void setModelColor();
    void setBackgroundColor();
    void loadModel();
    void loadModel(const QString &filePath);
    void modelLoaded();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent * wheelEvent);

private:
    QDialog *createDialog(const QString &windowTitle) const;

    void setModel(Model *model);

    bool m_wireframeEnabled;
    bool m_normalsEnabled;

    QColor m_modelColor;
    QColor m_backgroundColor;

    Model *m_model;

    QTime m_time;
    int m_lastTime;
    int m_mouseEventTime;

    float m_distance;
    Point3d m_rotation;
    Point3d m_angularMomentum;
    Point3d m_accumulatedMomentum;

    QLabel *m_labels[4];
    QWidget *m_modelButton;

    QGraphicsRectItem *m_lightItem;

#ifndef QT_NO_CONCURRENT
    QFutureWatcher<Model *> m_modelLoader;
#endif
};

#endif
