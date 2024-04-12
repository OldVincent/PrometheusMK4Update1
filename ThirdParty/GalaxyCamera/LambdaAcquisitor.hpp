#pragma once

#include "AbstractAcquisitor.hpp"

#include <functional>

namespace RoboPioneers::Cameras::Galaxy
{
	/**
	 * @brief Lambda采集器
	 * @author Vincent
	 * @details
	 *  ~ 该类允许以Lambda表达式的方式处理相机事件。
	 */
	class LambdaAcquisitor : public AbstractAcquisitor
	{
	public:
		/// 设备离线事件
		std::function<void()> OfflineEvent;
		/// 采集图像事件
		std::function<void(RawPicture)> CaptureEvent;

	protected:
		/// 离线事件，若离线函数器非空则将调用
		void OnDeviceOffline() override
		{
			if (OfflineEvent)
			{
				OfflineEvent();
			}
		}
		/// 采集事件，若采集函数器非空则将调用
		void OnReceivePicture(RawPicture picture) override
		{
			if (CaptureEvent)
			{
				CaptureEvent(picture);
			}
		}
	};
}