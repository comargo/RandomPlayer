#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QFile>
#include <QFileDialog>
#include <QListView>
#include <QSettings>
#include <QSplitter>
#include <QStandardPaths>
#include <QTextStream>

#include <QDragEnterEvent>
#include <QMediaPlaylist>
#include <QMimeData>
#include <QTimer>

#include <QtDebug>

static auto StoredPlaylistStr = QStringLiteral("RandomPlayer.rpl");
static auto PlaylistStr = QStringLiteral("Playlist");
static auto PlaylistLastDirStr = QStringLiteral("Playlist/LastDir");
static auto PlaylistFilterStr = QStringLiteral("RandomPlayer playlist (*.rpl);;All files (*.*)");
static auto MediaFileFilterStr = QStringLiteral("All supported (*.mp3; *.oog);;MP3 Audio file (*.mp3);;OOG Audio file (*.oog);;All files (*.*)");


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    model = new QStringListModel(this);
    ui->fileListView->setModel(model);
//    ui->fileListView->setDragDropOverwriteMode()
    player = new QMediaPlayer(this);

    connectSignals();

    auto playlist = QStandardPaths::locate(QStandardPaths::AppLocalDataLocation, StoredPlaylistStr);
    if(!playlist.isEmpty())
        loadPlaylist(playlist);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectSignals()
{
    connect(ui->btnLoadPlaylist, &QAbstractButton::clicked, this, &MainWindow::onLoadPlaylist);
    connect(ui->btnSavePlaylist, &QAbstractButton::clicked, this, &MainWindow::onSavePlaylist);
    connect(ui->btnAddFiles, &QAbstractButton::clicked, this, &MainWindow::onAddFiles);
    connect(ui->btnRemoveFiles, &QAbstractButton::clicked, this, &MainWindow::onRemoveFiles);
    connect(ui->btnStart, &QAbstractButton::clicked, this, &MainWindow::onStartPlay);
    connect(ui->btnSpeedUp, &QAbstractButton::clicked, this, &MainWindow::onSpeedUp);

//    connect(player, &QMediaPlayer::mediaChanged, this, [this](){
//        player->pause();
//        QTimer::singleShot(delay, player, &QMediaPlayer::play);
//    });
}

void MainWindow::onLoadPlaylist()
{
    QSettings settings;
    auto playlist = QFileDialog::getOpenFileName(this, tr("Open playlist"),
                                 settings.value(PlaylistLastDirStr).toString(),
                                 PlaylistFilterStr);
    if(playlist.isEmpty())
        return;
    if(!loadPlaylist(playlist))
        return;
    settings.setValue(PlaylistLastDirStr, QFileInfo(playlist).absolutePath());
}

void MainWindow::onSavePlaylist()
{
    QSettings settings;
    auto playlist = QFileDialog::getSaveFileName(this, tr("Save playlist"),
                                 settings.value(PlaylistLastDirStr).toString(),
                                 PlaylistFilterStr);
    if(playlist.isEmpty())
        return;
    savePlaylist(playlist);
    settings.setValue(PlaylistLastDirStr, QFileInfo(playlist).absolutePath());
}

void MainWindow::onAddFiles()
{
    QSettings settings;
    auto fileList = QFileDialog::getOpenFileNames(this, tr("Open media files"),
                                                  settings.value(PlaylistLastDirStr).toString(),
                                                  MediaFileFilterStr);
    if(fileList.isEmpty())
        return;

    settings.setValue(PlaylistLastDirStr, QFileInfo(fileList.first()).absolutePath());
    addFiles(fileList);

}

void MainWindow::onRemoveFiles()
{
    auto selModel = ui->fileListView->selectionModel();
    auto selection = selModel->selectedRows();
    qSort(selection);
    for(auto iter = selection.rbegin(); iter != selection.rend(); ++iter) {
        model->removeRow(iter->row());
    }
}

void MainWindow::onStartPlay()
{
    if(player->state() == QMediaPlayer::StoppedState) {
        delay = ui->baseDelaySpinBox->value()*1000;
        auto playlist = new QMediaPlaylist(this);
        auto fileList = model->stringList();
        for(const auto &file : fileList) {
            playlist->addMedia(QMediaContent(QUrl::fromLocalFile(file)));
        }
        playlist->setPlaybackMode(QMediaPlaylist::Random);
        playlist->setCurrentIndex(qrand()*fileList.size()/RAND_MAX);
        player->setPlaylist(playlist);
        player->play();
    }
    else {
        player->stop();
    }
}

void MainWindow::onSpeedUp()
{
    delay = static_cast<int>(floor(delay/1.25));
}

bool MainWindow::loadPlaylist(const QString &filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream fileStream(&file);
    QStringList fileList;
    while(!fileStream.atEnd()) {
        fileList << fileStream.readLine();
    }
    model->setStringList(fileList);

    return true;
}

bool MainWindow::savePlaylist(const QString &filename)
{
    QFile file(filename);
    if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    QTextStream fileStream(&file);
    for(const auto &entry : model->stringList()) {
        fileStream << entry << endl;
    }
    QStringList fileList;
    while(!file.atEnd()) {
        fileList << fileStream.readLine();
    }

    return true;
}

void MainWindow::addFiles(const QStringList &fileList)
{
    auto curIndex = ui->fileListView->currentIndex();
    auto modelList = model->stringList();
    modelList.append(fileList);
    model->setStringList(modelList);
    ui->fileListView->setCurrentIndex(curIndex);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    auto appDataLoc = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
    QDir().mkpath(appDataLoc);
    savePlaylist(appDataLoc + "/" + StoredPlaylistStr);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if(!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }
    for(const auto &url : event->mimeData()->urls()) {
        if(!url.isLocalFile()) {
            event->ignore();
            return;
        }
        QFileInfo fileInfo(url.toLocalFile());
        if(fileInfo.suffix() != "mp3" &&
                fileInfo.suffix() != "aac") {
            event->ignore();
            return;
        }
    }
    event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    if(!event->mimeData()->hasUrls()) {
        event->ignore();
        return;
    }
    QStringList fileList;
    for(const auto &url : event->mimeData()->urls()) {
        if(!url.isLocalFile()) {
            event->ignore();
            return;
        }
        QFileInfo fileInfo(url.toLocalFile());
        if(fileInfo.suffix() != "mp3" &&
                fileInfo.suffix() != "aac") {
            event->ignore();
            return;
        }
        fileList << fileInfo.absoluteFilePath();
    }
    addFiles(fileList);
    event->acceptProposedAction();
}

