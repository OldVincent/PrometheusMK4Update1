#include "Controller.hpp"

#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/cudaimgproc.hpp>

#include <pthread.h>
#include <iostream>
#include <thread>
#include <boost/filesystem.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <SerialPortUtilities/CRCTool.hpp>
#include <SerialPortUtilities/BytesAccessor.hpp>

namespace RoboPioneers::Prometheus
{
	/// 设置当前线程的线程亲和度
	void SetCurrentThreadCPUAffinity(const std::vector<unsigned int>& cpus)
	{
		cpu_set_t mask;

		CPU_ZERO(&mask);

		for (const auto index : cpus)
		{
			CPU_SET(index, &mask);
		}
		sched_setaffinity(0, sizeof(mask), &mask);
	}

	/// 执行方法
	void Controller::Launch()
	{
		#ifndef DEBUG
			SerialConnection.Open();
		#endif

		while (true)
		{
			std::vector<unsigned char> team_data;
			while(true)
			{
				team_data = SerialConnection.ReadBytes(-1);
				std::cout << "Color Code Receive: " << static_cast<unsigned int>(team_data[0]) << std::endl;
				if (static_cast<unsigned int>(team_data[0]) != 0)
				{
					break;
				}
			}

			if (team_data[0] <= 9)
			{
				EnemyColor = ColorType::Blue;
				std::cout << "Enemy Color: Blue" << std::endl;

				std::vector<unsigned char> response;
				response.push_back(0xFF);
				response.push_back(1);
				response.push_back(0xFF);
				SerialConnection.Write(response);

				break;
			}
			else
			{
				EnemyColor = ColorType::Red;
				std::cout << "Enemy Color: Red" << std::endl;

				std::vector<unsigned char> response;
				response.push_back(0xFF);
				response.push_back(2);
				response.push_back(0xFF);
				SerialConnection.Write(response);

				break;
			}
		}

		OnInstall();

	    std::this_thread::sleep_for(std::chrono::seconds(3));

		Cameras::Galaxy::RawPicture raw_picture;

		try
        {
		    while (true)
            {
                #ifdef DEBUG
                // 等待按下ASCII为27的键（ESC键），按下则终止程序
                if (cv::waitKey(1) == 27)
                {
                    break;
                }
                #endif

                /* 此处OpenCV可能会抛出异常，故加上该try块，以跳过因通信故障导致异常的帧
                 */

                raw_picture = Camera.GetCurrentPicture();

                cv::Mat original_picture(cv::Size(raw_picture.Width, raw_picture.Height), CV_8UC1, raw_picture.Data);
                cv::cvtColor(original_picture, original_picture, cv::COLOR_BayerBG2BGR);
                CuttingStage.OriginalPicture = &original_picture;
                CuttingStage.Execute();
                #ifdef DEBUG
                cv::imshow("Cutting Result", CuttingStage.CuttingPicture);
                #endif
                cv::cuda::GpuMat gpu_original_picture;
                gpu_original_picture.upload(CuttingStage.CuttingPicture, ColorStage.Stream);
                ColorStage.BGRPicture = &gpu_original_picture;
                ColorStage.Execute();
                #ifdef DEBUG
                cv::imshow("Binary", ColorStage.BinaryPicture);
                #endif
                LightBarStage.Execute();
                ArmorStage.Execute();
                RecommendStage.Execute();
                #ifdef DEBUG
                if (RecommendStage.Found)
                {
                    // 输出坐标
                    std::cout << "Found X:" << RecommendStage.X << " Y:" << RecommendStage.Y << " Distance:"
                              << RecommendStage.Distance << "cm" << std::endl;
                }
                #endif
                // 准备用于传输的字节包
                std::vector<unsigned char> bytes;
                bytes.resize(9);
                SerialPort::Utilities::BytesAccessor accessor(bytes.data(), 9);
                accessor.Access<unsigned char>(0) = 0xFF;
                if (RecommendStage.Found)
                {
                    accessor.Access<unsigned char>(1) = 1;
                } else
                {
                    accessor.Access<unsigned char>(1) = 0;
                }
                accessor.Access<unsigned short>(2) = static_cast<unsigned short>(RecommendStage.X);
                accessor.Access<unsigned short>(4) = static_cast<unsigned short>(RecommendStage.Y);
                accessor.Access<unsigned short>(6) = static_cast<unsigned short>(RecommendStage.Distance);
                accessor.Access<unsigned char>(8) =
                        SerialPort::Utilities::CRCTool::GetCRC8CheckSum(bytes.data(), 8);
                #ifndef DEBUG
                // 传输字节包
                SerialConnection.Write(bytes);
                #endif
                FPSStage.Execute();
            }
        }
        catch (cv::Exception& error)
        {
            std::cout << "Exception Occurs: " << error.what() << std::endl;
        }
		OnUninstall();
	}

	/// 安装方法
	void Controller::OnInstall()
	{
		OnLoadConfiguration();
		/*
		 * 允许当前线程及子线程覆盖全部的CPU核心
		 * 对于JetPack4.4而言，两个Denver大核因为执行延迟问题被单独隔离，需要手动指定在其上工作的线程
		 */
		SetCurrentThreadCPUAffinity({0,1,2,3,4,5});

		while(!Camera.IsOpened())
		{
			try {
				Camera.Open();
			}catch(std::exception& error)
			{
				std::cout << "[Error] Failed to Open Camera: '" << error.what()
				          << "', will Attempt in 10 Seconds." << std::endl;
				std::this_thread::sleep_for(std::chrono::seconds(10));
			}
		}

		CuttingStage.InterestedArea = &RecommendStage.InterestedArea;
		CuttingStage.Found = &RecommendStage.Found;
		LightBarStage.BinaryPicture = &ColorStage.BinaryPicture;
		ArmorStage.LightBars = &LightBarStage.LightBars;
		RecommendStage.Armors = &ArmorStage.Armors;
		RecommendStage.PositionOffset = &CuttingStage.PositionOffset;
		FPSStage.Found = &RecommendStage.Found;
	}

	/// 卸载方法
	void Controller::OnUninstall()
	{
		Camera.Close();

		#ifndef DEBUG
		SerialConnection.Close();
		#endif
	}

	/// 构造函数
	Controller::Controller() : Camera(0), SerialConnection("/dev/ttyTHS2")
	{}

	/// 加载配置文件
	void Controller::OnLoadConfiguration()
	{
		// 确保日志路径存在
		if(boost::filesystem::exists("Settings.json"))
		{
			boost::property_tree::ptree json_node;
			boost::property_tree::read_json("Settings.json", json_node);

			if (EnemyColor == ColorType::Red)
			{
				ColorStage.MinHue = json_node.get<int>("Mask.Red.Hue.Min");
				ColorStage.MaxHue = json_node.get<int>("Mask.Red.Hue.Max");
				ColorStage.MinSaturation = json_node.get<int>("Mask.Red.Saturation.Min");
				ColorStage.MaxSaturation = json_node.get<int>("Mask.Red.Saturation.Max");
				ColorStage.MinValue = json_node.get<int>("Mask.Red.Value.Min");
				ColorStage.MaxValue = json_node.get<int>("Mask.Red.Value.Max");
			}
			else
			{
				ColorStage.MinHue = json_node.get<int>("Mask.Blue.Hue.Min");
				ColorStage.MaxHue = json_node.get<int>("Mask.Blue.Hue.Max");
				ColorStage.MinSaturation = json_node.get<int>("Mask.Blue.Saturation.Min");
				ColorStage.MaxSaturation = json_node.get<int>("Mask.Blue.Saturation.Max");
				ColorStage.MinValue = json_node.get<int>("Mask.Blue.Value.Min");
				ColorStage.MaxValue = json_node.get<int>("Mask.Blue.Value.Max");
			}


			LightBarStage.MinArea = json_node.get<int>("LightBar.MinArea");
			LightBarStage.MinFillingRatio = json_node.get<int>("LightBar.MinFillingRatio");

			ArmorStage.MaxAngleDifference = json_node.get<int>("LightBar.MaxAngleDifference");
			ArmorStage.MaxDeltaYHeightRatio = json_node.get<int>("LightBar.DeltaYHeightRatio.Max");

			ArmorStage.MinHeightDistanceRatioBigArmor = json_node.get<int>("BigArmor.HeightDistanceRatio.Min");
			ArmorStage.MaxHeightDistanceRatioBigArmor = json_node.get<int>("BigArmor.HeightDistanceRatio.Max");
			ArmorStage.MinWidthDistanceRatioBigArmor = json_node.get<int>("BigArmor.WidthDistanceRatio.Min");
			ArmorStage.MaxWidthDistanceRatioBigArmor = json_node.get<int>("BigArmor.WidthDistanceRatio.Max");

			ArmorStage.MinHeightDistanceRatioSmallArmor = json_node.get<int>("SmallArmor.HeightDistanceRatio.Min");
			ArmorStage.MaxHeightDistanceRatioSmallArmor = json_node.get<int>("SmallArmor.HeightDistanceRatio.Max");
			ArmorStage.MinWidthDistanceRatioSmallArmor = json_node.get<int>("SmallArmor.WidthDistanceRatio.Min");
			ArmorStage.MaxWidthDistanceRatioSmallArmor = json_node.get<int>("SmallArmor.WidthDistanceRatio.Max");

			std::clog << "[Message] Using Settings in Settings.json." << std::endl;
		}
	}
}
