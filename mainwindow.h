#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <sqlite3.h>

#include <string>

#include <boost/shared_ptr.hpp>

namespace Ui {
class MainWindow;
}

struct QtImage;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void appLoadDB();
    void appCloseDB();
    void appRenderWave(int);

private slots:
    void onTimer();

private:
    Ui::MainWindow *ui;

    int last_fid;

    QtImage *canvas;

    QTimer *timer;

    boost::shared_ptr<sqlite3> db;

    boost::shared_ptr<sqlite3_stmt> prepare_statement(std::string const& query);
};

#endif // MAINWINDOW_H
