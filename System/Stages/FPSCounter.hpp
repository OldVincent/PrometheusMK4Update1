#pragma once

#include <chrono>

namespace RoboPioneers::Prometheus
{
	/**
	 * @brief FPS计数器
	 * @author Vincent
	 * @details
	 *  ~ 该计数器用于统计帧数。
	 */
	class FPSCounter
	{
	public:
		/// 上一次的输出时间
		std::chrono::steady_clock::time_point LastOutputTime {std::chrono::steady_clock::now()};
		/// 距上一次输出已经经过的帧数
		unsigned int Frames {0};
		bool* Found;
		unsigned int FoundCount {0};

	public:
		/// 执行，满1s时将输出帧率
		void Execute();
	};
}
