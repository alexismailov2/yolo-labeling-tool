#include "MainWindow.h"
#include "ui_MainWindow.h"

#include <QFileDialog>
#include <QColorDialog>
#include <QKeyEvent>
#include <QDebug>
#include <QShortcut>

#include <QtCore/QProcess>

#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>
#include <QtWidgets/QInputDialog>
#include <iomanip>

#include <ValidationClassBoxes.h>

namespace {

    auto openDatasetDir(QWidget* parent) -> QStringList
    {
        auto datasetDir = QDir{QFileDialog::getExistingDirectory(parent, parent->tr("Open Dataset Directory"), "./", QFileDialog::ShowDirsOnly)};
        auto datasetImagesList = datasetDir.entryList(QStringList() << "*.jpg" << "*.JPG" << "*.png" << "*.PNG", QDir::Files);
        if (datasetImagesList.empty())
        {
            QMessageBox::critical(parent, "Error", "Could not be found images for dataset!");
            return {};
        }
        auto const imgDir = datasetDir.canonicalPath();
        for (QString& str: datasetImagesList)
        {
          str = imgDir + "/" + str;
        }
        return datasetImagesList;
    }

} /// end namespace anonymous

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  _ui(new Ui::MainWindow)
{
  _ui->setupUi(this);

  connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this), &QShortcut::activated, this, &MainWindow::on_pushButton_save_clicked);
  connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this), &QShortcut::activated, this, &MainWindow::clearAllClassBoxes);
  connect(new QShortcut(QKeySequence(Qt::Key_S), this),            &QShortcut::activated, this, &MainWindow::nextClass);
  connect(new QShortcut(QKeySequence(Qt::Key_W), this),            &QShortcut::activated, this, &MainWindow::prevClass);
  connect(new QShortcut(QKeySequence(Qt::Key_A), this),            &QShortcut::activated, this, &MainWindow::on_pushButton_prev_clicked);
  connect(new QShortcut(QKeySequence(Qt::Key_D), this),            &QShortcut::activated, this, &MainWindow::on_pushButton_next_clicked);
  connect(new QShortcut(QKeySequence(Qt::Key_Space), this),        &QShortcut::activated, this, &MainWindow::on_pushButton_next_clicked);

  connect(_ui->label_image, &BoundingBoxSelector::datasetIteratorUpdated, this, &MainWindow::datasetListUpdated);

  init();
  initTableWidget();
}

MainWindow::~MainWindow()
{
  delete _ui;
}

void MainWindow::on_pushButton_create_from_video_clicked()
{
  if (!openVideos())
  {
    qDebug() << "Could not be opened video files";
  }
}

void MainWindow::on_pushButtonOpenProject_clicked()
{ 
    QString projectFilePath = QFileDialog::getOpenFileName(this, tr("Open project file"), "./", tr("Yolo labeling tool project (*.json)"));
    if (!projectFilePath.isEmpty())
    {
        _datasetProject.loadFromFile(projectFilePath);
        loadClassNameList();
        loadDatasetList();
    }
    init();
}

void MainWindow::on_pushButton_save_clicked()
{
   exportClassListToFile();
   for (auto it = _datasetList.begin(); it != _datasetList.end(); ++it)
   {
       _ui->label_image->exportClassBoxesToAnnotationFile(it, _classesList);
   }
}

void MainWindow::on_pushButton_remove_clicked()
{
   QFile::remove(_datasetIt.key());
   QFile::remove(toTxtExtention(_datasetIt.key()));
   _datasetIt = _datasetList.erase(_datasetIt);
   if (!_datasetList.isEmpty() && (_datasetIt == _datasetList.end()))
   {
       --_datasetIt;
   }
   setCurrentImg();
   updateButtonEnabling(!_datasetList.isEmpty());
}

void MainWindow::on_pushButton_prev_clicked()
{
   setPreviousImage();
}

void MainWindow::on_pushButton_next_clicked()
{
   setNextImage();
}

void MainWindow::on_tableWidget_label_cellDoubleClicked(int row, int column)
{
  switch(column)
  {
  case 0: {
    bool isAccepted{};
    auto newClassName = QInputDialog::getText(this, "Rename class", "Class name:", QLineEdit::Normal,  _ui->tableWidget_label->item(row, 0)->text(), &isAccepted, Qt::Dialog);
    if (isAccepted && !newClassName.isEmpty())
    {
      // TODO: Should be moved to datasetProject class
      auto const oldClassName = _ui->tableWidget_label->item(row, 0)->text();
      _classesList.remove(oldClassName);
      QList<QVariant> classData;
      classData.push_back(row);
      classData.push_back(static_cast<int>(_ui->tableWidget_label->item(row, 1)->backgroundColor().rgba()));
      _classesList[newClassName] = classData;
      _classesIt = _classesList.find(newClassName);
      _datasetProject.set("class_names_list", _classesList);

      for (auto it = _datasetList.begin(); it != _datasetList.end(); ++it)
      {
        auto classBoxes = it->toMap();
        classBoxes[newClassName] = classBoxes.take(oldClassName);
        it->setValue(classBoxes);
      }
      _datasetProject.set("dataset_list", _datasetList);

      _ui->label_image->loadClassBoxes(_datasetIt);

      _ui->tableWidget_label->item(row, 0)->setText(newClassName);
    }
    break;
  }
  case 1: {
    auto color = QColorDialog::getColor(Qt::white, nullptr, "Set Label Color");
    if(color.isValid())
    {
      // TODO: Should be moved to datasetProject class
      auto const className = _ui->tableWidget_label->item(row, 0)->text();
      QList<QVariant> classData;
      classData.push_back(row);
      classData.push_back(static_cast<int>(color.rgba()));
      _classesList[className] = classData;
      _classesIt = _classesList.find(className);
      _datasetProject.set("class_names_list", _classesList);

      _ui->tableWidget_label->item(row, 1)->setBackgroundColor(color);
    }
    break;
  }
  default:
    break;
  }
  updateCurrentClass();
  _ui->label_image->showImage();
}

void MainWindow::on_tableWidget_label_cellClicked(int row, int column)
{
   Q_UNUSED(column)
   _classesIt = _classesList.find(_ui->tableWidget_label->item(row, 0)->text());
   //_classesIt = std::next(_classesList.begin(), row);
   updateCurrentClass();
}

void MainWindow::on_horizontalSlider_images_sliderMoved(int position)
{
    _datasetIt = std::next(_datasetList.begin(), position);
   setCurrentImg();
}

void MainWindow::clearAllClassBoxes()
{
    _ui->label_image->clearAllClassBoxex();
}

void MainWindow::nextClass()
{
    if (_classesIt != _classesList.end())
    {
        ++_classesIt;
    }
    updateCurrentClass();
}

void MainWindow::prevClass()
{
    if (_classesIt != _classesList.begin())
    {
        --_classesIt;
    }
    updateCurrentClass();
}

// TODO: All logic for working with _datasetList and _classesList should be moved to DatasetProject class
void MainWindow::datasetListUpdated()
{
    _datasetProject.set("dataset_list", _datasetList);
}

void MainWindow::wheelEvent(QWheelEvent* ev)
{
    if (ev->delta() > 0)
    {
       setPreviousImage(); // up Wheel
    }
    else if(ev->delta() < 0)
    {
       setNextImage(); //down Wheel
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
   if(event->key() == Qt::Key_QuoteLeft)
   {
      updateCurrentClass();
   }
   else if ((event->key() >= Qt::Key_0) && (event->key() <= Qt::Key_9))
   {
      int const asciiToNumber = event->key() - Qt::Key_0;
      if (asciiToNumber < _classesList.size())
      {
         _classesIt = std::next(_classesList.begin(), asciiToNumber);
         updateCurrentClass();
      }
   }
}

void MainWindow::init()
{
    _ui->label_image->init();

    updateButtonEnabling(!_datasetList.isEmpty());
    updateDatasetNavigator();

    _classesIt = _classesList.begin();
    updateCurrentClass();

    _datasetIt = _datasetList.begin();
    setCurrentImg();
}

void MainWindow::initTableWidget()
{
  _ui->tableWidget_label->horizontalHeader()->setVisible(true);
  _ui->tableWidget_label->horizontalHeader()->setStretchLastSection(true);

  disconnect(_ui->tableWidget_label->horizontalHeader(), SIGNAL(sectionPressed(int)),_ui->tableWidget_label, SLOT(selectColumn(int)));

  initTableWidgetContextMenuSetup();
}

void MainWindow::initTableWidgetContextMenuSetup()
{
  _ui->tableWidget_label->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(_ui->tableWidget_label, &QTableWidget::customContextMenuRequested, this, [this]() {
      QMenu* menu = new QMenu;
      auto addClass = menu->addAction("Add class");
      auto point = _ui->tableWidget_label->mapFromGlobal(QCursor::pos() - QPoint{0, 25});
      auto selectedItemPtr = _ui->tableWidget_label->itemAt(point);
      if (selectedItemPtr)
      {
          auto removeClass = menu->addAction("Remove class");
          QObject::connect(removeClass, &QAction::triggered, this, [this, menu, removeClass, selectedItemPtr]() {
              _classesList.remove(selectedItemPtr->text());
              _classesIt = _classesList.begin();
              // TODO: May be should be set iterator _classesIt instead
              _ui->label_image->setFocusObjectLabel(!_classesList.isEmpty() ? _classesIt.key() : "");
              _datasetProject.set("class_names_list", _classesList);
              _ui->tableWidget_label->removeRow(selectedItemPtr->row());
              removeClass->deleteLater();
              menu->deleteLater();
          });
      }
      QObject::connect(addClass, &QAction::triggered, this, [this, menu, addClass]() {
          bool isAccepted{};
          while(true)
          {
              auto newClassName = QInputDialog::getText(this, "Create class", "Class name:", QLineEdit::Normal, "", &isAccepted, Qt::Dialog);
              if (isAccepted && !newClassName.isEmpty())
              {
                  if (_classesList.contains(newClassName))
                  {
                      // TODO: May be should be dialog box shown.
                      qDebug() << "The class with name " << newClassName << " has already exist, try again";
                      continue;
                  }
                  auto const lastRowIndex = _ui->tableWidget_label->rowCount();
                  auto classNameColor = QColor(QColor::colorNames().first());
                  qDebug() << newClassName << ", " << lastRowIndex << ", " << classNameColor;

                  _ui->tableWidget_label->insertRow(lastRowIndex);

                  _ui->tableWidget_label->setItem(lastRowIndex, 0, new QTableWidgetItem(newClassName));
                  _ui->tableWidget_label->item(lastRowIndex, 0)->setFlags(_ui->tableWidget_label->item(lastRowIndex, 0)->flags() ^ Qt::ItemIsEditable);

                  _ui->tableWidget_label->setItem(lastRowIndex, 1, new QTableWidgetItem(""));
                  _ui->tableWidget_label->item(lastRowIndex, 1)->setBackgroundColor(classNameColor);
                  _ui->tableWidget_label->item(lastRowIndex, 1)->setFlags(_ui->tableWidget_label->item(lastRowIndex, 1)->flags() ^ Qt::ItemIsEditable ^ Qt::ItemIsSelectable);

                  // TODO: May be should be set iterator _classesIt instead
                  _ui->label_image->setFocusObjectLabel(newClassName);
                  QList<QVariant> classData;
                  classData.push_back(lastRowIndex);
                  classData.push_back(static_cast<int>(classNameColor.rgba()));
                  _classesList[newClassName] = classData;
                  _classesIt = _classesList.find(newClassName);
                  _datasetProject.set("class_names_list", _classesList);

                  //ui->label_image->setClasses(&_classesList);
              }
              break;
          }
          addClass->deleteLater();
          menu->deleteLater();
      });
      menu->exec(QCursor::pos());
      menu->clear();
  });
}

void MainWindow::setCurrentImg()
{
    if (_datasetList.isEmpty())
    {
        return;
    }
    if (!_ui->label_image->openImage(_datasetIt.key()))
    {
        // TODO: Should be shown message box
        qDebug() << "Could not open file";
        return;
    }
    // TODO: May be more convinient to pass iterator to current dataset
    _ui->label_image->loadClassBoxes(_datasetIt);
    _ui->label_image->showImage();
    _ui->label_progress->setText(QString::number(std::distance(_datasetList.begin(), _datasetIt)) + " / " + QString::number(_datasetList.size() - 1));
    _ui->label_file->setText("File: " + _datasetIt.key());
    updateDatasetNavigator();
}

void MainWindow::setNextImage()
{
    if ((_datasetIt != _datasetList.end()) &&
        (std::next(_datasetIt) != _datasetList.end()))
    {
        ++_datasetIt;
    }
    setCurrentImg();
}

void MainWindow::setPreviousImage()
{
    if (_datasetIt != _datasetList.begin())
    {
        --_datasetIt;
    }
    setCurrentImg();
}

void MainWindow::updateCurrentClass()
{
    if (!_classesList.isEmpty())
    {
        _ui->label_image->setFocusObjectLabel(_classesIt.key());
        _ui->tableWidget_label->setCurrentCell(_classesIt.value().toList()[0].toInt(), 0);
    }
}

void MainWindow::updateClassesTable()
{
    qDebug() << _classesList.size();
    _ui->tableWidget_label->setRowCount(_classesList.size());
    for (auto it = _classesList.begin(); it != _classesList.end(); ++it)
    {
        auto const className = it.key();
        auto const classData = it->toList();
        // TODO: Should be fixed classData if wrong
        auto const classNameColor = (classData.size() == 2)
            ? QColor::fromRgba(static_cast<uint32_t>(classData[1].toInt()))
            : QColor(255, 255, 255, 255);
        auto const rowIndex = classData[0].toInt();
        qDebug() << className << ", " << rowIndex << ", " << classNameColor;

        _ui->tableWidget_label->setItem(rowIndex, 0, new QTableWidgetItem(className));
        _ui->tableWidget_label->item(rowIndex, 0)->setFlags(_ui->tableWidget_label->item(rowIndex, 0)->flags() ^ Qt::ItemIsEditable);

        _ui->tableWidget_label->setItem(rowIndex, 1, new QTableWidgetItem(""));
        _ui->tableWidget_label->item(rowIndex, 1)->setBackgroundColor(classNameColor);
        _ui->tableWidget_label->item(rowIndex, 1)->setFlags(_ui->tableWidget_label->item(rowIndex, 1)->flags() ^ Qt::ItemIsEditable ^ Qt::ItemIsSelectable);
    }
}

bool MainWindow::openVideos()
{
  auto selectedFileList = QFileDialog::getOpenFileNames(this, "Select one or more files to open", "./", "Video (*.mp4 *.avi *.webm)");
  if (!selectedFileList.isEmpty())
  {
      QProgressDialog progressDialog("Slicing video process", "&Cancel", 0, 100);
      progressDialog.setWindowModality(Qt::WindowModal);
      progressDialog.setMinimumSize(400, 40);
      progressDialog.setRange(0, 100);
      progressDialog.setValue(0);

      QProcess slicingDatasetProcess{};

      for (auto const& item : selectedFileList)
      {
        qDebug() << item;
        auto workingDirectoryPath = QDir::currentPath() + "/" + QFileInfo(item).fileName().split(".")[0];
        if (QDir().exists(workingDirectoryPath) || QDir().mkdir(workingDirectoryPath))
        {
          qDebug() << workingDirectoryPath;
        }
        else
        {
          qDebug() << "Could not create directory: " << workingDirectoryPath;
        }
        slicingDatasetProcess.setWorkingDirectory(workingDirectoryPath);
        QObject::connect(&slicingDatasetProcess, &QProcess::readyReadStandardError, [&]() {
            static auto durationSeconds = 0;

            auto toSeconds = [](std::string const& duration) {
               return (((duration[0] - '0')*10 + duration[1] - '0')*60*60) + (((duration[3] - '0')*10 + duration[4] - '0')*60) + ((duration[6] - '0')*10 + duration[7] - '0');
            };

            std::string string = slicingDatasetProcess.readAllStandardError().toStdString();
            auto durationStart = string.find("Duration: ");
            if (durationStart != std::string::npos)
            {
               auto durationString = string.substr(durationStart + 10, 8);
               durationSeconds = toSeconds(durationString);
               qDebug() << "Duration: " << QString::fromStdString(durationString) << ", " << durationSeconds;
            }
            auto start = string.rfind("time=");
            if (start != std::string::npos)
            {
               start += 5;
               auto end = string.rfind("bitrate=");
               if (end != std::string::npos)
               {
                  end -= 4;
                  auto currentPosString = string.substr(start, end - start);
                  auto currentPos = toSeconds(currentPosString);
                  qDebug() << QString::fromStdString(currentPosString) << ", " << currentPos;
                  progressDialog.setValue((currentPos * 100)/durationSeconds);
               }
            }
        });
        QObject::connect(&slicingDatasetProcess, static_cast<void(QProcess::*)(int)>(&QProcess::finished), [&](){
           progressDialog.close();
        });
        QStringList ffmpegArguments("-i");
        ffmpegArguments << item << "-vf" << "fps=1" << "%06d.png";//jpg";
        slicingDatasetProcess.start("/usr/local/bin/ffmpeg", ffmpegArguments);

        progressDialog.setLabelText(QString("Slicing video %1 to dataset").arg(item));

        auto isCanceled = false;
        QObject::connect(&progressDialog, &QProgressDialog::canceled, [&]() {
            isCanceled = true;
            slicingDatasetProcess.terminate();
        });
        progressDialog.exec();
        qDebug() << "Result of process: " << slicingDatasetProcess.waitForFinished();
        if (isCanceled)
        {
            qDebug() << "Cancelled slicing process";
            break;
        }
      }
      return true;
  }
  return false;
}

void MainWindow::updateButtonEnabling(bool isEnabled)
{
  _ui->pushButton_next->setEnabled(isEnabled);
  _ui->pushButton_prev->setEnabled(isEnabled);
  _ui->pushButton_save->setEnabled(isEnabled);
  _ui->pushButton_remove->setEnabled(isEnabled);
  _ui->tableWidget_label->setEnabled(isEnabled);
  _ui->label_image->setEnabled(isEnabled);
  _ui->pushButtonValidate->setEnabled(isEnabled);
}

void MainWindow::updateDatasetNavigator()
{
  _ui->horizontalSlider_images->setEnabled(true);
  _ui->horizontalSlider_images->setRange(0, _datasetList.size() - 1);
  _ui->horizontalSlider_images->blockSignals(true);
  _ui->horizontalSlider_images->setValue(std::distance(_datasetList.begin(), _datasetIt));
  _ui->horizontalSlider_images->blockSignals(false);
}

void MainWindow::loadClassNameList()
{
    _ui->tableWidget_label->clear();

    _classesList = _datasetProject.get("class_names_list", [&]() -> QVariantMap {
        QVariantMap data;
        auto classesFilePath = QFileDialog::getOpenFileName(this, tr("Open class list file"), "./", tr("Darknet class list file (*.txt *.names)"));
        if (!classesFilePath.isEmpty())
        {
            QFile classesFile{classesFilePath};
            if (classesFile.open(QIODevice::ReadOnly))
            {
                auto classIndex = 0;
                QTextStream in{&classesFile};
                while (!in.atEnd())
                {
                    QList<QVariant> classData;
                    classData.push_back(classIndex++);
                    classData.push_back(-1); // White color with alpha 255
                    data[in.readLine()] = classData;
                    qDebug() << classData;
                }
            }
        }
        return data;
    });

    _classesIt = _classesList.begin();
    _ui->label_image->setClasses(&_classesList);
    if (_classesList.isEmpty())
    {
        return;
    }
    _ui->label_image->setFocusObjectLabel(_classesIt.key());

    updateClassesTable();
}

void MainWindow::exportClassListToFile()
{
    auto const classNameFilePath = QFileDialog::getSaveFileName(this, tr("Save class list file"), QFileInfo(_datasetIt.key()).absoluteDir().absolutePath(), tr("Darknet class list file (*.txt *.names)"));
    if (classNameFilePath.isEmpty())
    {
        QMessageBox::warning(this, "Warning", "Since you reject selecting file for saving class names, it will not be saved!");
        return;
    }
    qDebug() << _classesList;
    auto classesList = _classesList.keys();
    std::sort(classesList.begin(), classesList.end(), [&](auto const& left, auto const& right) {
        return _classesList.find(left)->toList()[0] < _classesList.find(right)->toList()[0];
    });
    qDebug() << classesList;
    QFile classesFile{classNameFilePath};
    if (classesFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        QTextStream out(&classesFile);
        for (auto const& classItem : classesList)
        {
            out << classItem << '\n';
        }
    }
}

void MainWindow::loadDatasetList()
{
    _datasetList = _datasetProject.get("dataset_list", [&]() -> QVariantMap {
        QVariantMap data;
        for (auto const& item : openDatasetDir(this))
        {
            data[item] = _ui->label_image->importClassBoxesFromAnnotationFile(item, _classesList);
        }
        return data;
    });
    updateClassesTable();
    _datasetIt = _datasetList.begin();
    _ui->label_image->loadClassBoxes(_datasetIt);
}

void MainWindow::on_pushButtonValidate_clicked()
{
    ValidationClassBoxes validationClassBoxes{this, &_datasetList, &_classesList};
    connect(&validationClassBoxes, &ValidationClassBoxes::datasetListUpdated, this, &MainWindow::datasetListUpdated);
    validationClassBoxes.exec();
}
