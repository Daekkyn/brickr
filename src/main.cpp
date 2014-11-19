#include "openglscene.h"

#include <QtGui>
#include <QGLWidget>
#include <QGraphicsView>
#include <QApplication>

class GraphicsView : public QGraphicsView
{
public:
    GraphicsView()
    {
        setWindowTitle(tr("Brick Assembly"));
    }

protected:
    void resizeEvent(QResizeEvent *event) {
        if (scene())
            scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
        QGraphicsView::resizeEvent(event);
    }
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    GraphicsView view;
    view.setViewport(new QGLWidget(QGLFormat(QGL::SampleBuffers)));
    view.setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    view.setScene(new OpenGLScene);
    view.show();

    view.resize(1280, 1024);

    return app.exec();
}
