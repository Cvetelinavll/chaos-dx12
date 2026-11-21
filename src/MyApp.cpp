#include "MyApp.h"
#include "MainWindow.h"
#include <QTimer>

bool MyApp::init()
{
	if (false==initWindow())
	{
		return false;
	}
	mainWnd->show();

	renderer.prepareForRendering(mainWnd->getViewportHandle());

	idleTimer = new QTimer(mainWnd);
	connect(idleTimer, &QTimer::timeout, this, &MyApp::onIdleTick);
	idleTimer->start(0);

	fpsTimer = new QTimer(mainWnd);
	connect(fpsTimer,&QTimer::timeout, this, &MyApp::updateRenderStats);
	fpsTimer->start(1000);

	return true;
}

bool MyApp::initWindow()
{
	mainWnd = new MainWindow(this, renderer.getWidth(), renderer.getHeight());
	return true;
}

void MyApp::onQuit()
{
	idleTimer->stop();
	fpsTimer->stop();
	renderer.stopRendering();
}

void MyApp::onIdleTick()
{
	renderFrame();
}

void MyApp::renderFrame()
{
	renderer.renderFrameWithSwapChain();
}

void MyApp::updateRenderStats()
{
	const UINT64 frameIdx = renderer.getFrameIdx();
	mainWnd->setFPS(static_cast<int>(frameIdx - frameIdxAtLastFPSCalc));
	frameIdxAtLastFPSCalc = frameIdx;
}