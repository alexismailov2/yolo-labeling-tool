#include "mainwindow.h"
#include "ui_mainwindow.h"

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
  ui(new Ui::MainWindow)
{
  ui->setupUi(this);

  connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), this), SIGNAL(activated()), this, SLOT(save_label_data()));
  connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_C), this), SIGNAL(activated()), this, SLOT(clear_label_data()));

  connect(new QShortcut(QKeySequence(Qt::Key_S), this), SIGNAL(activated()), this, SLOT(nextClass()));
  connect(new QShortcut(QKeySequence(Qt::Key_W), this), SIGNAL(activated()), this, SLOT(prevClass()));
  connect(new QShortcut(QKeySequence(Qt::Key_A), this), SIGNAL(activated()), this, SLOT(setPreviousImage()));
  connect(new QShortcut(QKeySequence(Qt::Key_D), this), SIGNAL(activated()), this, SLOT(setNextImage()));
  connect(new QShortcut(QKeySequence(Qt::Key_Space), this), SIGNAL(activated()), this, SLOT(setNextImage()));

  connect(ui->label_image, &BoundingBoxSelector::datasetIteratorUpdated, this, &MainWindow::datasetIteratorUpdated);

  initTableWidget();
}

MainWindow::~MainWindow()
{
  delete ui;
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

void MainWindow::wheelEvent(QWheelEvent *ev)
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

void MainWindow::on_pushButton_prev_clicked()
{
   setPreviousImage();
}

void MainWindow::on_pushButton_next_clicked()
{
   setNextImage();
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
   int  nKey                    = event->key();
   bool graveAccentKeyIsPressed = (nKey == Qt::Key_QuoteLeft);
   bool numKeyIsPressed         = (nKey >= Qt::Key_0 && nKey <= Qt::Key_9 );

   if(graveAccentKeyIsPressed)
   {
      updateCurrentClass();
   }
   else if(numKeyIsPressed)
   {
      int asciiToNumber = nKey - Qt::Key_0;
      if (asciiToNumber < _classesList.size())
      {
         _classesIt = std::next(_classesList.begin(), asciiToNumber);
         updateCurrentClass();
      }
   }
}

void MainWindow::on_pushButton_save_clicked()
{
   for (auto it = _datasetList.begin(); it != _datasetList.end(); ++it)
   {
       ui->label_image->exportClassBoxesToAnnotationFile(it, _classesList.keys());
   }
}

void MainWindow::on_pushButton_remove_clicked()
{
// TODO: Should be adopted
#if 0
   QFile::remove(m_imgList.at(m_imgIndex));

   // TODO: Text file should not be created every time all info should be stored in the json
   QString qstrOutputLabelData = get_labeling_data(m_imgList.at(m_imgIndex));
   QFile::remove(qstrOutputLabelData);

   m_imgList.removeAt(m_imgIndex);

   if(m_imgIndex == m_imgList.size())
   {
      m_imgIndex--;
      setCurrentImg(m_imgIndex);
   }

   updateButtonEnabling(!m_imgList.isEmpty());
#endif
}

void MainWindow::on_tableWidget_label_cellDoubleClicked(int row, int column)
{
  switch(column)
  {
  case 0: {
    bool isAccepted{};
    auto newClassName = QInputDialog::getText(this, "Rename class", "Class name:", QLineEdit::Normal,  ui->tableWidget_label->item(row, 0)->text(), &isAccepted, Qt::Dialog);
    if (isAccepted && !newClassName.isEmpty())
    {
      _classesList.remove(ui->tableWidget_label->item(row, 0)->text());
      _classesList[newClassName] = static_cast<int>(ui->tableWidget_label->item(row, 1)->backgroundColor().rgba());
      _classesIt = _classesList.find(newClassName);
      _datasetProject.set("class_names_list", _classesList);

      ui->tableWidget_label->item(row, 0)->setText(newClassName);
    }
    break;
  }
  case 1: {
    auto color = QColorDialog::getColor(Qt::white, nullptr, "Set Label Color");
    if(color.isValid())
    {
      auto const className = ui->tableWidget_label->item(row, 0)->text();
      _classesList[className] = static_cast<int>(color.rgba());
      _classesIt = _classesList.find(className);
      _datasetProject.set("class_names_list", _classesList);

      ui->tableWidget_label->item(row, 1)->setBackgroundColor(color);
    }
    break;
  }
  default:
    break;
  }
  updateCurrentClass();
  ui->label_image->showImage();
}

void MainWindow::on_tableWidget_label_cellClicked(int row, int column)
{
   Q_UNUSED(column)
   _classesIt = std::next(_classesList.begin(), row);
   updateCurrentClass();
}

void MainWindow::on_horizontalSlider_images_sliderMoved(int position)
{
    _datasetIt = std::next(_datasetList.begin(), position);
   setCurrentImg();
}

void MainWindow::clear_label_data()
{
    ui->label_image->clearAllClassBoxex();
}

void MainWindow::init()
{
    ui->label_image->init();

    updateButtonEnabling(!_datasetList.isEmpty());
    updateDatasetNavigator();

    _classesIt = _classesList.begin();
    updateCurrentClass();

    _datasetIt = _datasetList.begin();
    setCurrentImg();
}

void MainWindow::setCurrentImg()
{
    if (_datasetList.isEmpty())
    {
        return;
    }
    if (!ui->label_image->openImage(_datasetIt.key()))
    {
        // TODO: Should be shown message box
        qDebug() << "Could not open file";
        return;
    }
    // TODO: May be more convinient to pass iterator to current dataset
    ui->label_image->loadClassBoxes(_datasetIt);
    ui->label_image->showImage();
    ui->label_progress->setText(QString::number(std::distance(_datasetList.begin(), _datasetIt)) + " / " + QString::number(_datasetList.size() - 1));
    ui->label_file->setText("File: " + _datasetIt.key());
    updateDatasetNavigator();
}

void MainWindow::setNextImage()
{
    if (std::next(_datasetIt) != _datasetList.end())
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
        ui->label_image->setFocusObjectLabel(_classesIt.key());
        ui->tableWidget_label->setCurrentCell(std::distance(_classesList.begin(), _classesIt), 0);
    }
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

bool MainWindow::openVideos()
{
  auto selectedFileList = QFileDialog::getOpenFileNames(this, "Select one or more files to open", "./", "Video (*.mp4 *.avi *.webm)");

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

    if (m_SlicingDatasetProcess == nullptr)
    {
        m_SlicingDatasetProcess = new QProcess(this);
    }
    m_SlicingDatasetProcess->setWorkingDirectory(workingDirectoryPath);
    QObject::connect(m_SlicingDatasetProcess, &QProcess::readyReadStandardError, [&]() {
        static auto durationSeconds = 0;

        auto toSeconds = [](std::string const& duration) {
           return (((duration[0] - '0')*10 + duration[1] - '0')*60*60) + (((duration[3] - '0')*10 + duration[4] - '0')*60) + ((duration[6] - '0')*10 + duration[7] - '0');
        };

        std::string string = m_SlicingDatasetProcess->readAllStandardError().toStdString();
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
              m_progressDialog->setValue((currentPos * 100)/durationSeconds);
           }
        }
    });
    QObject::connect(m_SlicingDatasetProcess, static_cast<void(QProcess::*)(int)>(&QProcess::finished), [&](){
       m_progressDialog->close();
    });
    QStringList ffmpegArguments("-i");
    ffmpegArguments << item << "-vf" << "fps=1" << "%06d.png";//jpg";
    m_SlicingDatasetProcess->start("/usr/local/bin/ffmpeg", ffmpegArguments);

    if (m_progressDialog == nullptr)
    {
      m_progressDialog = new QProgressDialog("Slicing video process", "&Cancel", 0, 100);
      m_progressDialog->setWindowModality(Qt::WindowModal);
      m_progressDialog->setMinimumSize(400, 40);
      m_progressDialog->setRange(0, 100);
      m_progressDialog->setValue(0);
    }

    m_progressDialog->setLabelText(QString("Slicing video %1 to dataset").arg(item));

    auto isCanceled = false;
    QObject::connect(m_progressDialog, &QProgressDialog::canceled, [&]() {
        isCanceled = true;
        m_SlicingDatasetProcess->terminate();
    });
    m_progressDialog->exec();
    qDebug() << "Result of process: " << m_SlicingDatasetProcess->waitForFinished();
    if (isCanceled)
    {
        qDebug() << "Cancelled slicing process";
        break;
    }
  }
  return true;
}

void MainWindow::loadClassNameList()
{
    ui->tableWidget_label->clear();

    _classesList = _datasetProject.get("class_names_list");
    _classesIt = _classesList.begin();
    ui->label_image->setClasses(&_classesList);
    if (_classesList.isEmpty())
    {
        return;
    }
    ui->label_image->setFocusObjectLabel(_classesIt.key());
    for (auto it = _classesList.begin(); it != _classesList.end(); ++it)
    {
        auto className = it.key();
        auto classNameColor = QColor::fromRgba(static_cast<uint32_t>(it->toInt()));
        qDebug() << className << ", " << classNameColor;

        auto const lastRowIndex = ui->tableWidget_label->rowCount();
        ui->tableWidget_label->insertRow(lastRowIndex);

        ui->tableWidget_label->setItem(lastRowIndex, 0, new QTableWidgetItem(className));
        ui->tableWidget_label->item(lastRowIndex, 0)->setFlags(ui->tableWidget_label->item(lastRowIndex, 0)->flags() ^ Qt::ItemIsEditable);

        ui->tableWidget_label->setItem(lastRowIndex, 1, new QTableWidgetItem(""));
        ui->tableWidget_label->item(lastRowIndex, 1)->setBackgroundColor(classNameColor);
        ui->tableWidget_label->item(lastRowIndex, 1)->setFlags(ui->tableWidget_label->item(lastRowIndex, 1)->flags() ^ Qt::ItemIsEditable ^ Qt::ItemIsSelectable);
    }
}

void MainWindow::updateButtonEnabling(bool isEnabled)
{
  ui->pushButton_next->setEnabled(isEnabled);
  ui->pushButton_prev->setEnabled(isEnabled);
  ui->pushButton_save->setEnabled(isEnabled);
  ui->pushButton_remove->setEnabled(isEnabled);
}

void MainWindow::updateDatasetNavigator()
{
  ui->horizontalSlider_images->setEnabled(true);
  ui->horizontalSlider_images->setRange(0, _datasetList.size() - 1);
  ui->horizontalSlider_images->blockSignals(true);
  ui->horizontalSlider_images->setValue(std::distance(_datasetList.begin(), _datasetIt));
  ui->horizontalSlider_images->blockSignals(false);
}

void MainWindow::initTableWidgetContextMenuSetup()
{
  ui->tableWidget_label->setContextMenuPolicy(Qt::CustomContextMenu);
  QObject::connect(ui->tableWidget_label, &QTableWidget::customContextMenuRequested, this, [this]() {
      QMenu* menu = new QMenu;
      auto addClass = menu->addAction("Add class");
      auto point = ui->tableWidget_label->mapFromGlobal(QCursor::pos() - QPoint{0, 25});
      auto selectedItemPtr = ui->tableWidget_label->itemAt(point);
      if (selectedItemPtr)
      {
          auto removeClass = menu->addAction("Remove class");
          QObject::connect(removeClass, &QAction::triggered, this, [this, menu, removeClass, selectedItemPtr]() {
              _classesList.remove(selectedItemPtr->text());
              _classesIt = _classesList.begin();
              // TODO: May be should be set iterator _classesIt instead
              ui->label_image->setFocusObjectLabel(!_classesList.isEmpty() ? _classesIt.key() : "");
              _datasetProject.set("class_names_list", _classesList);
              ui->tableWidget_label->removeRow(selectedItemPtr->row());
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
                  auto const lastRowIndex = ui->tableWidget_label->rowCount();
                  auto classNameColor = QColor(QColor::colorNames().first());
                  qDebug() << newClassName << ", " << lastRowIndex << ", " << classNameColor;

                  ui->tableWidget_label->insertRow(lastRowIndex);

                  ui->tableWidget_label->setItem(lastRowIndex, 0, new QTableWidgetItem(newClassName));
                  ui->tableWidget_label->item(lastRowIndex, 0)->setFlags(ui->tableWidget_label->item(lastRowIndex, 0)->flags() ^ Qt::ItemIsEditable);

                  ui->tableWidget_label->setItem(lastRowIndex, 1, new QTableWidgetItem(""));
                  ui->tableWidget_label->item(lastRowIndex, 1)->setBackgroundColor(classNameColor);
                  ui->tableWidget_label->item(lastRowIndex, 1)->setFlags(ui->tableWidget_label->item(lastRowIndex, 1)->flags() ^ Qt::ItemIsEditable ^ Qt::ItemIsSelectable);

                  // TODO: May be should be set iterator _classesIt instead
                  ui->label_image->setFocusObjectLabel(newClassName);
                  _classesList[newClassName] = static_cast<int>(classNameColor.rgba());
                  _classesIt = _classesList.find(newClassName);
                  _datasetProject.set("class_names_list", _classesList);
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

void MainWindow::initTableWidget()
{
  ui->tableWidget_label->horizontalHeader()->setVisible(true);
  ui->tableWidget_label->horizontalHeader()->setStyleSheet("");
  ui->tableWidget_label->horizontalHeader()->setStretchLastSection(true);

  disconnect(ui->tableWidget_label->horizontalHeader(), SIGNAL(sectionPressed(int)),ui->tableWidget_label, SLOT(selectColumn(int)));

  initTableWidgetContextMenuSetup();
}

void MainWindow::loadDatasetList()
{
    _datasetList = _datasetProject.get("dataset_list", [&]() -> QVariantMap {
        QVariantMap data;
        for (auto const& item : openDatasetDir(this))
        {
            data[item] = {};
        }
        return data;
    });
    _datasetIt = _datasetList.begin();
    ui->label_image->loadClassBoxes(_datasetIt);
}

void MainWindow::datasetIteratorUpdated()
{
    _datasetProject.set("dataset_list", _datasetList);
}
