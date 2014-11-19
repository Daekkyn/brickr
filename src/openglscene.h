#ifndef OPENGLSCENE_H
#define OPENGLSCENE_H

#include "Vector3.h"

#include <QGraphicsScene>
#include <QLabel>
#include <QTime>

#ifndef QT_NO_CONCURRENT
#include <QFutureWatcher>
#endif

class Model;
class LegoCloudNode;
class AssemblyPlugin;
class AssemblyWidget;

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
    void resetScene();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent * wheelEvent);

private:
    QDialog *createDialog(const QString &windowTitle) const;

    void setModel(std::shared_ptr<Model> model);

    bool m_wireframeEnabled;
    bool m_normalsEnabled;

    QColor m_modelColor;
    QColor m_backgroundColor;

    std::shared_ptr<Model> m_model;

    QTime m_time;
    int m_lastTime;
    int m_mouseEventTime;

    float m_distance;
    float m_scale;
    Vector3 m_translation;
    Vector3 m_rotation;
    Vector3 m_angularMomentum;
    Vector3 m_accumulatedMomentum;

    QLabel *m_labels[4];
    QWidget *m_modelButton;

    AssemblyWidget *m_assembly;
    std::shared_ptr<AssemblyPlugin> m_plugin;

    QGraphicsRectItem *m_lightItem;

#ifndef QT_NO_CONCURRENT
    QFutureWatcher<Model *> m_modelLoader;
#endif
};

#endif
