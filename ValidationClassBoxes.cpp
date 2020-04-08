#include "ValidationClassBoxes.h"
#include "ui_ValidationClassBoxes.h"

#include "Utils.h"

#include <QDebug>
#include <QMenu>
#include <QProgressDialog>

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QVXYModelMapper>
#include <QtCharts/QLineSeries>

#include <QtWidgets/QTableView>

ValidationClassBoxes::ValidationClassBoxes(QWidget *parent, QVariantMap* datasetList, QVariantMap* classList)
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

    for (auto const& item : classList->keys())
    {
        _classBoxesList[item];
    }

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
        connect(currentClassList, &QListView::customContextMenuRequested, this, [&, currentClassList, currentClassKey = it.key()](const QPoint &) {
            QMenu menu;
            for (auto const& className : _classBoxesList.keys())
            {
                if (className != currentClassKey)
                {
                   menu.addAction(className);
                }
            }
            menu.addAction("remove");
            auto action = menu.exec(QCursor::pos());
            if (action)
            {
                auto currentListViewModel = currentClassList->selectionModel();
                if (action->text() != "remove")
                {
                    auto selectedListViewModel = _listWidgets[action->text()]->model();
                    for (auto selectedIndex : currentListViewModel->selectedIndexes())
                    {
                        selectedListViewModel->insertRow(selectedListViewModel->rowCount(), selectedIndex);
                    }
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
            createGistogram();
            _chartView.update();
        });
    }

    createGistogram();
    _ui->tabWidget->addTab(&_chartView, "Gistogram");

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
        // TODO: Should be fixed
        //auto dataItem = datasetIt.value().toMap();
        //auto dataItemClassBoxes = dataItem["classBoxes"];
        datasetIt->setValue(classBoxes);
    }
}

void ValidationClassBoxes::createGistogram()
{
  auto* objectsFrequency = new QtCharts::QBarSet("Count");
  QStringList categories;
  auto maxSize = 0;
  for (auto it = _classBoxesList.begin(); it != _classBoxesList.end(); ++it)
  {
    if (it.key() == "excluded_from_annotation")
    {
      continue;
    }
    categories << it.key();
    *objectsFrequency << it->size();
    maxSize = (it->size() > maxSize) ? it->size() : maxSize;
  }

  auto* barseries = new QtCharts::QStackedBarSeries();
  barseries->append(objectsFrequency);

  auto* chart = new QtCharts::QChart();
  chart->addSeries(barseries);
  chart->setTitle("Objects frequency");
  chart->update();

  auto* axisX = new QtCharts::QBarCategoryAxis();
  axisX->append(categories);
  chart->addAxis(axisX, Qt::AlignBottom);
  //lineseries->attachAxis(axisX);
  barseries->attachAxis(axisX);
  axisX->setRange(QString("Jan"), QString("Jun"));

  auto* axisY = new QtCharts::QValueAxis();
  chart->addAxis(axisY, Qt::AlignLeft);
  //lineseries->attachAxis(axisY);
  barseries->attachAxis(axisY);
  axisY->setRange(0, maxSize);

  _chartView.setChart(chart);
  _chartView.setRenderHint(QPainter::Antialiasing);
}

ValidationClassBoxes::~ValidationClassBoxes()
{
    sync();
    emit datasetListUpdated();
    delete _ui;
}
