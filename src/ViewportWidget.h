#pragma once
#include <QImage>
#include <QPainter>
#include <QWidget>


class ViewportWidget:public QWidget
{
	Q_OBJECT
public:
	explicit ViewportWidget(QWidget* parent = nullptr) :QWidget(parent) {}
	void updateImage(const QImage& image) 
	{
		this->image = image;
		update();
	}
protected:
	void paintEvent(QPaintEvent* event) override 
	{
		QPainter painter(this);
		painter.drawImage(rect(), image);
	}
private:
	QImage image;
};