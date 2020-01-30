#include "validationclassboxes.h"
#include "ui_validationclassboxes.h"

#include <QStandardItemModel>
#include <QStandardItem>
#include <QFileSystemModel>
#include <QDir>

ValidationClassBoxes::ValidationClassBoxes(QWidget *parent, QMap<QString, QList<QImage>>* classBoxes)
    : QDialog(parent)
    , ui(new Ui::ValidationClassBoxes)
{
    ui->setupUi(this);

    if (classBoxes == nullptr)
    {
        return;
    }

    QStandardItemModel* model = new QStandardItemModel;

//     for (int groupnum = 0; groupnum < 3 ; ++groupnum)
//     {
//         QStandardItem *group = new QStandardItem(QString("Group %1").arg(groupnum));
//         for (int personnum = 0; personnum < 5 ; ++personnum)
//         {
//             QStandardItem *child = new QStandardItem(QString("Person %1 (group %2)").arg(personnum).arg(groupnum));
//             group->appendRow(child);
//         }
//         model->appendRow(group);
//     }

    for (auto it = classBoxes->begin(); it != classBoxes->end(); ++it)
    {
        QStandardItem* classGroup = new QStandardItem(it.key());
        for (auto crop = it.value().begin(); crop != it.value().end(); ++crop)
        {
            QStandardItem* item = new QStandardItem();
            item->setData(QVariant{QPixmap::fromImage(*crop)}, Qt::DecorationRole);
            classGroup->appendRow(item);
        }
        //model->appendColumn()
        model->appendRow(classGroup);
        //model->setItem(0, 0, item);
    }

    //setWindowModality(Qt::WindowModal);

    ui->columnView->setModel(model);
    ui->columnView->show();
}

ValidationClassBoxes::~ValidationClassBoxes()
{
    delete ui;
}
