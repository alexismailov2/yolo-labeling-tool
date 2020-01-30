#ifndef VALIDATIONCLASSBOXES_H
#define VALIDATIONCLASSBOXES_H

#include <QDialog>
#include <QMap>

namespace Ui {
class ValidationClassBoxes;
}

class ValidationClassBoxes : public QDialog
{
    Q_OBJECT

public:
    explicit ValidationClassBoxes(QWidget *parent = nullptr, QMap<QString, QList<QImage>>* classBoxes = nullptr);
    ~ValidationClassBoxes();

private:
    Ui::ValidationClassBoxes *ui;
};

#endif // VALIDATIONCLASSBOXES_H
