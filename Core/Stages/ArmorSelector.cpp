#include "ArmorSelector.hpp"

#include "../Modules/GeometryFeatureModule.hpp"

#include <tbb/tbb.h>
#include <cmath>

#ifdef DEBUG
#include <iostream>
#endif

namespace RoboPioneers::Prometheus::Core
{
	using RotatedRectPair = std::tuple<cv::RotatedRect, cv::RotatedRect>;

	/// 计算装甲板得分
	double ArmorSelector::GetArmorScore(const std::tuple<cv::RotatedRect, cv::RotatedRect>& armor) const
	{
		auto first_light = Modules::GeometryFeatureModule::StandardizeRotatedRectangle(std::get<0>(armor));
		auto second_light = Modules::GeometryFeatureModule::StandardizeRotatedRectangle(std::get<1>(armor));

		auto length = cv::norm(first_light.Center - second_light.Center);
		auto width = first_light.Length > second_light.Length ? first_light.Length : second_light.Length;

		auto center_point = (first_light.Center + second_light.Center) / 2;
		auto real_center_point = center_point + *PositionOffset;
		auto real_offset = cv::norm(real_center_point - (cv::Point(ScreenWidth, ScreenHeight) / 2));

		auto score = static_cast<long>(length * width / (real_offset * real_offset));

		return score;
	}

	/// 粗略估计距离
	int ArmorSelector::GetEstimatedDistance(int height) const
	{
		return static_cast<int>(DistanceHeightAFactor * std::exp(-DistanceHeightBFactor * height)
			+ DistanceHeightConstantItem);
	}

	void ArmorSelector::Execute()
	{
		if (Armors->empty())
		{
			// 直接设置未找到，不更新兴趣区，由外部程序自行决定
			Found = false;

			return;
		}
		else
		{
			using scored_pair = std::tuple<double, RotatedRectPair>;

			// 更坏比较器
			struct worse_comparer
			{
				// 该操作符返回true，则a会排到b的后方
				bool operator () (const scored_pair& a, const scored_pair& b)
				{
					return std::get<0>(a) < std::get<0>(b);
				}
			};

			// 计算过分数的装甲板队列
			tbb::concurrent_priority_queue<scored_pair, worse_comparer> scored_armors;

			// 并行地计算分数并放入优先级队列
			tbb::parallel_for_each(*Armors, [this, &scored_armors](const RotatedRectPair& armor_candidate){
				scored_armors.push({this->GetArmorScore(armor_candidate), armor_candidate});
			});

			//==============================
			// 解包获取最优项
			//==============================

			scored_pair best_scored_pair;
			while(!scored_armors.try_pop(best_scored_pair));
			RotatedRectPair best_pair = std::get<1>(best_scored_pair);

			auto& first_light = std::get<0>(best_pair);
			auto& second_light = std::get<1>(best_pair);

			auto center_point = (first_light.center + second_light.center) / 2;

			//==============================
			// 计算兴趣区
			//==============================

			std::vector<cv::Point2f> armor_vertices;
			armor_vertices.resize(8);
			first_light.points(&armor_vertices[0]);
			second_light.points(&armor_vertices[4]);
			auto armor_rectangle = cv::minAreaRect(armor_vertices).boundingRect();

			auto& global_offset = *PositionOffset;

			//==============================
			// 填充输出结果
			//==============================

			// 计算并限制兴趣区长宽
			InterestedArea.width = armor_rectangle.width * (WidthExpandRatio + 1);
			if (InterestedArea.width < LockingBoxMinWidth) InterestedArea.width = LockingBoxMinWidth;
			InterestedArea.height = armor_rectangle.height * (HeightExpandRatio + 1);
			if (InterestedArea.height < LockingBoxMinHeight) InterestedArea.height = LockingBoxMinHeight;

			// 计算并限制兴趣区坐标
			InterestedArea.x = global_offset.x + armor_rectangle.x - 0.5f * (InterestedArea.width - armor_rectangle.width);
			if (InterestedArea.x < 0) InterestedArea.x = 0;
			InterestedArea.y = global_offset.y + armor_rectangle.y - 0.5f * (InterestedArea.height - armor_rectangle.height);
			if (InterestedArea.y < 0) InterestedArea.y = 0;

			// 结合兴趣区坐标限制兴趣区长宽
			if (InterestedArea.x + InterestedArea.width > ScreenWidth)
				InterestedArea.width = ScreenWidth - InterestedArea.x;
			if (InterestedArea.y + InterestedArea.height > ScreenHeight)
				InterestedArea.height = ScreenHeight - InterestedArea.y;

			// 估算距离
			Distance = GetEstimatedDistance(armor_rectangle.height);

			Found = true;

			X = static_cast<int>(center_point.x) + global_offset.x;
			Y = static_cast<int>(center_point.y) + global_offset.y;

			#ifdef DEBUG
			std::cout << "Width:" << armor_rectangle.width << " Height:" << armor_rectangle.height << std::endl;
			#endif
		}
	}
}