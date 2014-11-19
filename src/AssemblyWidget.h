#ifndef ASSEMBLY_WIDGET_H
#define ASSEMBLY_WIDGET_H

#include <QWidget>
#include "ui_AssemblyWidget.h"

#include "LegoBrick.h" //For BrickSize

class AssemblyPlugin;

class AssemblyWidget: public QWidget, private Ui_AssemblyWidget {
  Q_OBJECT

public:

  explicit AssemblyWidget(std::shared_ptr<AssemblyPlugin> _plugin, QWidget* _parent=0);
  ~AssemblyWidget();

  void setMaxLayerSpinBox(int max);

private slots:
  void on_testButton_pressed();
  void on_layerSpinBox_valueChanged(int _value);
  void on_loadFileButton_pressed();
  void on_loadTextureButton_pressed();
  void on_mergeButton_pressed();
  void on_layerBox_stateChanged(int state);

  void on_renderBricksBox_stateChanged(int state);
  void on_renderGraphBox_stateChanged(int state);
  void on_randomRadioButton_toggled(bool checked);
  void on_realColorRadioButton_toggled(bool checked);
  void on_conRadioButton_toggled(bool checked);
  void on_biconRadioButton_toggled(bool checked);

  void on_splitConCompButton_pressed();
  void on_loopConCompButton_pressed();

  void on_splitBiconCompButton_pressed();
  void on_loopBiconCompButton_pressed();

  void on_postHollowButton_pressed();

  void on_saveInstructionsButton_pressed();
  void on_objExportButton_pressed();

  void on_printStatsButton_pressed();

  void on_autoOptimizeButton_pressed();
  void on_finalizeButton_pressed();

  void on_solveBrickLimitButton_pressed();
  void on_spinBox1x2_valueChanged(int value);
  void on_spinBox1x3_valueChanged(int value);
  void on_spinBox1x4_valueChanged(int value);
  void on_spinBox1x6_valueChanged(int value);
  void on_spinBox1x8_valueChanged(int value);
  void on_spinBox2x2_valueChanged(int value);
  void on_spinBox2x3_valueChanged(int value);
  void on_spinBox2x4_valueChanged(int value);
  void on_spinBox2x6_valueChanged(int value);
  void on_spinBox2x8_valueChanged(int value);



  //void on_colorRenderingBox_currentIndexChanged(const QString & text );

private:
  void setBrickLimit(BrickSize size, int value);
  void resetUi();
  void loadFile(const QString& filePath, int voxelizationResolution = 0);
  bool isMeshExtensionSupported(const QString& extension) const;

  std::shared_ptr<AssemblyPlugin> plugin_;
};

#endif
