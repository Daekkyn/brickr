#include "openglscene.h"
#include "model.h"
#include "LegoCloudNode.h"
#include "AssemblyWidget.h"
#include "AssemblyPlugin.h"
#include "QDebugStream.h"

#include <QtGui>
#include <QtOpenGL>
#include <QSettings>

#ifdef WIN32
#include <windows.h>
#include <gl/GLU.h>
#else
#include <GL/glu.h>
#endif

#define QT_NO_CONCURRENT

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

QDialog *OpenGLScene::createDialog(const QString &windowTitle) const
{
    QDialog *dialog = new QDialog(0, Qt::CustomizeWindowHint | Qt::WindowTitleHint);

    dialog->setWindowOpacity(0.95);
    dialog->setWindowTitle(windowTitle);
    dialog->setLayout(new QVBoxLayout);

    return dialog;
}

OpenGLScene::OpenGLScene(int width, int height)
    : m_backgroundColor(180, 225, 255)
    , m_distance(1.4f)
{
    m_plugin = std::auto_ptr<AssemblyPlugin>(new AssemblyPlugin);
    connect(m_plugin.get(), SIGNAL(geometryChanged()), this, SLOT(resetScene()));

    QWidget *assembly = createDialog(tr("Brickr"));
    m_assembly = new AssemblyWidget(m_plugin.get());
    assembly->layout()->addWidget(m_assembly);

    QWidget *console = createDialog(tr("Console"));
    QTextEdit *consoleEdit = new QTextEdit();
    console->layout()->setMargin(0);
    consoleEdit->setMinimumSize(580,200);
    consoleEdit->setReadOnly(true);
    console->layout()->addWidget(consoleEdit);

    QWidget *info = createDialog(tr("About"));
    QLabel *infoLabel = new QLabel;
    infoLabel->setText("Automatic Generation of Constructable Brick Sculptures\n© 2013-2015 Romain Testuz and Yuliy Schwartzburg\nDetails at http://lgg.epfl.ch/publications/2013/lego.php\nContact: romain.testuz@rayform.ch");
    info->layout()->addWidget(infoLabel);

    debugStreamOut_ = std::unique_ptr<QDebugStream>(new QDebugStream(consoleEdit));
#ifdef WIN32
    std::cout.rdbuf(debugStreamOut_.get());
#else
    teebufOut_ = std::unique_ptr<teebuf>(new teebuf(std::cout.rdbuf(), debugStreamOut_.get()));
    std::cout.rdbuf(teebufOut_.get());
#endif

    debugStreamErr_ = std::unique_ptr<QDebugStream>(new QDebugStream(consoleEdit));
#ifdef WIN32
    std::cerr.rdbuf(debugStreamErr_.get());
#else
    teebufErr_ = std::unique_ptr<teebuf>(new teebuf(std::cerr.rdbuf(), debugStreamErr_.get()));
    std::cerr.rdbuf(teebufErr_.get());
#endif

    QWidget *widgets[] = { info, console, assembly  }; /*instructions, controls, statistics*/

    for (uint i = 0; i < sizeof(widgets) / sizeof(*widgets); ++i) {
        QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(0, Qt::Dialog);
        proxy->setWidget(widgets[i]);
        addItem(proxy);
    }

    uint i = 0;
    foreach (QGraphicsItem *item, items()) {
      item->setFlag(QGraphicsItem::ItemIsMovable);
      item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

      const QRectF rect = item->boundingRect();

      switch (i)
      {
      case 0:
      default:
          items()[0]->setPos(10-rect.x(),10-rect.y());
          break;
      case 1:
          items()[1]->setPos(width-rect.x()-rect.width()-10, height-rect.y()-rect.height()-10);
          break;
      case 2:
          items()[2]->setPos(width-rect.x()-rect.width()-10, 10-rect.y());
          break;
      }

      i++;
    }


    QRadialGradient gradient(40, 40, 40, 40, 40);
    gradient.setColorAt(0.2, Qt::yellow);
    gradient.setColorAt(1, Qt::transparent);

    m_lightItem = new QGraphicsRectItem(0, 0, 80, 80);
    m_lightItem->setPen(Qt::NoPen);
    m_lightItem->setBrush(gradient);
    m_lightItem->setFlag(QGraphicsItem::ItemIsMovable);
    m_lightItem->setPos(800, 200);
    addItem(m_lightItem);

    resetScene();
}

OpenGLScene::~OpenGLScene()
{
#ifndef WIN32
  std::cout.rdbuf(teebufOut_->sb1());
  std::cerr.rdbuf(teebufErr_->sb1());
#endif
}

void OpenGLScene::drawBackground(QPainter *painter, const QRectF &)
{
    if (painter->paintEngine()->type() != QPaintEngine::OpenGL
        && painter->paintEngine()->type() != QPaintEngine::OpenGL2)
    {
        qWarning("OpenGLScene: drawBackground needs a QGLWidget to be set as viewport on the graphics view");
        return;
    }

    painter->beginNativePainting();

    glClearColor(m_backgroundColor.redF(), m_backgroundColor.greenF(), m_backgroundColor.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluPerspective(70, width() / height(), 0.01, 1000);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    float w = m_lightItem->x() - width() / 2;
    float h = height() / 2 - m_lightItem->y();
    const float pos[] = { w, h, 512, 0 };
    glLightfv(GL_LIGHT0, GL_POSITION, pos);

    glTranslatef(m_cameraTranslation.x(), m_cameraTranslation.y(), m_cameraTranslation.z());
    glTranslatef(0, 0, -m_distance);
    glRotatef(m_rotation.x(), 1, 0, 0);
    glRotatef(m_rotation.y(), 0, 1, 0);
    glRotatef(m_rotation.z(), 0, 0, 1);

    glScalef(m_scale, m_scale, m_scale);
    glTranslatef(m_translation.x(), m_translation.y(), m_translation.z());

    glEnable(GL_MULTISAMPLE);

    m_plugin->draw();

    glDisable(GL_MULTISAMPLE);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    painter->endNativePainting();

//    QTimer::singleShot(20, this, SLOT(update()));
}

static std::shared_ptr<Model> loadModel(const QString &filePath)
{
    return std::make_shared<Model>(filePath);
}

void OpenGLScene::resetScene()
{
  Vector3 boundsMin(0.0,0.0,0.0);
  Vector3 boundsMax(0.0,0.0,0.0);

  if (m_plugin->getLegoCloudNode())
  {
    boundsMin = boundsMin.min(m_plugin->getLegoCloudNode()->minPoint());
    boundsMax = boundsMin.max(m_plugin->getLegoCloudNode()->maxPoint());
  }

  m_translation = (boundsMin + boundsMax) / -2.0;

  float norm = (boundsMax - boundsMin).norm();

  if (norm < std::numeric_limits<float>::epsilon())
    m_scale = 1.0;
  else
    m_scale = 1.0/norm;
}

void OpenGLScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);
    if (event->isAccepted())
        return;
    if (event->buttons() & Qt::LeftButton) {
        const QPointF delta = event->scenePos() - event->lastScenePos();
        const Vector3 angularImpulse = Vector3(delta.y(), delta.x(), 0.0f) * 0.1f;

        m_rotation += angularImpulse;

        event->accept();
        update();
    }
    else if (event->buttons() & Qt::RightButton) {
        const QPointF delta = event->scenePos() - event->lastScenePos();
        m_cameraTranslation += Vector3(delta.x(), -1*delta.y(), 0.0f) * 0.001f;
        event->accept();
        update();
    }
}

void OpenGLScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
    if (event->isAccepted())
        return;

    event->accept();
}

void OpenGLScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    if (event->isAccepted())
        return;

    event->accept();
    update();
}

void OpenGLScene::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    QGraphicsScene::wheelEvent(event);
    if (event->isAccepted())
        return;

    m_distance *= qPow(1.2, -event->delta() / 120);
    event->accept();
    update();
}
