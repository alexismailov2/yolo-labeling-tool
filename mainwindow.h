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

    void keyPressEvent(QKeyEvent *);

    void setNextImage();
    void setPreviousImage();
    void clear_label_data();

    void nextClass();
    void prevClass();

    void on_tableWidget_label_cellDoubleClicked(int , int );
    void on_tableWidget_label_cellClicked(int , int );

    void on_horizontalSlider_images_sliderMoved(int );

    void datasetIteratorUpdated();

protected:
    void wheelEvent(QWheelEvent*);

private:
    void init();
    void initTableWidget();
    void initTableWidgetContextMenuSetup();

    void updateButtonEnabling(bool isEnabled);
    void updateDatasetNavigator();

    bool openVideos();
    void img_open(const int);

    void setCurrentImg();

    void load_label_list_data(QString);
    auto get_labeling_data(QString) const -> QString;

    void updateCurrentClass();

    // TODO: Should be deletd after test
    //void saveClassNamesListToJson();
    void loadClassNameList();
    void loadDatasetList();

    Ui::MainWindow*        ui;
    // TODO: May be it should be stored in the datasetproject source
    QVariantMap            _datasetList;
    QVariantMap::iterator  _datasetIt;
    QVariantMap            _classesList;
    QVariantMap::iterator  _classesIt;

    QStringList            m_objList;
    int                    m_objIndex{};
    QProcess*              m_SlicingDatasetProcess{};
    QProgressDialog*       m_progressDialog{};
    DatasetProject         _datasetProject;
};

#endif // MAINWINDOW_H
