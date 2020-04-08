#pragma once

#include <QDialog>

#include <QtCharts/QChartView>
#include <QtCore/QAbstractTableModel>
#include <QtWidgets/QTableView>

#include "DatasetProject.h"

namespace Ui {
class BalanceObjectsDialog;
}

class BalanceObjectsModel : public QAbstractTableModel
{
Q_OBJECT

public:
  using tList = QList<QPair<QString, QMap<QString, size_t>>>;

public:
  BalanceObjectsModel(QObject *parent = nullptr,
                      tList* list = nullptr,
                      QStringList const& classesList = {},
                      Qt::GlobalColor color = {});

  auto rowCount(QModelIndex const& parent = QModelIndex()) const -> int32_t override;
  auto columnCount(QModelIndex const& parent = QModelIndex()) const -> int32_t override;
  auto data(QModelIndex const& index, int32_t role = Qt::DisplayRole) const -> QVariant override;
  auto headerData(int32_t section, Qt::Orientation orientation, int32_t role = Qt::DisplayRole) const -> QVariant override;
  bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
  bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;

private:
  tList*          _classesCountList;
  QStringList     _classesList;
  Qt::GlobalColor _color;
};

class BalanceObjectsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BalanceObjectsDialog(QWidget *parent = nullptr,
                                  QVariantMap* datasetList = nullptr,
                                  QVariantMap* classesList = nullptr);
    ~BalanceObjectsDialog();

private:
  auto createChart(DatasetProject::eItemType neededItemType) -> QtCharts::QChart*;
  void createGistograms();
  void createTable(QStringList const& classList);
  void initTableWidgetContextMenuSetup(QString const& tableName, QTableView* tableView);

private:
    Ui::BalanceObjectsDialog*                 _ui;
    QVariantMap*                              _datasetList;
    QtCharts::QChartView                      _chartViewTrain;
    QtCharts::QChartView                      _chartViewValid;
    QMap<QString, BalanceObjectsModel::tList> _balanceObjectsModelLists;
    //QMap<QString, BalanceObjectsModel>        _balanceObjectsModels;
    QMap<QString, QTableView*>                 _listViews;
};