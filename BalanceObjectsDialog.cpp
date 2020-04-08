#include "BalanceObjectsDialog.h"

#include "ui_BalanceObjectsDialog.h"

#include "Utils.h"

#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QBarSet>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QVXYModelMapper>
#include <QtCharts/QLineSeries>

#include <QtWidgets/QTableView>

#include <QMenu>
#include <QDebug>

BalanceObjectsModel::BalanceObjectsModel(QObject *parent,
                                         tList* classesCountList,
                                         QStringList const& classesList,
                                         Qt::GlobalColor color)
  : QAbstractTableModel(parent)
  , _classesCountList{classesCountList}
  , _classesList{classesList}
  , _color{color}
{
}

auto BalanceObjectsModel::rowCount(const QModelIndex &parent) const -> int32_t
{
  Q_UNUSED(parent);
  return _classesCountList->count();
}

auto BalanceObjectsModel::columnCount(const QModelIndex &parent) const -> int32_t
{
  Q_UNUSED(parent);
  return _classesList.count();
}

auto BalanceObjectsModel::data(const QModelIndex &index, int role) const -> QVariant
{
  if (!index.isValid())
  {
    return QVariant();
  }
  if (role == Qt::BackgroundRole)
  {
    return QBrush(QColor(_color));
  }
  if (role != Qt::DisplayRole)
  {
    auto const& listItem = (*_classesCountList)[index.row()];
    auto found = listItem.second.find(_classesList[index.column()]);
    return (found != listItem.second.end()) ? (int)*found : 0;
  }
}

auto BalanceObjectsModel::headerData(int section, Qt::Orientation orientation, int role) const -> QVariant
{
  if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
  {
    return _classesList[section];
  }
  return QVariant();
}

bool BalanceObjectsModel::insertRows(int row, int count, const QModelIndex &parent)
{
  beginInsertRows(parent, row, row + count - 1);
  while(count--)
  {
    auto intAsPtrToClassObject = reinterpret_cast<tList::value_type*>(parent.model()->data(parent, Qt::UserRole + 1).toLongLong());
    _classesCountList->insert(_classesCountList->count(), *intAsPtrToClassObject);
  }
  endInsertRows();
  return true;
}

bool BalanceObjectsModel::removeRows(int row, int count, const QModelIndex &parent)
{
  beginRemoveRows(parent, row, row + count - 1);
  for(auto current = row + count - 1; current >= row; --current)
  {
    _classesCountList->removeAt(current);
  }
  endRemoveRows();
  return true;
}

void BalanceObjectsDialog::initTableWidgetContextMenuSetup(QString const& listType, QTableView* tableView)
{
  tableView->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(tableView, &QTableView::customContextMenuRequested, this, [this, listType, tableView]() {
    const std::array<QString, 3> tableNamesList = {{"train", "valid", "excluded"}};
    QMenu menu;
    for (auto const& item : tableNamesList)
    {
      if (item != listType)
      {
        menu.addAction(item);
      }
    }
    auto action = menu.exec(QCursor::pos());
    if (action)
    {
      auto selectedIndexes = tableView->selectionModel()->selectedIndexes();
      auto selectedListViewModel = _listViews[action->text()]->model();
      for (auto selectedIndex : selectedIndexes)
      {
        selectedListViewModel->insertRow(selectedListViewModel->rowCount(),
                                         selectedIndex);
      }
      while (true)
      {
        if (selectedIndexes.isEmpty())
        {
          break;
        }
        _listViews[listType]->model()->removeRow(selectedIndexes.front().row());
      }
    }
    //createGistograms();
  });
}

BalanceObjectsDialog::BalanceObjectsDialog(QWidget *parent,
                                           QVariantMap* datasetList,
                                           QVariantMap* classesList)
    : QDialog(parent)
    , _ui(new Ui::BalanceObjectsDialog)
    , _datasetList{datasetList}
{
  _ui->setupUi(this);

  if (datasetList == nullptr)
  {
    return;
  }
  for (auto datasetIt = datasetList->begin(); datasetIt != datasetList->end(); ++datasetIt)
  {
    //auto const currentItemType =
    //    static_cast<DatasetProject::eItemType>(datasetIt.value().toMap()["selectedDataset"].toInt());
    // if (currentItemType != neededItemType)
    //{
    //  continue;
    //}
    QMap<QString, size_t> classesCount;
    extractClassesCount(datasetIt, [&](QString const& className, size_t count) {
      if (className != "excluded_from_annotation")
      {
        classesCount[className] += count;
      }
    });
    _balanceObjectsModelLists["excluded"].push_back(QPair{datasetIt.key(), classesCount});
  }

  createTable(classesList->keys());
  createGistograms();
}

BalanceObjectsDialog::~BalanceObjectsDialog()
{
    delete _ui;
}

auto BalanceObjectsDialog::createChart(DatasetProject::eItemType neededItemType) -> QtCharts::QChart*
{
   QMap<QString, size_t> classesCount;
   for (auto datasetIt = _datasetList->begin(); datasetIt != _datasetList->end(); ++datasetIt)
   {
      auto const currentItemType =
              static_cast<DatasetProject::eItemType>(datasetIt.value().toMap()["selectedDataset"].toInt());
      if (currentItemType != neededItemType)
      {
         continue;
      }
      extractClassesCount(datasetIt, [&](QString const& className, size_t count) {
          if (className != "excluded_from_annotation")
          {
             classesCount[className] += count;
          }
      });
   }
   auto* objectsFrequency = new QtCharts::QBarSet(neededItemType == DatasetProject::eItemType::TRAIN ? "Train" : "Valid");
   auto maxSize = 0;
   for (auto it = classesCount.begin(); it != classesCount.end(); ++it)
   {
      *objectsFrequency << it.value();
      maxSize = (it.value() > maxSize) ? it.value() : maxSize;
   }

   auto* barseries = new QtCharts::QStackedBarSeries();
   barseries->append(objectsFrequency);

   auto* chart = new QtCharts::QChart();
   chart->addSeries(barseries);
   chart->update();

   auto* axisX = new QtCharts::QBarCategoryAxis();
   axisX->append(classesCount.keys());
   chart->addAxis(axisX, Qt::AlignBottom);
   barseries->attachAxis(axisX);
   axisX->setRange(QString("Jan"), QString("Jun"));

   auto* axisY = new QtCharts::QValueAxis();
   chart->addAxis(axisY, Qt::AlignLeft);
   barseries->attachAxis(axisY);
   axisY->setRange(0, maxSize);

   return chart;
}

void BalanceObjectsDialog::createGistograms()
{
   _chartViewTrain.setChart(createChart(DatasetProject::eItemType::TRAIN));
   _chartViewTrain.setRenderHint(QPainter::Antialiasing);
   _chartViewTrain.update();
   _ui->verticalLayout_2->addWidget(&_chartViewTrain);

   _chartViewValid.setChart(createChart(DatasetProject::eItemType::VALID));
   _chartViewValid.setRenderHint(QPainter::Antialiasing);
   _chartViewValid.update();
   _ui->verticalLayout_2->addWidget(&_chartViewValid);

}

void BalanceObjectsDialog::createTable(QStringList const& classList)
{
  {
    auto tableTrain = new QTableView(this);
    auto* balanceObjectsModel = new BalanceObjectsModel{this, &_balanceObjectsModelLists["train"], classList, Qt::GlobalColor::green};
    tableTrain->setModel(balanceObjectsModel);
    tableTrain->horizontalHeader()->setVisible(true);
    tableTrain->verticalHeader()->hide();
    tableTrain->setSelectionBehavior(QAbstractItemView::SelectRows);
    initTableWidgetContextMenuSetup("train", tableTrain);
    _ui->verticalLayout->addWidget(tableTrain);
    _listViews["train"] = tableTrain;
  }

  {
    auto tableValid = new QTableView(this);
    auto* balanceObjectsModel = new BalanceObjectsModel{this, &_balanceObjectsModelLists["valid"], classList, Qt::GlobalColor::yellow};
    tableValid->setModel(balanceObjectsModel);
    tableValid->horizontalHeader()->setVisible(true);
    tableValid->verticalHeader()->hide();
    tableValid->setSelectionBehavior(QAbstractItemView::SelectRows);
    initTableWidgetContextMenuSetup("valid", tableValid);
    _ui->verticalLayout->addWidget(tableValid);
    _listViews["valid"] = tableValid;
  }

  {
    auto tableExcluded = new QTableView(this);
    auto* balanceObjectsModel = new BalanceObjectsModel{this, &_balanceObjectsModelLists["excluded"], classList, Qt::GlobalColor::white};
    tableExcluded->setModel(balanceObjectsModel);
    tableExcluded->horizontalHeader()->setVisible(true);
    tableExcluded->verticalHeader()->hide();
    tableExcluded->setSelectionBehavior(QAbstractItemView::SelectRows);
    initTableWidgetContextMenuSetup("excluded", tableExcluded);
    _ui->verticalLayout->addWidget(tableExcluded);
    _listViews["excluded"] = tableExcluded;
  }
}
