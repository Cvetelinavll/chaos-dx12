#include <QApplication>
#include "MyApp.h"

int main(int argc, char* argv[])
{
    QApplication qtApp(argc, argv);
    MyApp app;

	if (false==app.init())
	{
		return 1;
	}
	return qtApp.exec();
}