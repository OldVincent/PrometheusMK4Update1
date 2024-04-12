#pragma once

#include <memory>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/cudafilters.hpp>
#include <list>

namespace RoboPioneers::Prometheus::Core
{
	/**
	 * @brief 蒙版过滤器
	 * @author Vincent
	 * @details
	 *  ~ 该过滤器用于从原始HSV输入图像上过滤出敌对颜色的区域蒙版，并进行预处理以增强。
	 */
	class ColorFilter
	{
	public:
		/// 输入的原始图像
		cv::cuda::GpuMat* BGRPicture{};
		/// 输出的蒙版图像
		cv::Mat BinaryPicture;
		/// 工作流
		cv::cuda::Stream Stream;

	protected:
		cv::Ptr<cv::cuda::Filter> CloseFilter;
		cv::Ptr<cv::cuda::Filter> GaussFilter;
	public:
		/// 最小色调
		int MinHue {};
		/// 最大色调
		int MaxHue {};
		/// 最小饱和度
		int MinSaturation {};
		/// 最大饱和度
		int MaxSaturation {};
		/// 最小亮度
		int MinValue {};
		/// 最大亮度
		int MaxValue {};

	public:
		ColorFilter()
		{
			CloseFilter =
					cv::cuda::createMorphologyFilter(
							cv::MORPH_CLOSE,CV_8UC1,
							cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3,3)));
			GaussFilter = cv::cuda::createGaussianFilter(CV_8UC3, CV_8UC3,
												cv::Size(3,3), 0.8);
		}

		/// 执行
		void Execute();
	};
}