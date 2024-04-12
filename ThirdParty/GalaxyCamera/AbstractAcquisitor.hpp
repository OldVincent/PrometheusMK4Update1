#pragma once

#include "RawPicture.hpp"

namespace RoboPioneers::Cameras::Galaxy
{
	/**
	 * @brief 图片获取器接口
	 * @author Vincent
	 * @details
	 *  ~ 采集器可以接收图片采集事件。
	 */
	class AbstractAcquisitor
	{
		friend class CameraDevice;

	protected:
		/**
		 * @brief 设备离线事件
		 * @details 当设备意外离线时，该方法将被调用。
		 */
		virtual void OnDeviceOffline() {};

		/**
		 * @brief 接收到图片事件
		 * @param picture 原始图片参数
		 * @details
		 *  ~ 注意，picture仅存储了图片的地址和参数信息，地址上的内存应即用即取。
		 */
		virtual void OnReceivePicture(RawPicture picture) = 0;
	};
}
