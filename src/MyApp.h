#pragma once
#include <QImage>
#include "QObject.h"
#include "Renderer.h"

class MainWindow;
class QTimer;

class MyApp:public QObject
{
	Q_OBJECT
public:
	bool init();
public slots:
	void onIdleTick();
	void onQuit();
private:
	bool initWindow();
	void renderFrame();
	void updateRenderStats();

private:
	Renderer renderer;
	MainWindow* mainWnd;
	QTimer* idleTimer;
	QTimer* fpsTimer;
	QImage frameImage;
	int frameIdxAtLastFPSCalc = 0;
};