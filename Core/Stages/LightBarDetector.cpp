#include "LightBarDetector.hpp"

#include "../Modules/GeometryFeatureModule.hpp"
#include <vector>

namespace RoboPioneers::Prometheus::Core
{
	/// 执行方法
	void LightBarDetector::Execute()
	{
		// 查找轮廓
		std::vector<std::vector<cv::Point>> contours;
		cv::findContours(*BinaryPicture, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);

		LightBars.clear();
		LightBars.reserve(contours.size());

		tbb::parallel_for_each(contours, [light_bars = &this->LightBars,
									min_area = &this->MinArea,
									min_filling_ratio = &this->MinFillingRatio]
									(const std::vector<cv::Point>& contour){
			auto area = cv::contourArea(contour);

			if (area < *min_area) return;
			auto rotated_rectangle = cv::minAreaRect(contour);
			if (rotated_rectangle.size.area() <= 0) return;

			if (area / rotated_rectangle.size.area() * 100 < *min_filling_ratio) return;

			auto geometry_feature = Modules::GeometryFeatureModule::StandardizeRotatedRectangle(rotated_rectangle);
			if (geometry_feature.Angle < 20 || geometry_feature.Angle > 160) return;

			light_bars->push_back(rotated_rectangle);
		});
	}
}