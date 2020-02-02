#include "validationclassboxes.h"
#include "ui_validationclassboxes.h"

#include <QListWidget>
#include <QMenu>
#include <QDebug>
#include <QProgressDialog>

//#include <QStandardItemModel>
//#include <QStandardItem>
//#include <QFileSystemModel>
//#include <QDir>

namespace {
    // TODO: Should be moved to utils or datasetproject.cpp
    void extractClassBoxes(QVariantMap::iterator datasetIt, std::function<void(QString const&, QRectF&&)>&& extractedClassBoxFn)
    {
        if (!datasetIt->isValid())
        {
            return;
        }
        auto const classBoxes = datasetIt.value().toMap();
        for (auto classBoxesIt = classBoxes.begin(); classBoxesIt != classBoxes.end(); ++classBoxesIt)
        {
            for (auto box : classBoxesIt.value().toList())
            {
                auto const boxCoords = box.toList();
                extractedClassBoxFn(classBoxesIt.key(), QRectF{boxCoords[0].toReal(), boxCoords[1].toReal(), boxCoords[2].toReal(), boxCoords[3].toReal()});
            }
        }
    }

    // TODO: Should be moved to utils
    auto toAbsolute(QRectF const& in, QSize const& size) noexcept -> QRect
    {
        return QRect{static_cast<int>(in.x() * size.width() + 0.5),
                     static_cast<int>(in.y() * size.height() + 0.5),
                     static_cast<int>(in.width() * size.width() + 0.5),
                     static_cast<int>(in.height()* size.height() + 0.5)};
    }

} /// end namespace anonymous

ValidationClassBoxes::ValidationClassBoxes(QWidget *parent, QVariantMap* datasetList)
    : QDialog(parent)
    , _ui(new Ui::ValidationClassBoxes)
{
    _ui->setupUi(this);

    _ui->tabWidget->clear();

    QProgressDialog progressDialog("Generation crops process...", "&Cancel", 0, 100);
    progressDialog.setWindowModality(Qt::WindowModal);
    progressDialog.setMinimumSize(400, 40);
    progressDialog.setRange(0, 100);
    progressDialog.setValue(0);

    //QMap<QString, QListWidget*> listWidgets;
    for (auto datasetIt = datasetList->begin(); datasetIt != datasetList->end(); ++datasetIt)
    {
        progressDialog.setValue((std::distance(datasetList->begin(), datasetIt) * 100)/datasetList->size());
        QImage image{datasetIt.key()};
        extractClassBoxes(datasetIt, [&](QString const& className, QRectF&& boxRect) {
            auto cropped = image.copy(toAbsolute(std::move(boxRect), image.size()));
            if (!cropped.isNull())
            {
                if (!_listWidgets.contains(className))
                {
                    QListWidget* currentClassList = new QListWidget(this);
                    currentClassList->setViewMode(QListWidget::IconMode);
                    currentClassList->setIconSize(QSize(100,100));
                    currentClassList->setResizeMode(QListWidget::Adjust);
                    currentClassList->setSelectionMode(QAbstractItemView::ExtendedSelection);
                    _ui->tabWidget->addTab(currentClassList, className);
                    _listWidgets[className] = currentClassList;
                }
                QListWidgetItem* listWidgetItem = new QListWidgetItem(QString::number(std::distance(datasetList->begin(), datasetIt)));
                listWidgetItem->setIcon(QPixmap::fromImage(cropped));
                _listWidgets[className]->addItem(listWidgetItem);
            }
            // TODO: Should be moved to dedicated function
            // auto croppedImageFile = datasetIt.key().toStdString();
            // croppedImageFile = croppedImageFile.substr(croppedImageFile.find_last_of('/') + 1,
            //                                            croppedImageFile.find_last_of('.') - croppedImageFile.find_last_of('/') - 1);
            // cropped.save(QString::fromStdString(croppedImageFile) + "_cropped_" + QString::number(i) + ".png");
        });
    }

    for (auto it = _listWidgets.begin(); it != _listWidgets.end(); ++it)
    {
        auto listWidgetPtr = it.value();
        listWidgetPtr->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(listWidgetPtr, &QListWidget::customContextMenuRequested, this, [=](const QPoint &) {
            QMenu menu;
            for (auto const& className : _listWidgets.keys())
            {
                menu.addAction(className);
            }
            auto action = menu.exec(QCursor::pos());
            if (action)
            {
                for (auto selectedItem : listWidgetPtr->selectedItems())
                {
                   auto takenItem = listWidgetPtr->takeItem(listWidgetPtr->row(selectedItem));
                   _listWidgets[action->text()]->addItem(takenItem);
                   // TODO: Should be added synchronization to the datasetJson file
                }
            }
        });
    }
    progressDialog.close();
    _ui->tabWidget->show();
}

ValidationClassBoxes::~ValidationClassBoxes()
{
    delete _ui;
}
