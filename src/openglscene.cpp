#include "openglscene.h"
#include "model.h"
#include "LegoCloudNode.h"
#include "AssemblyWidget.h"
#include "AssemblyPlugin.h"

#include <QtGui>
#include <QtOpenGL>
#include <glu.h>
#include <QSettings>

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

OpenGLScene::OpenGLScene()
    : m_wireframeEnabled(false)
    , m_normalsEnabled(false)
    , m_modelColor(153, 255, 0)
    , m_backgroundColor(0, 170, 255)
    , m_model(0)
    , m_lastTime(0)
    , m_distance(1.4f)
    , m_angularMomentum(0, 40, 0)
{
    QWidget *controls = createDialog(tr("Controls"));

    m_modelButton = new QPushButton(tr("Load model"));
    connect(m_modelButton, SIGNAL(clicked()), this, SLOT(loadModel()));
#ifndef QT_NO_CONCURRENT
    connect(&m_modelLoader, SIGNAL(finished()), this, SLOT(modelLoaded()));
#endif
    controls->layout()->addWidget(m_modelButton);

    QCheckBox *wireframe = new QCheckBox(tr("Render as wireframe"));
    connect(wireframe, SIGNAL(toggled(bool)), this, SLOT(enableWireframe(bool)));
    controls->layout()->addWidget(wireframe);

    QCheckBox *normals = new QCheckBox(tr("Display normals vectors"));
    connect(normals, SIGNAL(toggled(bool)), this, SLOT(enableNormals(bool)));
    controls->layout()->addWidget(normals);

    QPushButton *colorButton = new QPushButton(tr("Choose model color"));
    connect(colorButton, SIGNAL(clicked()), this, SLOT(setModelColor()));
    controls->layout()->addWidget(colorButton);

    QPushButton *backgroundButton = new QPushButton(tr("Choose background color"));
    connect(backgroundButton, SIGNAL(clicked()), this, SLOT(setBackgroundColor()));
    controls->layout()->addWidget(backgroundButton);

    QWidget *statistics = createDialog(tr("Model info"));
    statistics->layout()->setMargin(20);

    for (int i = 0; i < 3; ++i) {
        m_labels[i] = new QLabel;
        statistics->layout()->addWidget(m_labels[i]);
    }

    /*
    QWidget *instructions = createDialog(tr("Instructions"));
    instructions->layout()->addWidget(new QLabel(tr("Use mouse wheel to zoom model, and click and drag to rotate model")));
    instructions->layout()->addWidget(new QLabel(tr("Move the sun around to change the light position")));
    */

    m_plugin = std::make_shared<AssemblyPlugin>();
    connect(m_plugin.get(), SIGNAL(geometryChanged()), this, SLOT(resetScene()));

    QWidget *assembly = createDialog(tr("Assembly"));
    m_assembly = new AssemblyWidget(m_plugin);
    assembly->layout()->addWidget(m_assembly);

    QWidget *widgets[] = { assembly, /*instructions, controls, statistics*/ };

    for (uint i = 0; i < sizeof(widgets) / sizeof(*widgets); ++i) {
        QGraphicsProxyWidget *proxy = new QGraphicsProxyWidget(0, Qt::Dialog);
        proxy->setWidget(widgets[i]);
        addItem(proxy);
    }

    QPointF pos(10, 10);
    foreach (QGraphicsItem *item, items()) {
        item->setFlag(QGraphicsItem::ItemIsMovable);
        item->setCacheMode(QGraphicsItem::DeviceCoordinateCache);

        const QRectF rect = item->boundingRect();
        item->setPos(pos.x() - rect.x(), pos.y() - rect.y());
        pos += QPointF(0, 10 + rect.height());
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
    m_time.start();
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
    glColor4f(m_modelColor.redF(), m_modelColor.greenF(), m_modelColor.blueF(), 1.0f);

    const int delta = m_time.elapsed() - m_lastTime;
    m_rotation += m_angularMomentum * (delta / 1000.0);
    m_lastTime += delta;

    glTranslatef(0, 0, -m_distance);
    glRotatef(m_rotation.x(), 1, 0, 0);
    glRotatef(m_rotation.y(), 0, 1, 0);
    glRotatef(m_rotation.z(), 0, 0, 1);

    glScalef(m_scale, m_scale, m_scale);
    glTranslatef(m_translation.x(), m_translation.y(), m_translation.z());

    glEnable(GL_MULTISAMPLE);

    m_plugin->draw();

    if (m_model)
      m_model->render(m_wireframeEnabled, m_normalsEnabled);

    glDisable(GL_MULTISAMPLE);

    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    painter->endNativePainting();

    QTimer::singleShot(20, this, SLOT(update()));
}

static std::shared_ptr<Model> loadModel(const QString &filePath)
{
    return std::make_shared<Model>(filePath);
}

void OpenGLScene::loadModel()
{
  QSettings settings;
  QString lastOpenedFile = settings.value("OpenGLScene::LoadModel", "").toString();

  QString selectedFilePath = QFileDialog::getOpenFileName(0, tr("Choose model"), lastOpenedFile, QLatin1String("*.obj"));
  loadModel(selectedFilePath);

  settings.setValue("OpenGLScene::LoadModel", selectedFilePath);
}

void OpenGLScene::loadModel(const QString &filePath)
{
    if (filePath.isEmpty())
        return;

    m_modelButton->setEnabled(false);
    QApplication::setOverrideCursor(Qt::BusyCursor);
#ifndef QT_NO_CONCURRENT
    m_modelLoader.setFuture(QtConcurrent::run(::loadModel, filePath));
#else
    setModel(::loadModel(filePath));
    modelLoaded();
#endif
}

void OpenGLScene::modelLoaded()
{
#ifndef QT_NO_CONCURRENT
    setModel(m_modelLoader.result());
#endif
    m_modelButton->setEnabled(true);
    QApplication::restoreOverrideCursor();
}

void OpenGLScene::setModel(std::shared_ptr<Model> model)
{
    m_model = model;
    connect(m_model.get(), SIGNAL(geometryChanged()), this, SLOT(resetScene()));

    m_labels[0]->setText(tr("File: %0").arg(m_model->fileName()));
    m_labels[1]->setText(tr("Points: %0 Edges : %1 Faces: %2").arg(m_model->points()).arg(m_model->edges()).arg(m_model->faces()));
    m_labels[2]->setText(tr("Bounds: (%0 %1 %2), (%3 %4 %5)")
                         .arg(m_model->minPoint().x()).arg(m_model->minPoint().y()).arg(m_model->minPoint().z())
                         .arg(m_model->maxPoint().x()).arg(m_model->maxPoint().y()).arg(m_model->maxPoint().z()));

    resetScene();
    update();
}

void OpenGLScene::resetScene()
{
  Vector3 boundsMin(0.0,0.0,0.0);
  Vector3 boundsMax(0.0,0.0,0.0);

  if (m_model)
  {
    boundsMin = boundsMin.min(m_model->minPoint());
    boundsMax = boundsMin.max(m_model->maxPoint());
  }

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

void OpenGLScene::enableWireframe(bool enabled)
{
    m_wireframeEnabled = enabled;
    update();
}

void OpenGLScene::enableNormals(bool enabled)
{
    m_normalsEnabled = enabled;
    update();
}

void OpenGLScene::setModelColor()
{
    const QColor color = QColorDialog::getColor(m_modelColor);
    if (color.isValid()) {
        m_modelColor = color;
        update();
    }
}

void OpenGLScene::setBackgroundColor()
{
    const QColor color = QColorDialog::getColor(m_backgroundColor);
    if (color.isValid()) {
        m_backgroundColor = color;
        update();
    }
}

void OpenGLScene::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseMoveEvent(event);
    if (event->isAccepted())
        return;
    if (event->buttons() & Qt::LeftButton) {
        const QPointF delta = event->scenePos() - event->lastScenePos();
        const Vector3 angularImpulse = Vector3(delta.y(), delta.x(), 0) * 0.1;

        m_rotation += angularImpulse;
        m_accumulatedMomentum += angularImpulse;

        event->accept();
        update();
    }
}

void OpenGLScene::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mousePressEvent(event);
    if (event->isAccepted())
        return;

    m_mouseEventTime = m_time.elapsed();
    m_angularMomentum = m_accumulatedMomentum = Vector3();
    event->accept();
}

void OpenGLScene::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsScene::mouseReleaseEvent(event);
    if (event->isAccepted())
        return;

    const int delta = m_time.elapsed() - m_mouseEventTime;
    m_angularMomentum = m_accumulatedMomentum * (1000.0 / qMax(1, delta));
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
