#include "ArmorMatcher.hpp"

#include "../Modules/GeometryFeatureModule.hpp"

namespace RoboPioneers::Prometheus::Core
{
	/// 执行方法
	void ArmorMatcher::Execute()
	{
		auto& light_bars = *LightBars;

		unsigned int max_result_count = (light_bars.size() * (light_bars.size() - 1) + 1) / 2;

		// 候选组合，提前将组合准备就绪，以便后续进行多线程判断
		tbb::concurrent_vector<std::tuple<cv::RotatedRect, cv::RotatedRect>> candidate_pairs;
		candidate_pairs.reserve(max_result_count);

		// 清空结果
		Armors.clear();
		Armors.reserve(max_result_count);

		// 串行地制作候选列表
		for (auto first_index = light_bars.begin(); first_index != light_bars.end(); ++first_index)
		{
			auto second_index = first_index;
			++second_index;

			auto first_rectangle = *first_index;

			for (; second_index != light_bars.end(); ++second_index)
			{
				auto second_rectangle = *second_index;

				candidate_pairs.emplace_back(first_rectangle, second_rectangle);
			}
		}

		// 并行地进行判断
		tbb::parallel_for_each(candidate_pairs, [this, armors = &this->Armors](
				const std::tuple<cv::RotatedRect, cv::RotatedRect>& candidate){

			auto first_rectangle = std::get<0>(candidate);
			auto second_rectangle = std::get<1>(candidate);

			auto first_geometry_feature = Modules::GeometryFeatureModule::StandardizeRotatedRectangle(first_rectangle);
			auto second_geometry_feature = Modules::GeometryFeatureModule::StandardizeRotatedRectangle(second_rectangle);

			//------------------------------
			// 角度之差
			//------------------------------

			auto angle_difference = first_geometry_feature.Angle - second_geometry_feature.Angle;
			if (angle_difference < 0) angle_difference = -angle_difference;

			if (angle_difference > MaxAngleDifference) return;

			//------------------------------
			// Y坐标差值-高度比
			//------------------------------

			auto height = first_geometry_feature.Length > second_geometry_feature.Length ?
			              first_geometry_feature.Length : second_geometry_feature.Length;

			auto delta_y = first_geometry_feature.Center.y - second_geometry_feature.Center.y;
			if (delta_y < 0) delta_y = -delta_y;

			auto delta_y_height_ratio = delta_y / height * 100;
			if (delta_y_height_ratio > MaxDeltaYHeightRatio)
				return;

			//------------------------------
			// 高度-距离比
			//------------------------------

			auto distance = cv::norm(first_geometry_feature.Center - second_geometry_feature.Center);

			auto height_distance_ratio = height / distance * 100;
			auto big_armor_hd_matched = height_distance_ratio <= MaxHeightDistanceRatioBigArmor && height_distance_ratio >= MinHeightDistanceRatioBigArmor;
			auto small_armor_hd_matched = height_distance_ratio <= MaxHeightDistanceRatioSmallArmor && height_distance_ratio >= MinHeightDistanceRatioSmallArmor;

			if (!big_armor_hd_matched && !small_armor_hd_matched) return;

			//------------------------------
			// 宽度-距离比
			//------------------------------

			auto width = first_geometry_feature.Width > second_geometry_feature.Width ?
			             first_geometry_feature.Width : second_geometry_feature.Width;
			auto width_distance_ratio = width / distance * 100;
			auto big_armor_wd_matched = width_distance_ratio < MaxWidthDistanceRatioBigArmor && width_distance_ratio > MinWidthDistanceRatioBigArmor;
			auto small_armor_wd_matched = width_distance_ratio < MaxWidthDistanceRatioSmallArmor && width_distance_ratio > MinWidthDistanceRatioSmallArmor;

			if (!big_armor_wd_matched && !small_armor_wd_matched) return;

			armors->emplace_back(first_rectangle, second_rectangle);
		});
	}
}