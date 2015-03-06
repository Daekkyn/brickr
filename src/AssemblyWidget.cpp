#include "AssemblyWidget.h"

#include "AssemblyPlugin.h"
#include "LegoCloud.h"

#include <QFileDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QImage>
#include <QSvgGenerator>
#include <QtCore/QSettings>
#include <QInputDialog>
#include <QApplication>
#include <QProcess>
#include <QTextStream>
#include <fstream>

//#define STATISTICS

AssemblyWidget::AssemblyWidget(AssemblyPlugin* _plugin, QWidget* _parent)
  : QWidget(_parent), Ui_AssemblyWidget(), plugin_(_plugin) {

  setupUi(this);
  //instructionView->setScene(&scene_);
  //instructionView->scale(10, 10);

  plugin_->setWidget(this);
}

AssemblyWidget::~AssemblyWidget() {
}

void AssemblyWidget::setMaxLayerSpinBox(int max)
{
  layerSpinBox->setMaximum(max);
}

void AssemblyWidget::on_testButton_pressed() {
  resetUi();
  plugin_->test(testXSpinBox->value(), testYSpinBox->value(), testZSpinBox->value());

  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  if(hollowCheckBox->isChecked())
    legoCloudNode->getLegoCloud()->preHollow(shellThicknessSpinBox->value());
}

void AssemblyWidget::on_loadFileButton_pressed()
{
#ifdef STATISTICS
  QStringList filePathList = QFileDialog::getOpenFileNames(this, "Open File", "", "All (*.binvox *.obj)");

  if(filePathList.isEmpty())
  {
    return;
  }

  const QFileInfo firstFileInfo(*(filePathList.begin()));
  QFile statFile(firstFileInfo.absolutePath() + "/stats.txt");


  statFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
  QTextStream statFileTextStream(&statFile);

  std::cout << "Writing statistics to: " << statFile.fileName().toStdString() << std::endl;
  assert(statFile.exists());


  for(QStringList::Iterator filePathIt = filePathList.begin(); filePathIt != filePathList.end(); filePathIt++)
  {
    const QString selectedFilePath = *filePathIt;
    const QFileInfo selectedFileInfo(selectedFilePath);

    for(int voxRes = 50; voxRes <= 50; voxRes += 20)
    {
      for(int repeat = 0; repeat < 20; repeat++)
      {
        loadFile(selectedFilePath, voxRes);

        LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
        if(!legoCloudNode)
          return;

        statFileTextStream << selectedFileInfo.baseName() << "\t";
        statFileTextStream << voxRes << "\t";
        statFileTextStream << legoCloudNode->getLegoCloud()->getBrickNumber() << "\t";

        // (time, (conCompIterations, artPointIteration))
        QPair<float, QPair<int, int> > autoOptimizeResult = plugin_->autoOptimize();

        statFileTextStream << autoOptimizeResult.first << "\t";

        statFileTextStream << legoCloudNode->getLegoCloud()->getBrickNumber() << "\t";

        float optimizeDurationWithPostHollow = autoOptimizeResult.first + legoCloudNode->getLegoCloud()->postHollow();

        statFileTextStream << optimizeDurationWithPostHollow << "\t";

        statFileTextStream << legoCloudNode->getLegoCloud()->getBrickNumber() << "\t";

        statFileTextStream << legoCloudNode->getLegoCloud()->getConCompNumber() << "\t";

        statFileTextStream << legoCloudNode->getLegoCloud()->getBadArtPointNumber() << "\t";

        statFileTextStream << autoOptimizeResult.second.first << "\t";

        statFileTextStream << autoOptimizeResult.second.second << "\t";

        statFileTextStream << "\n";

        statFileTextStream.flush();
      }
    }


  }

#else
  QSettings settings;
  QString lastOpenedFile = settings.value("AssemblyPlugin::LoadFile", "").toString();

  QString selectedFilePath = QFileDialog::getOpenFileName(this, "Open File", lastOpenedFile);

  if(selectedFilePath.isNull())
  {
    return;
  }

  settings.setValue("AssemblyPlugin::LoadFile", selectedFilePath);


  //We now check if the extension is a mesh
  QFileInfo selectedFileinfo(selectedFilePath);
  if(isMeshExtensionSupported(selectedFileinfo.suffix()))
  {
    bool ok;
    int voxelizationResolution = QInputDialog::getInt(qApp->activeWindow(), "Voxelization resolution",
                                                      "Please enter the resolution that you want:", 30, 1, 999999999, 1, &ok);

    if(!ok)
    {
      return;//User pressed cancel
    }

    loadFile(selectedFilePath, voxelizationResolution);

  }
  else if(selectedFileinfo.suffix() == "binvox")
  {
    loadFile(selectedFilePath);
  }
  else
  {
    std::cerr << "This file extension is not supported: " << selectedFileinfo.suffix().toStdString() << std::endl;
  }

#endif

  resetUi();
}

/*
void AssemblyWidget::on_loadTextureButton_pressed()
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  QString objFileName = QFileDialog::getOpenFileName(this, tr("Open OBJ File"), "",tr("OBJ (*.obj)"));
  if(objFileName.isNull())
    return;

  QString texFileName = QFileDialog::getOpenFileName(this, tr("Open Texture File"), "",tr("Images (*.bmp *.jpg *.jpeg *.tga *.dds *.png)"));

  if(texFileName.isNull())
    return;

//  plugin_->removeAllMeshes();
//  plugin_->loadObj(objFileName);
//  plugin_->loadTexture(texFileName);
  realColorRadioButton->setChecked(true);
  legoCloudNode->nodeUpdated();
}
*/

void AssemblyWidget::on_mergeButton_pressed()
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  legoCloudNode->getLegoCloud()->merge();
  legoCloudNode->nodeUpdated();
}

void AssemblyWidget::on_layerSpinBox_valueChanged(int _value){
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  legoCloudNode->setRenderLayer(_value);
  legoCloudNode->nodeUpdated();

  //legoCloudNode->drawInstructions(&scene_);
}


void AssemblyWidget::on_layerBox_stateChanged(int state)
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  if(state == Qt::Checked)
    legoCloudNode->setRenderLayerByLayer(true);
  else
    legoCloudNode->setRenderLayerByLayer(false);
  legoCloudNode->nodeUpdated();
}

void AssemblyWidget::on_renderBricksBox_stateChanged(int state)
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  if(state == Qt::Checked)
    legoCloudNode->setRenderBricks(true);
  else
    legoCloudNode->setRenderBricks(false);

  legoCloudNode->nodeUpdated();
}

void AssemblyWidget::on_renderGraphBox_stateChanged(int state)
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  if(state == Qt::Checked)
    legoCloudNode->setRenderGraph(true);
  else
    legoCloudNode->setRenderGraph(false);

  legoCloudNode->nodeUpdated();
}

void AssemblyWidget::on_randomRadioButton_toggled(bool checked)
{
  if(checked)
  {
    LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
    if(!legoCloudNode)
      return;

    legoCloudNode->setColorRendering(LegoCloudNode::Random);
    legoCloudNode->nodeUpdated();
  }
}

void AssemblyWidget::on_realColorRadioButton_toggled(bool checked)
{
  if(checked)
  {
    LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
    if(!legoCloudNode)
      return;

    legoCloudNode->setColorRendering(LegoCloudNode::RealColor);
    legoCloudNode->nodeUpdated();
  }
}

void AssemblyWidget::on_conRadioButton_toggled(bool checked)
{
  if(checked)
  {
    LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
    if(!legoCloudNode)
      return;

    legoCloudNode->setColorRendering(LegoCloudNode::ConnectedComp);
    legoCloudNode->nodeUpdated();
  }
}

void AssemblyWidget::on_biconRadioButton_toggled(bool checked)
{
  if(checked)
  {
    LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
    if(!legoCloudNode)
      return;

    legoCloudNode->setColorRendering(LegoCloudNode::BiconnectedComp);
    legoCloudNode->nodeUpdated();
  }
}

void AssemblyWidget::on_splitConCompButton_pressed()
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  legoCloudNode->getLegoCloud()->splitConComp();
  legoCloudNode->nodeUpdated();
}

void AssemblyWidget::on_loopConCompButton_pressed()
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  legoCloudNode->getLegoCloud()->loopConComp();
  legoCloudNode->nodeUpdated();
}

void AssemblyWidget::on_splitBiconCompButton_pressed()
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  legoCloudNode->getLegoCloud()->splitBiconComp();
  legoCloudNode->nodeUpdated();
}

void AssemblyWidget::on_loopBiconCompButton_pressed()
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  legoCloudNode->getLegoCloud()->loopBiconComp();
  legoCloudNode->nodeUpdated();
}

void AssemblyWidget::on_postHollowButton_pressed()
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  legoCloudNode->getLegoCloud()->postHollow();
  legoCloudNode->nodeUpdated();
}

void AssemblyWidget::on_saveInstructionsButton_pressed()
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  QSettings settings;
  QString lastSavedFile = settings.value("AssemblyPlugin::SaveInstruction", "").toString();

  QString filePathBase = QFileDialog::getSaveFileName(this, "Save instructions (.png .jpg or .svg)", lastSavedFile, "Images (*.png *.jpg *.svg)");
  if(filePathBase == NULL)
  {
    //User canceled
    return;
  }

  settings.setValue("AssemblyPlugin::SaveInstruction", filePathBase);

  QFileInfo fileInfo(filePathBase);
  bool useSVG = fileInfo.suffix().compare("svg", Qt::CaseInsensitive) == 0;

  const int BRICK_PIXEL_SIZE = 20;

  QGraphicsScene scene;

//  progress::setNumberOfSteps(legoCloudNode->getLegoCloud()->getLevelNumber(), "Saving instructions...");
//  progress::setProgress(0);

  for(int level = 0; level < legoCloudNode->getLegoCloud()->getLevelNumber(); level++)
  {
    legoCloudNode->setRenderLayer(level);
    legoCloudNode->drawInstructions(&scene, !useSVG);

    int imageSizeX = legoCloudNode->getLegoCloud()->getWidth()*BRICK_PIXEL_SIZE;
    int imageSizeY = legoCloudNode->getLegoCloud()->getDepth()*BRICK_PIXEL_SIZE;

    QString filePathLevel = fileInfo.absolutePath() + "/" + fileInfo.baseName() + "_" + QString::number(level) + "." + fileInfo.completeSuffix();
    std::cout << "Saving: " << filePathLevel.toStdString().c_str() << std::endl;

    if(useSVG)
    {
      QSvgGenerator svgGen;

      svgGen.setFileName(filePathLevel);
      svgGen.setSize(QSize(imageSizeX, imageSizeY));
      svgGen.setViewBox(QRect(0, 0, imageSizeX, imageSizeY));

      QPainter painter( &svgGen );
      scene.render( &painter );
    }
    else
    {
      QImage image(QSize(imageSizeX, imageSizeY), QImage::Format_ARGB32_Premultiplied);
      image.fill(QColor("White"));
      QPainter painter(&image);
      painter.setRenderHint(QPainter::Antialiasing);
      scene.render(&painter);
      image.save(filePathLevel);
    }

//    progress::setProgress(level);
  }

//  progress::finish();
}

void AssemblyWidget::on_objExportButton_pressed()
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  QString filename = QFileDialog::getSaveFileName(this, "Save as", "", "Obj (*.obj)");

  if(filename.isNull())
  {
    return;
  }

  legoCloudNode->exportToObj(filename);
}

void AssemblyWidget::on_printStatsButton_pressed()
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  legoCloudNode->getLegoCloud()->printStats();
}

void AssemblyWidget::on_autoOptimizeButton_pressed()
{
  plugin_->autoOptimize();
}

void AssemblyWidget::on_finalizeButton_pressed()
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;


  legoCloudNode->getLegoCloud()->postHollow();
  legoCloudNode->getLegoCloud()->solveBrickNumberLimitation();
  legoCloudNode->getLegoCloud()->merge();

  std::cout << "Finalization done, the instructions can be saved." << std::endl;
}

void AssemblyWidget::on_solveBrickLimitButton_pressed()
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  legoCloudNode->getLegoCloud()->solveBrickNumberLimitation();
  legoCloudNode->nodeUpdated();
}


void AssemblyWidget::on_spinBox1x2_valueChanged(int value)
{
  setBrickLimit(BrickSize(1, 2), value);
}

void AssemblyWidget::on_spinBox1x3_valueChanged(int value)
{
  setBrickLimit(BrickSize(1, 3), value);
}

void AssemblyWidget::on_spinBox1x4_valueChanged(int value)
{
  setBrickLimit(BrickSize(1, 4), value);
}

void AssemblyWidget::on_spinBox1x6_valueChanged(int value)
{
  setBrickLimit(BrickSize(1, 6), value);
}

void AssemblyWidget::on_spinBox1x8_valueChanged(int value)
{
  setBrickLimit(BrickSize(1, 8), value);
}

void AssemblyWidget::on_spinBox2x2_valueChanged(int value)
{
  setBrickLimit(BrickSize(2, 2), value);
}

void AssemblyWidget::on_spinBox2x3_valueChanged(int value)
{
  setBrickLimit(BrickSize(2, 3), value);
}

void AssemblyWidget::on_spinBox2x4_valueChanged(int value)
{
  setBrickLimit(BrickSize(2, 4), value);
}

void AssemblyWidget::on_spinBox2x6_valueChanged(int value)
{
  setBrickLimit(BrickSize(2, 6), value);
}

void AssemblyWidget::on_spinBox2x8_valueChanged(int value)
{
  setBrickLimit(BrickSize(2, 8), value);
}


void AssemblyWidget::resetUi()
{
  //instructionView->setSceneRect(NULL);
  //instructionView->scene()->setSceneRect(0, 0, 1, 1);

  layerBox->setCheckState(Qt::Unchecked);
  layerSpinBox->setValue(0);
  renderBricksBox->setCheckState(Qt::Checked);
  renderGraphBox->setCheckState(Qt::Unchecked);
  realColorRadioButton->setChecked(true);

  spinBox1x2->setValue(-1);
  spinBox1x3->setValue(-1);
  spinBox1x4->setValue(-1);
  spinBox1x6->setValue(-1);
  spinBox1x8->setValue(-1);
  spinBox2x2->setValue(-1);
  spinBox2x3->setValue(-1);
  spinBox2x4->setValue(-1);
  spinBox2x6->setValue(-1);
  spinBox2x8->setValue(-1);
}

void AssemblyWidget::loadFile(const QString &filePath, int voxelizationResolution)
{
  QFileInfo selectedFileinfo(filePath), scaledFileinfo(filePath);

  if(!selectedFileinfo.exists() || !selectedFileinfo.isReadable())
  {
    std::cerr << "Unable to open file: " << filePath.toStdString().c_str() << std::endl;
    return;
  }

  QString binvoxFilePath;
  if(isMeshExtensionSupported(selectedFileinfo.suffix()))
  {
    QString scaledFilePath = selectedFileinfo.absolutePath() + "/_" +selectedFileinfo.baseName() + "_scaled.obj";

    {
      QFile scaledFile(scaledFilePath);
      scaledFile.remove();
    }
    scaleMesh(filePath, scaledFilePath);
    scaledFileinfo = QFileInfo(scaledFilePath);

    assert(voxelizationResolution > 0);
    binvoxFilePath = selectedFileinfo.absolutePath() + "/"+selectedFileinfo.baseName()+QString::number(voxelizationResolution) + ".binvox";
    QFile binvoxFile(binvoxFilePath);

    if (binvoxFile.exists())
    {
      //We make sure that binvoxFilePath does not exist
      std::cout << "Removing file: " << qPrintable(binvoxFilePath) << std::endl;
      binvoxFile.remove();
    }

    //First we try to find it in its default location
#ifdef WIN32
    QFileInfo binvoxProgramFileInfo(QCoreApplication::applicationDirPath() + "/binvox.exe");
#else
    QFileInfo binvoxProgramFileInfo(QCoreApplication::applicationDirPath() + "/../Resources/binvox");
#endif
//    std::cout << qPrintable(QCoreApplication::applicationFilePath()) << std::endl;

    if(!binvoxProgramFileInfo.exists())
    {
      QSettings settings;
      binvoxProgramFileInfo.setFile(settings.value("AssemblyPlugin::Binvox", "").toString());

      //If it is not in the default location, we try from the last specified location
      if(!binvoxProgramFileInfo.exists())
      {
        //If it is not found in the last specified location; we ask the user
        QString binvoxFilePath = QFileDialog::getOpenFileName(this, "Locate binvox executable");
        if(binvoxFilePath.isNull())
        {
          std::cerr << "The binvox executable was not found." << std::endl;
          return;
        }

        settings.setValue("AssemblyPlugin::Binvox", binvoxFilePath);


        binvoxProgramFileInfo.setFile(binvoxFilePath);
        assert(binvoxProgramFileInfo.exists());
      }
    }

    //We first check that the following file does not exist or we remove it (otherwise the binvox program will rename the output file)
    QFile binvoxProgramOutputFile(scaledFileinfo.absolutePath()+"/"+scaledFileinfo.baseName()+".binvox");

    if(binvoxProgramOutputFile.exists())
    {
      std::cout << "Removing file: " << qPrintable(binvoxProgramOutputFile.fileName()) << std::endl;
      assert(binvoxProgramOutputFile.remove());
    }

#ifdef WIN32
    QString command("\"" + binvoxProgramFileInfo.absoluteFilePath() + "\" -d "+ QString::number(voxelizationResolution) + " \"" + scaledFilePath + "\"");
#else
    QString command("\"" + binvoxProgramFileInfo.absoluteFilePath()+ "\" -pb -d "+ QString::number(voxelizationResolution) + " \"" +scaledFilePath + "\"");
#endif
//    QString command(binvoxProgramFileInfo.absoluteFilePath()+ " -d "+ QString::number(voxelizationResolution) + " " +filePath);
    std::cout << "Running " << qPrintable(command) << std::endl;


    QProcess process;
    process.start(command);
    process.waitForFinished(-1); // will wait forever until finished

//    std::cout << process.readAllStandardOutput().data() << std::endl;
    std::cerr << process.readAllStandardError().data() << std::endl;

    if(!binvoxProgramOutputFile.exists())
    {
      std::cerr << "The file could not be voxelized with binvox. The format is probably not correctly handled by binvox or the software does not have administrative rights." << std::endl;
      return;
    }

    binvoxProgramOutputFile.rename(binvoxFile.fileName());

    {
      QFile scaledFile(scaledFilePath);
      scaledFile.remove();
    }

    assert(binvoxFile.exists());
  }
  else
  {
    assert(selectedFileinfo.suffix() == "binvox");
    binvoxFilePath = filePath;
  }

  plugin_->loadVoxelization(binvoxFilePath);

  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  if(hollowCheckBox->isChecked())
    legoCloudNode->getLegoCloud()->preHollow(shellThicknessSpinBox->value());
}

bool AssemblyWidget::isMeshExtensionSupported(const QString &extension) const
{
  return extension.compare("obj", Qt::CaseInsensitive) == 0;// ||
//      extension.compare("off", Qt::CaseInsensitive) == 0 ||
//      extension.compare("stl", Qt::CaseInsensitive) == 0 ||
//      extension.compare("ply", Qt::CaseInsensitive) == 0 ||
//      extension.compare("dxf", Qt::CaseInsensitive) == 0;
}

void AssemblyWidget::scaleMesh(const QString &filePath, const QString &scaledFilePath)
{
  QFile infile(filePath);
  if (!infile.open(QFile::ReadOnly))
    return;

  QFile outfile(scaledFilePath);
  if (!outfile.open(QFile::WriteOnly | QFile::Truncate))
    return;

  QTextStream in(&infile);
  QTextStream out(&outfile);

  while (!in.atEnd()) {
    QString input = in.readLine();
    if (input.isEmpty() || input[0] == '#')
      continue;

    QTextStream ts(&input);
    QString id;
    ts >> id;
    if (id == "v") {
      Vector3 p;
      for (int i = 0; i < 3; ++i) {
        ts >> p[i];
      }
      out << "v " << p[0] << " " << 0.83333333*p[1] << " " << p[2] << "\n";
    } else {
      out << input << "\n";
    }
  }

  infile.close();
  outfile.close();
}


void AssemblyWidget::setBrickLimit(BrickSize size, int value)
{
  LegoCloudNode* legoCloudNode = plugin_->getLegoCloudNode();
  if(!legoCloudNode)
    return;

  legoCloudNode->getLegoCloud()->setBrickLimit(size, value);
}

