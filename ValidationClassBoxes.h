#pragma once

#include "ValidationClassBoxesListModel.h"

#include <QDialog>
#include <QListView>
#include <QMap>

#include <QtCharts/QChartView>

namespace Ui {
class ValidationClassBoxes;
}

class ValidationClassBoxes : public QDialog
{
    Q_OBJECT

public:
    explicit ValidationClassBoxes(QWidget *parent = nullptr,
                                  QVariantMap* datasetList = nullptr,
                                  QVariantMap* classesList = nullptr);
    ~ValidationClassBoxes();

signals:
    void datasetListUpdated();

private:
    void sync();
    void createGistogram();

private:
    Ui::ValidationClassBoxes* _ui;
    QVariantMap* _datasetList;
    QMap<QString, QListView*> _listWidgets;
    QMap<QString, QList<ClassBoxes>> _classBoxesList;
    QtCharts::QChartView _chartView;
};
