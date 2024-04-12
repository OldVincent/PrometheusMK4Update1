#include "FPSCounter.hpp"

#include <iostream>

namespace RoboPioneers::Prometheus
{
	// 执行方法
	void FPSCounter::Execute()
	{
		auto current_time = std::chrono::steady_clock::now();
		++Frames;

		if (*Found)
		{
			++FoundCount;
		}

		if (std::chrono::duration_cast<std::chrono::milliseconds>(current_time - LastOutputTime).count() > 1000)
		{
			std::cout << "FPS: " << Frames << std::endl;
			LastOutputTime = current_time;
			std::cout << "Ratio: " << static_cast<double>(FoundCount) / static_cast<double>(Frames) * 100.0f << "%" << std::endl;
			FoundCount = 0;

			Frames = 0;
		}
	}
}
