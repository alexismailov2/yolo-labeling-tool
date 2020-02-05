#pragma once

#include "ValidationClassBoxesListModel.h"

#include <QDialog>
#include <QMap>
#include <QListView>

namespace Ui {
class ValidationClassBoxes;
}

class ValidationClassBoxes : public QDialog
{
    Q_OBJECT

public:
    explicit ValidationClassBoxes(QWidget *parent = nullptr,
                                  QVariantMap* datasetList = nullptr);
    ~ValidationClassBoxes();

signals:
    void datasetListUpdated();

private:
    void sync();

private:
    Ui::ValidationClassBoxes* _ui;
    QVariantMap* _datasetList;
    QMap<QString, QListView*> _listWidgets;
    QMap<QString, QList<ClassBoxes>> _classBoxesList;
};
