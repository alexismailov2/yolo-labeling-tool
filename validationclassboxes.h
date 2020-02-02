#ifndef VALIDATIONCLASSBOXES_H
#define VALIDATIONCLASSBOXES_H

#include <QDialog>
#include <QMap>
#include <QListWidget>

namespace Ui {
class ValidationClassBoxes;
}

class ValidationClassBoxes : public QDialog
{
    Q_OBJECT

public:
    explicit ValidationClassBoxes(QWidget *parent = nullptr, QVariantMap* datasetList = nullptr);
    ~ValidationClassBoxes();

private:
    Ui::ValidationClassBoxes* _ui;
    QMap<QString, QListWidget*> _listWidgets;
};

#endif // VALIDATIONCLASSBOXES_H
