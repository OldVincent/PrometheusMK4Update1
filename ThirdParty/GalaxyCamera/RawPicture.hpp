#pragma once

namespace RoboPioneers::Cameras::Galaxy
{
	/**
	 * @brief 原始图片数据
	 * @details
	 *  ~ 该结构体用于存储相机采集的原始数据的内存指针及相关信息。
	 */
	class RawPicture
	{
	public:
		/// 默认构造函数
		RawPicture() = default;

		/**
		 * @brief 构造函数
		 * @param data 图片数据指针
		 * @param width 图片的宽度，即横向像素点个数
		 * @param height 图片的高度，即纵向像素点个数
		 */
		RawPicture(void* data, int width ,int height) :
				Data(data), Width(width), Height(height)
		{}

		/// 数据指针，格式为BayerBG
		void* Data {nullptr};
		/// 通道数量
		int Size {0};
		/// 图像宽度
		int Width {0};
		/// 图像高度
		int Height {0};
	};
}
