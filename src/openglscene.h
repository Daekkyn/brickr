#ifndef OPENGLSCENE_H
#define OPENGLSCENE_H

#include <memory>

#include <QGraphicsScene>
#include <QLabel>
#include <QTime>

#ifndef QT_NO_CONCURRENT
#include <QFutureWatcher>
#endif

#include "Vector3.h"

class Model;
class LegoCloudNode;
class AssemblyPlugin;
class AssemblyWidget;

#include <ostream>
class teebuf;
class QDebugStream;

class OpenGLScene : public QGraphicsScene
{
    Q_OBJECT

public:
    OpenGLScene(int width, int height);
    ~OpenGLScene();

    void drawBackground(QPainter *painter, const QRectF &rect);

public slots:
    void resetScene();

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent * wheelEvent);

private:
    QDialog *createDialog(const QString &windowTitle) const;

    QColor m_backgroundColor;

    float m_distance;
    float m_scale;
    Vector3 m_translation;
    Vector3 m_rotation;
    Vector3 m_cameraTranslation;

    QLabel *m_labels[4];

    AssemblyWidget *m_assembly;
    std::auto_ptr<AssemblyPlugin> m_plugin;

    QGraphicsRectItem *m_lightItem;

    std::unique_ptr<QDebugStream> debugStreamOut_;
    std::unique_ptr<QDebugStream> debugStreamErr_;
    std::unique_ptr<teebuf> teebufOut_;
    std::unique_ptr<teebuf> teebufErr_;
};

#endif
