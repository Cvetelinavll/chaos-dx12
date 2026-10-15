#include "Renderer.h"

int main()
{
	Renderer renderer;
	std::wcout<<std::boolalpha<<renderer.render() ? 0 : 1;
	return 0;
}

