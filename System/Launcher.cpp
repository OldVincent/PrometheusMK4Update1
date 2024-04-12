#include "Controller.hpp"
#include <cstdlib>

int main(int argc, char** argv)
{
	using namespace RoboPioneers::Prometheus;

	Controller controller;
	controller.Launch();

	return 0;
}