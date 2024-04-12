#pragma once

#include <list>
#include <unordered_set>
#include <functional>
#include <atomic>
#include <mutex>
#include <chrono>
#include "RawPicture.hpp"

namespace RoboPioneers::Cameras::Galaxy
{
	/// 图像获取器
	class AbstractAcquisitor;

	/**
	 * @brief 相机设备
	 * @author Vincent
	 * @details
	 *  ~ 该类提供基本的相机控制方法和回调事件控制。
	 */
	class CameraDevice
	{
		/// 相机离线事件处理
		friend void HandleCameraDeviceOfflineEvent(void* parameter);
		/// 图片到达事件处理
		friend void HandlePictureIncomeEvent(void* parameters_package);

	private:
		/// 相机设备索引
		unsigned int CameraIndex;

        std::chrono::system_clock::time_point LastPictureTimeStamp;

		/**
		 * @brief 触发采集器的离线事件
		 * @details 该函数将串行地触发采集器的离线事件。
		 */
		void InvokeAcquisitorsOfflineEvent();

		/**
		 * @brief 触发采集器的采集事件
		 * @details 该函数将串行地触发采集器的离线事件。
		 */
		void InvokeAcquisitorsCaptureEvent(RawPicture picture);

		/// 当前图片索引
		unsigned long long CurrentPictureIndex {0};
		/// 当前图片
		RawPicture CurrentPicture;

	protected:
		/// 设备句柄
		void* DeviceHandle {nullptr};
		/// 设备离线事件句柄
		void* DeviceOfflineHandle {nullptr};

		/// 设备是否已经被打开
		std::atomic_bool Opened {false};
		/// 相机控制互斥量
		std::mutex ControlMutex;

		/**
		 * @brief 采集器集合
		 * @details
		 *  ~ 当设备离线、采集到图片等事件发生后，集合中的采集器的相应方法将被依次触发。
		 */
		std::unordered_set<AbstractAcquisitor*> Acquisitors;

	public:
		//==============================
		// 构造与析构函数部分
		//==============================

		/**
		 * @brief 构造并绑定设备索引
		 * @param device_index 设备索引
		 */
		explicit CameraDevice(unsigned int device_index);
		/// 析构函数，若设备未关闭则将关闭设备
		virtual ~CameraDevice();

		//==============================
		// 相机基本控制部分
		//==============================

		/**
		 * @brief 打开相机
		 */
		virtual void Open();

		/**
		 * @brief 重新指定目标相机的索引并打开相机
		 * @param index 目标相机索引
		 */
		virtual void Open(unsigned int index)
		{
			CameraIndex = index;
			Open();
		}

		/**
		 * @brief 关闭相机
		 * @details
		 *  ~ 不会触发相机离线事件。
		 */
		virtual void Close();

		/**
		 * @brief 获取设备句柄
		 * @return 用于与GalaxySDK通信的设备句柄
		 */
		[[nodiscard]] void* GetDeviceHandle() const noexcept
		{
			return DeviceHandle;
		}

		/**
		 * @brief 判断是否是否已经开启
		 * @retval true 当相机已经开启
		 * @retval false 当相机未被开启
		 */
		[[nodiscard]] bool IsOpened() const noexcept
		{
			return Opened;
		}

		//==============================
		// 原始图片控制部分
		//==============================

		/**
		 * @brief 获取当前图片索引
		 * @return 图片索引计数
		 */
		[[nodiscard]] inline auto GetCurrentPictureIndex() const -> const unsigned long long&
		{
			return CurrentPictureIndex;
		}

		/**
		 * @brief 获取当前图片
		 * @return 图片信息
		 * @details
		 *  ~ 注意，大恒提供的相机驱动中，图像的内存存储采用的是交换链的形式。
		 *    也就是说，该方法返回的图像地址会在数帧后被重复利用。
		 *    如果图像处理耗时较长，则需要将内存拷贝后再进行处理，以免多个线程同时读写内存导致错误。
		 */
		[[nodiscard]] inline auto GetCurrentPicture() const -> RawPicture
		{
		    auto current_time_stamp = std::chrono::system_clock::now();

		    if (std::chrono::duration_cast<std::chrono::seconds>(current_time_stamp - LastPictureTimeStamp).count()
		        > 1)
            {
		        throw std::runtime_error("Long time no picture income.");
            }

			return CurrentPicture;
		}

		//==============================
		// 采集器控制部分
		//==============================

		/// 注册采集器，此后该采集器将接收到相应事件
		void RegisterAcquisitor(AbstractAcquisitor* acquisitor);

		/// 注销采集器，此后该采集器将接收不到任何事件
		void UnregisterAcquisitor(AbstractAcquisitor* acquisitor);

		//==============================
		// 相机参数设置部分
		//==============================

		/**
		 * @brief 设置曝光时间
		 * @param value 曝光时间，单位为微秒(us)
		 * @pre 设备已经打开
		 * @retval true 当相机已经开启
		 * @retval false 当相机未被开启
		 */
		virtual bool SetExposureTime(double value);

		/**
		 * @brief 设置增益
		 * @param value 增益，单位为db
		 * @pre 设备已经打开
		 * @return 是否操作成功，操作成功则返回true，失败则返回false
		 */
		virtual bool SetGain(double value);

		/**
		 * @brief 设置白平衡通道红色值
		 * @param value 白平衡值
		 * @retval true 当操作成功
		 * @retval false 当相机未开启或操作失败
		 */
		virtual bool SetWhiteBalanceRedChannel(double value);

		/**
		 * @brief 设置白平衡通道绿色值
		 * @param value 白平衡值
		 * @pre 设备已经打开
		 * @retval true 当相机已经开启
		 * @retval false 当相机未开启或操作失败
		 */
		virtual bool SetWhiteBalanceGreenChannel(double value);

		/**
		 * @brief 设置白平衡通道蓝色值
		 * @param value 白平衡值
		 * @pre 设备已经打开
		 * @retval true 当相机已经开启
		 * @retval false 当相机未开启或操作失败
		 */
		virtual bool SetWhiteBalanceBlueChannel(double value);
	};
}