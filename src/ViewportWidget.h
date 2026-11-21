#pragma once
#include <Windows.h>
#include <QWidget>


class ViewportWidget:public QWidget
{
	Q_OBJECT
public:
	explicit ViewportWidget(QWidget* parent = nullptr) :QWidget(parent) {}

	HWND getNativeWindowHandle() 
	{
		return reinterpret_cast<HWND>(winId());
	}
};