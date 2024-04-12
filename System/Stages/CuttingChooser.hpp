#pragma once

#include <opencv4/opencv2/opencv.hpp>

namespace RoboPioneers::Prometheus
{
	/**
	 * @brief 裁剪选择器
	 * @author Vincent
	 * @details
	 *  ~ 该阶段用于根据上一帧的检测结果确定本帧的裁剪范围。
	 */
	class CuttingChooser
	{
	public:
		//==============================
		// 输入部分
		//==============================

		/// 输入的源图像
		cv::Mat* OriginalPicture;

		/// 输入的上一帧兴趣区
		cv::Rect* InterestedArea;
		/// 输入的是否找到目标
		bool* Found;

		/// 输出的裁剪图像
		cv::Mat CuttingPicture;
		/// 输出的当前图片偏移坐标
		cv::Point PositionOffset;

	public:
		//==============================
		// 状态部分
		//==============================

		/**
		 * @brief 锁定剩余机会
		 * @details
		 *  ~ 非零表示处于锁定状态。
		 *  ~ 正数表示剩余锁定次数，归零时将进入全局检索状态。
		 */
		unsigned int LockingRemainTimes {0};

		/**
		 * @brief 准许锁定还需要的推荐次数
		 */
		unsigned int ApprovalRequiredTimes {2};

		/**
		 * @brief 上一次兴趣区域
		 * @details
		 *  ~ 该处存放上一次的目标检测到的区域。
		 */
		cv::Rect LastInterestedArea;

	public:
		//==============================
		// 设定部分
		//==============================

		/**
		 * @brief 锁定起始机会
		 * @details
		 *  ~ 当目标被检测到并允许进入锁定状态时，锁定机会将被赋予该值。
		 */
		unsigned int LockingStartupTimes {5};

		/**
		 * @brief 锁定准入门槛
		 * @details
		 *  ~ 连续至少如是次数被选择为最优目标，才允许该目标被锁定。
		 */
		unsigned int LockingApprovalThreshold {2};

		/**
		 * @brief 用于判断是否为同一区域的相交面积比
		 */
		double MinIntersectionAreaRatio {0.6};

	public:
		/// 执行方法
		void Execute();
	};
}
