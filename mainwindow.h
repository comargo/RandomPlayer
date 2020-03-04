#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QStringListModel>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void connectSignals();

protected slots:
    void onLoadPlaylist();
    void onSavePlaylist();
    void onAddFiles();
    void onRemoveFiles();
    void onStartPlay();
    void onSpeedUp();

protected:
    bool loadPlaylist(const QString &filename);
    bool savePlaylist(const QString &filename);
    void addFiles(const QStringList &fileList);

private:
    Ui::MainWindow *ui;
    QStringListModel *model = nullptr;
    QMediaPlayer *player;
    int delay = 0;

    // QWidget interface
protected:
    void closeEvent(QCloseEvent *event) override;

    // QWidget interface
protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};
#endif // MAINWINDOW_H
