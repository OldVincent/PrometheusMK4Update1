#pragma once

#include <list>
#include <tuple>
#include <tbb/tbb.h>
#include <opencv4/opencv2/opencv.hpp>

namespace RoboPioneers::Prometheus::Core
{
	/**
	 * @brief 装甲板选择器
	 * @author Vincent
	 * @details
	 *  ~ 该类用于从众多候选装甲板中选择一个推荐，目前不含数字识别，因而只推荐面积较大的。
	 *  ~ 该类同时承担锁定区域选定的职责。
	 *  ~ 输出的装甲板坐标是加上兴趣区坐标偏移的坐标。
	 */
	class ArmorSelector
	{
	public:
		/// 装甲板列表
		tbb::concurrent_vector<std::tuple<cv::RotatedRect, cv::RotatedRect>>* Armors;
		/// 位置偏移量
		cv::Point2i* PositionOffset;

		/// 是否找到目标
		bool Found {false};

		/// 装甲板中心点横坐标
		int X {};
		/// 装甲板中心点纵坐标
		int Y {};
		/**
		 * @brief 根据高度和比例系数估算的距离
		 * @details
		 *  ~ 单位为厘米，但精度是米级。
		 *  ~ 估算函数是个一元二次函数，所有系数都是经验值。
		 */
		int Distance {};

		/// 兴趣区域
		cv::Rect InterestedArea;

	public:
		/// 屏幕宽度
		int ScreenWidth {1280};
		/// 屏幕高度
		int ScreenHeight {1024};

		/// 追踪框最小宽度
		int LockingBoxMinWidth {240};
		/// 追踪框最小高度
		int LockingBoxMinHeight {120};

		/// 找到目标后，追踪框的宽度扩张比例
		double WidthExpandRatio {1.0f};
		/// 找到目标后，追踪框的高度扩张比例
		double HeightExpandRatio {1.0f};

		/// 距离高度二次系数 d=Ae^(-Bh)+C
		double DistanceHeightAFactor {1008.28f};
		/// 距离高度一次系数 d=Ae^(-Bh)+C
		double DistanceHeightBFactor {0.08f};
		/// 距离高度常数项 d=Ae^(-Bh)+C
		double DistanceHeightConstantItem {74.43f};

	protected:
		/**
		 * @brief 计算装甲板的得分
		 * @param position_offset 当前处理的画面相对于全屏的位置偏移量
		 * @param armor 装甲板灯条元组
		 * @return 该装甲板得分
		 */
		[[nodiscard]] double GetArmorScore(const std::tuple<cv::RotatedRect, cv::RotatedRect>& armor) const;
		/**
		 * @brief 获取粗略估计的距离
		 * @param height 灯条高度
		 * @return
		 *  ~ 距离，为指数函数d=A*exp(-B*h)+C计算而来。
		 */
		[[nodiscard]] int GetEstimatedDistance(int height) const;

	public:
		/**
		 * @brief 执行
		 * @details
		 *  ~ 若找到了目标，则更新目标的坐标等信息，并更新兴趣区，若未找到，则只更新Found为false，不修改其他信息。
		 */
		void Execute();
	};
}