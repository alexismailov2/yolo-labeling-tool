#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "datasetproject.h"

#include <QMainWindow>
#include <QWheelEvent>
#include <QTableWidgetItem>
#include <QMessageBox>
#include <QListWidget>
#include <QtWidgets/QProgressDialog>

#include <QtCore/QProcess>

#include <iostream>
#include <fstream>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_pushButton_create_from_video_clicked();
    void on_pushButtonOpenProject_clicked();
    void on_pushButton_save_clicked();
    void on_pushButton_remove_clicked();
    void on_pushButton_prev_clicked();
    void on_pushButton_next_clicked();
    void on_tableWidget_label_cellDoubleClicked(int , int );
    void on_tableWidget_label_cellClicked(int , int );
    void on_horizontalSlider_images_sliderMoved(int );

    void clearAllClassBoxes();
    void nextClass();
    void prevClass();
    void datasetIteratorUpdated();

protected:
    void wheelEvent(QWheelEvent*);
    void keyPressEvent(QKeyEvent *);

private:
    void init();
    void initTableWidget();
    void initTableWidgetContextMenuSetup();

    void setNextImage();
    void setPreviousImage();
    void setCurrentImg();

    void updateButtonEnabling(bool isEnabled);
    void updateDatasetNavigator();

    bool openVideos();

    void updateCurrentClass();
    void updateClassesTable();

    void loadClassNameList();
    void loadDatasetList();
    void exportClassListToFile();

    Ui::MainWindow*        _ui;
    // TODO: May be it should be stored in the datasetproject source
    QVariantMap            _datasetList;
    QVariantMap::iterator  _datasetIt;
    QVariantMap            _classesList;
    QVariantMap::iterator  _classesIt;

    DatasetProject         _datasetProject;
};

#endif // MAINWINDOW_H
