#pragma once
#include <QImage>
#include <QPainter>
#include <QWidget>


class ViewportWidget:public QWidget
{
	Q_OBJECT
public:
	explicit ViewportWidget(QWidget* parent = nullptr) :QWidget(parent) {}
	void updateImage(const QImage& newImage) 
	{
		this->image = newImage;
		update();
	}
protected:
	void paintEvent(QPaintEvent*) override 
	{
		QPainter painter(this);
		painter.drawImage(rect(), image);
	}
private:
	QImage image;
};