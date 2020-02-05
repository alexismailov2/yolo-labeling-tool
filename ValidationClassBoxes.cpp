#include "ValidationClassBoxes.h"
#include "ui_validationclassboxes.h"

#include "Utils.h"

#include <QMenu>
#include <QDebug>
#include <QProgressDialog>

ValidationClassBoxes::ValidationClassBoxes(QWidget *parent, QVariantMap* datasetList)
    : QDialog(parent)
    , _ui(new Ui::ValidationClassBoxes)
    , _datasetList{datasetList}
{
    _ui->setupUi(this);
    _ui->tabWidget->clear();

    QProgressDialog progressDialog("Generation crops process...", "&Cancel", 0, 100);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumSize(400, 40);
    progressDialog.setRange(0, 100);
    progressDialog.setValue(0);

    for (auto datasetIt = datasetList->begin(); datasetIt != datasetList->end(); ++datasetIt)
    {
        progressDialog.setValue((std::distance(datasetList->begin(), datasetIt) * 100)/datasetList->size());
        QImage image{datasetIt.key()};
        extractClassBoxes(datasetIt, [&](QString const& className, QRectF&& boxRect) {
            _classBoxesList[className].push_back(
                        ClassBoxes{datasetIt,
                                   datasetIt.key(),
                                   boxRect,
                                   QPixmap::fromImage(image.copy(toAbsolute(boxRect, image.size()))
                                                           .scaled(100, 100, Qt::AspectRatioMode::KeepAspectRatio))
                        });
            // TODO: Should be moved to dedicated function
            // auto croppedImageFile = datasetIt.key().toStdString();
            // croppedImageFile = croppedImageFile.substr(croppedImageFile.find_last_of('/') + 1,
            //                                            croppedImageFile.find_last_of('.') - croppedImageFile.find_last_of('/') - 1);
            // cropped.save(QString::fromStdString(croppedImageFile) + "_cropped_" + QString::number(i) + ".png");
        });
    }

    for (auto it = _classBoxesList.begin(); it != _classBoxesList.end(); ++it)
    {
        QListView* currentClassList = new QListView(this);
        currentClassList->setViewMode(QListView::IconMode);
        currentClassList->setIconSize(QSize(100,100));
        currentClassList->setResizeMode(QListView::Adjust);
        currentClassList->setSelectionMode(QAbstractItemView::ExtendedSelection);
        ValidationClassBoxesListModel* classBoxesListModel = new ValidationClassBoxesListModel{this, &it.value()};
        currentClassList->setModel(classBoxesListModel);
        _ui->tabWidget->addTab(currentClassList, it.key());
        _listWidgets[it.key()] = currentClassList;

        currentClassList->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(currentClassList, &QListView::customContextMenuRequested, this, [&, currentClassList](const QPoint &) {
            QMenu menu;
            for (auto const& className : _classBoxesList.keys())
            {
                if (className != it.key())
                {
                   menu.addAction(className);
                }
            }
            auto action = menu.exec(QCursor::pos());
            if (action)
            {
                auto currentListViewModel = currentClassList->selectionModel();
                auto selectedListViewModel = _listWidgets[action->text()]->model();
                for (auto selectedIndex : currentListViewModel->selectedIndexes())
                {
                    selectedListViewModel->insertRow(selectedListViewModel->rowCount(), selectedIndex);
                }
                while (true)
                {
                    auto selectedIndex = currentListViewModel->selectedIndexes();
                    if (selectedIndex.isEmpty())
                    {
                        break;
                    }
                    currentClassList->model()->removeRow(selectedIndex.front().row());
                }
            }
        });
    }

    progressDialog.close();
    _ui->tabWidget->show();
}

void ValidationClassBoxes::sync()
{
    for (auto datasetIt = _datasetList->begin(); datasetIt != _datasetList->end(); ++datasetIt)
    {
        QMap<QString, QVariant> classBoxes;
        for (auto classBoxIt = _classBoxesList.begin(); classBoxIt != _classBoxesList.end(); ++classBoxIt)
        {
            for (auto boxIt = classBoxIt->begin(); boxIt != classBoxIt->end(); ++boxIt)
            {
                if (boxIt->it == datasetIt)
                {
                    auto boxesList = classBoxes[classBoxIt.key()].toList();
                    boxesList.push_back(QList<QVariant>{boxIt->boxCoords.x(), boxIt->boxCoords.y(), boxIt->boxCoords.width(), boxIt->boxCoords.height()});
                    classBoxes[classBoxIt.key()] = boxesList;
                }
            }
        }
        datasetIt->setValue(classBoxes);
    }
}

ValidationClassBoxes::~ValidationClassBoxes()
{
    sync();
    emit datasetListUpdated();
    delete _ui;
}
