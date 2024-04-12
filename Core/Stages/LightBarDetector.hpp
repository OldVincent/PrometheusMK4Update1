#pragma once

#include <opencv4/opencv2/opencv.hpp>
#include <list>
#include <tbb/tbb.h>

namespace RoboPioneers::Prometheus::Core
{
	/**
	 * @brief 灯条检测器
	 */
	class LightBarDetector
	{
	public:
		/// 输入的二值蒙版图
		cv::Mat* BinaryPicture;
		/// 输出的可能的灯条矩形
		tbb::concurrent_vector<cv::RotatedRect> LightBars;

	public:
		/// 最小面积
		int MinArea {};
		/// 最小填充比
		int MinFillingRatio {};

	public:
		/// 执行方法
		void Execute();
	};
}