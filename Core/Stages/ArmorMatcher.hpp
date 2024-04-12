#pragma once

#include <opencv4/opencv2/opencv.hpp>
#include <list>
#include <tuple>
#include <tbb/tbb.h>

namespace RoboPioneers::Prometheus::Core
{
	/**
	 * @brief 装甲板匹配器
	 * @author Vincent
	 * @details
	 *  ~ 该类用于从灯条列表中匹配出装甲板。
	 */
	class ArmorMatcher
	{
	public:
		/// 灯条列表
		tbb::concurrent_vector<cv::RotatedRect>* LightBars;
		/// 装甲板列表
		tbb::concurrent_vector<std::tuple<cv::RotatedRect, cv::RotatedRect>> Armors;

	public:
		/// 最大转角偏差值
		int MaxAngleDifference = 15;
		/// 最大Y坐标-高度比例，单位1%，该项用于显著消除噪点匹配和三灯条问题
		int MaxDeltaYHeightRatio = 30;

		/// 大装甲板的最小高度-距离比例，单位1%，该项用于避免灯条与血条上噪点匹配
		int MinHeightDistanceRatioBigArmor = 0;
		/// 大装甲板的最大高度-距离比例，单位1%，该项用于解决四灯条问题
		int MaxHeightDistanceRatioBigArmor = 59;
		/// 大装甲板的最小宽度-距离比例，单位1%，该项用于避免灯条与血条上噪点匹配
		int MinWidthDistanceRatioBigArmor = 15;
		/// 大装甲板的最小宽度-距离比例，单位1%，该项用于避免灯条与血条上噪点匹配
		int MaxWidthDistanceRatioBigArmor = 25;

		/// 小装甲板的最小高度-距离比例，单位1%
		int MinHeightDistanceRatioSmallArmor = 30;
		/// 小装甲板的最大高度-距离比例，单位1%
		int MaxHeightDistanceRatioSmallArmor = 100;
		/// 小装甲板的最小宽度-距离比例，单位1%
		int MinWidthDistanceRatioSmallArmor = 5;
		/// 小装甲板的最小宽度-距离比例，单位1%
		int MaxWidthDistanceRatioSmallArmor = 20;

		/// 执行
		void Execute();
	};
}
