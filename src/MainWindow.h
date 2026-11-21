#pragma once
#include <QMainWindow>
#include "ViewportWidget.h"

class MyApp;
class QGridLayout;
class QLabel;
class QCloseEvent;

class MainWindow :public QMainWindow
{
	Q_OBJECT
public:
	MainWindow(MyApp* app, int viewportWidth, int viewportHeight, QWidget* parent = nullptr);

	void setFPS(const int fps);

	HWND getViewportHandle() const;

	void closeEvent(QCloseEvent* event)override;

private:
	QGridLayout* mainLayout;
	ViewportWidget* viewport;
	QWidget* statusBar;
	QLabel* statusFPS;
	MyApp* app;
};