#include "MainWindow.h"
#include "MyApp.h"
#include <QCloseEvent>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>

MainWindow::MainWindow(MyApp* app,  int viewportWidth, int viewportHeight, QWidget* parent)
    : QMainWindow(parent), app(app)
{
    setWindowTitle("DX12Renderer");

    QWidget* centralWidget = new QWidget(this);
    mainLayout = new QGridLayout(centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    viewport = new ViewportWidget(centralWidget);
    viewport->setFixedSize(viewportWidth, viewportHeight);

    statusBar = new QWidget(centralWidget);
    QHBoxLayout* statusLayout = new QHBoxLayout(statusBar);
    statusLayout->setContentsMargins(4, 2, 4, 2);
    statusLayout->addStretch();
    statusLayout->addWidget(new QLabel("FPS:", statusBar));
    statusFPS = new QLabel("0", statusBar);
    statusLayout->addWidget(statusFPS);

    mainLayout->addWidget(viewport, 0, 0);
    mainLayout->addWidget(statusBar, 1, 0);

    setCentralWidget(centralWidget);
}

void MainWindow::setFPS(const int fps)
{
    statusFPS->setText(QString::number(fps));
}

void MainWindow::updateViewport(const QImage & image)
{
    viewport->updateImage(image);
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    app->onQuit();
    QMainWindow::closeEvent(event);
}

