#pragma once

#include <Core/PrometheusCore.hpp>
#include <GalaxyCamera/GalaxyCamera.hpp>
#include <SerialPort/SerialPort.hpp>

#include "./Stages/CuttingChooser.hpp"
#include "./Stages/FPSCounter.hpp"

namespace RoboPioneers::Prometheus
{
	/**
	 * @brief 控制器类
	 * @author Vincent
	 * @details
	 *  ~ 控制器类控制程序中活跃的帧的数量。
	 */
	class Controller
	{
	protected:
		//==============================
		// 设备
		//==============================

		/// 相机设备
		Cameras::Galaxy::CameraDevice Camera;
		/// 串口通信连接
		SerialPort::Port SerialConnection;

		//==============================
		// 处理阶段
		//==============================

		/// 裁剪阶段
		CuttingChooser CuttingStage;
		/// 颜色过滤阶段
		Core::ColorFilter ColorStage;
		/// 灯条检测阶段
		Core::LightBarDetector LightBarStage;
		/// 装甲板匹配阶段
		Core::ArmorMatcher ArmorStage;
		/// 推荐阶段
		Core::ArmorSelector RecommendStage;
		/// 帧率计数器
		FPSCounter FPSStage;

		//==============================
		// 全局属性部分
		//==============================

		cv::Point PositionOffset {0, 0};

		/// 灯条颜色类型
		enum class ColorType
		{
			Red, Blue
		};

		/// 敌对势力颜色
		ColorType EnemyColor {ColorType::Red};

	public:
		/// 构造函数
		Controller();

		/// 加载配置文件
		void OnLoadConfiguration();

		/// 安装方法
		void OnInstall();
		/// 卸载方法
		void OnUninstall();

		/// 执行
		void Launch();
	};
}