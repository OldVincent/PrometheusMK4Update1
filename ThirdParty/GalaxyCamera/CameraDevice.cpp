#include "CameraDevice.hpp"
#include "AbstractAcquisitor.hpp"

#include <sstream>
#include <GxIAPI.h>

namespace RoboPioneers::Cameras::Galaxy
{
	/// 相机管理脚本
	class GalaxyCameraLibManagerScript
	{
	public:
		GalaxyCameraLibManagerScript() noexcept
		{
			GXInitLib();
		}
	}GalaxyCameraLibManagerScriptInstance;

	/// 相机离线事件外置处理函数
	void HandleCameraDeviceOfflineEvent(void *parameter)
	{
		using namespace RoboPioneers::Cameras::Galaxy;

        throw std::runtime_error("CameraDevice Offline");

		auto* target = static_cast<CameraDevice*>(parameter);
		if (target)
		{
			target->InvokeAcquisitorsOfflineEvent();
		}
	}

	/// 图片采集事件外置处理函数
	void HandlePictureIncomeEvent(void* parameters_package)
	{
		using namespace RoboPioneers::Cameras::Galaxy;

		auto* parameters =  static_cast<GX_FRAME_CALLBACK_PARAM*>(parameters_package);
		auto* target = static_cast<CameraDevice*>(parameters->pUserParam);

		if (target)
		{
			RawPicture picture;
			picture.Data = const_cast<void*>(parameters->pImgBuf);
			picture.Size = parameters->nImgSize;
			picture.Width = parameters->nWidth;
			picture.Height = parameters->nHeight;
			target->InvokeAcquisitorsCaptureEvent(picture);
		}

        target->LastPictureTimeStamp = std::chrono::system_clock::now();
	}

	/// 构造并绑定设备索引
	CameraDevice::CameraDevice(unsigned int device_index) : CameraIndex(device_index)
	{}

	/// 析构并确保相机关闭
	CameraDevice::~CameraDevice()
	{
		if (IsOpened())
		{
			std::unique_lock lock(ControlMutex);

			if (DeviceHandle)
			{
				GXUnregisterCaptureCallback(DeviceHandle);
				GXUnregisterDeviceOfflineCallback(DeviceHandle, DeviceOfflineHandle);

				GXCloseDevice(DeviceHandle);
				DeviceHandle = nullptr;
			}
		}
	}

	/// 开启设备并开始采集
	void CameraDevice::Open()
	{
		if (IsOpened())
		{
			Close();
		}

		std::unique_lock lock(ControlMutex);

		// 操作执行结果
		GX_STATUS operation_result;

		// 获取设备列表
		uint32_t device_count = 0;
		operation_result = GXUpdateDeviceList(&device_count, 500);
		if (operation_result != GX_STATUS_LIST::GX_STATUS_SUCCESS)
		{
			throw std::runtime_error("CameraDevice::Open Failed to Query Device List.");
		}
		if (device_count <= 0)
		{
			throw std::runtime_error("CameraDevice::Open No Camera Detected.");
		}
		if (device_count <= CameraIndex)
		{
			std::stringstream message;
			message << "CameraDevice::Open Invalid Device Index: " << CameraIndex
				<< " , Camera Counts: " << device_count;
			throw std::runtime_error(message.str());
		}

		// 开启指定设备，注意，GalaxySDK中开启设备的该方法中设备索引是从1开始编号的
		operation_result = GXOpenDeviceByIndex(CameraIndex + 1, &DeviceHandle);
		if (operation_result != GX_STATUS_LIST::GX_STATUS_SUCCESS)
		{
			throw std::runtime_error("CameraDevice::Open Failed to Open Device.");
		}

		// 注册采集回调
		operation_result = GXRegisterCaptureCallback(DeviceHandle,
		                                             this,
		                                             reinterpret_cast<GXCaptureCallBack>(HandlePictureIncomeEvent));
		if (operation_result != GX_STATUS_LIST::GX_STATUS_SUCCESS)
		{
			throw std::runtime_error("MatAcquisitor::Start Failed to Register Capture Callback.");
		}

		// 注册离线回调
		operation_result = GXRegisterDeviceOfflineCallback(DeviceHandle, this,
		                                                 HandleCameraDeviceOfflineEvent, &DeviceOfflineHandle);
		if (operation_result != GX_STATUS_LIST::GX_STATUS_SUCCESS)
		{
			throw std::runtime_error("CameraDevice::Open Failed to Register Offline Callback.");
		}

		// 发送命令开始采集
		operation_result = GXSendCommand(DeviceHandle,
		                                 GX_COMMAND_ACQUISITION_START);
		if (operation_result != GX_STATUS_LIST::GX_STATUS_SUCCESS)
		{
			throw std::runtime_error("CameraDevice::Open Failed to Start Acquisition.");
		}

		Opened = true;
		LastPictureTimeStamp = std::chrono::system_clock::now();
	}

	/// 停止采集并关闭设备
	void CameraDevice::Close()
	{
		std::unique_lock lock(ControlMutex);

		if (DeviceHandle)
		{
			GXSendCommand(DeviceHandle, GX_COMMAND_ACQUISITION_STOP);

			GXUnregisterCaptureCallback(DeviceHandle);
			GXUnregisterDeviceOfflineCallback(DeviceHandle, DeviceOfflineHandle);

			GXCloseDevice(DeviceHandle);
			DeviceHandle = nullptr;
		}
	}

	/// 设置曝光时间
	bool CameraDevice::SetExposureTime(double value)
	{
		if (DeviceHandle &&
		    GXSetFloat(DeviceHandle, GX_FLOAT_EXPOSURE_TIME, value) == GX_STATUS_LIST::GX_STATUS_SUCCESS)
		{
			return true;
		}
		return false;
	}

	/// 设置增益
	bool CameraDevice::SetGain(double value)
	{
		if (DeviceHandle &&
		    GXSetFloat(DeviceHandle, GX_FLOAT_GAIN, value) == GX_STATUS_LIST::GX_STATUS_SUCCESS)
		{
			return true;
		}
		return false;
	}

	/// 设置白平衡红色通道值
	bool CameraDevice::SetWhiteBalanceRedChannel(double value)
	{
		if (DeviceHandle)
		{
			GX_STATUS operation_result;
			operation_result = GXSetEnum(DeviceHandle, GX_ENUM_BALANCE_RATIO_SELECTOR,
			                             GX_BALANCE_RATIO_SELECTOR_RED);
			if (operation_result != GX_STATUS_LIST::GX_STATUS_SUCCESS)
			{
				return false;
			}
			operation_result = GXSetFloat(DeviceHandle, GX_FLOAT_BALANCE_RATIO, value);
			if (operation_result != GX_STATUS_LIST::GX_STATUS_SUCCESS)
			{
				return false;
			}

			return true;
		}
		return false;
	}

	/// 设置白平衡绿色通道值
	bool CameraDevice::SetWhiteBalanceGreenChannel(double value)
	{
		if (DeviceHandle)
		{
			GX_STATUS operation_result;
			operation_result = GXSetEnum(DeviceHandle, GX_ENUM_BALANCE_RATIO_SELECTOR,
			                             GX_BALANCE_RATIO_SELECTOR_GREEN);
			if (operation_result != GX_STATUS_LIST::GX_STATUS_SUCCESS)
			{
				return false;
			}
			operation_result = GXSetFloat(DeviceHandle, GX_FLOAT_BALANCE_RATIO, value);
			if (operation_result != GX_STATUS_LIST::GX_STATUS_SUCCESS)
			{
				return false;
			}

			return true;
		}
		return false;
	}

	/// 设置白平衡蓝色通道值
	bool CameraDevice::SetWhiteBalanceBlueChannel(double value)
	{
		if (DeviceHandle)
		{
			GX_STATUS operation_result;
			operation_result = GXSetEnum(DeviceHandle, GX_ENUM_BALANCE_RATIO_SELECTOR,
			                             GX_BALANCE_RATIO_SELECTOR_BLUE);
			if (operation_result != GX_STATUS_LIST::GX_STATUS_SUCCESS)
			{
				return false;
			}
			operation_result = GXSetFloat(DeviceHandle, GX_FLOAT_BALANCE_RATIO, value);
			if (operation_result != GX_STATUS_LIST::GX_STATUS_SUCCESS)
			{
				return false;
			}

			return true;
		}
		return false;
	}

	/// 调用所有采集器的设备离线事件
	void CameraDevice::InvokeAcquisitorsOfflineEvent()
	{
		if (!Acquisitors.empty())
		{
			for (auto acquisitor : Acquisitors)
			{
				acquisitor->OnDeviceOffline();
			}
		}
	}

	/// 调用所有采集器的采集事件
	void CameraDevice::InvokeAcquisitorsCaptureEvent(RawPicture picture)
	{
		++CurrentPictureIndex;
		CurrentPicture = picture;
		if (!Acquisitors.empty())
		{
			for (auto acquisitor : Acquisitors)
			{
				acquisitor->OnReceivePicture(picture);
			}
		}
	}

	/// 注册采集器
	void CameraDevice::RegisterAcquisitor(AbstractAcquisitor *acquisitor)
	{
		if (acquisitor)
		{
			Acquisitors.insert(acquisitor);
		}
		else
		{
			throw std::runtime_error("CameraDevice::RegisterAcquisitor Acquisitor Pointer is Null.");
		}
	}

	/// 注销采集器
	void CameraDevice::UnregisterAcquisitor(AbstractAcquisitor *acquisitor)
	{
		if (acquisitor)
		{
			Acquisitors.erase(acquisitor);
		}
		else
		{
			throw std::runtime_error("CameraDevice::UnregisterAcquisitor Acquisitor Pointer is Null.");
		}
	}
}