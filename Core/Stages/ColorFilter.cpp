#include "ColorFilter.hpp"
#include <opencv4/opencv2/cudaarithm.hpp>
#include <opencv4/opencv2/cudaimgproc.hpp>
namespace RoboPioneers::Prometheus::Core
{
	/// 执行方法
	void ColorFilter::Execute()
	{
		GaussFilter->apply(*BGRPicture, *BGRPicture, Stream);

		cv::cuda::GpuMat hsv_picture(BGRPicture->size(), CV_8UC3, cv::Scalar(0,0,0));
		cv::cuda::cvtColor(*BGRPicture, hsv_picture, cv::COLOR_BGR2HSV, 0, Stream);

		cv::cuda::GpuMat channels[3], hue_masks[2], saturation_masks[2], value_masks[2],
			mask(BGRPicture->size(),CV_8UC1, cv::Scalar(0,0,0));

		cv::cuda::split(hsv_picture, channels, Stream);

		cv::cuda::threshold(channels[0], hue_masks[0], MinHue, 255, cv::THRESH_BINARY, Stream);
		cv::cuda::threshold(channels[0], hue_masks[1], MaxHue, 255, cv::THRESH_BINARY_INV, Stream);
		cv::cuda::threshold(channels[1], saturation_masks[0], MinSaturation, 255, cv::THRESH_BINARY, Stream);
		cv::cuda::threshold(channels[1], saturation_masks[1], MaxSaturation, 255, cv::THRESH_BINARY_INV, Stream);
		cv::cuda::threshold(channels[2], value_masks[0], MinValue, 255, cv::THRESH_BINARY, Stream);
		cv::cuda::threshold(channels[2], value_masks[1], MaxValue, 255, cv::THRESH_BINARY_INV, Stream);

		cv::cuda::GpuMat hue_mask, saturation_mask, value_mask;
		cv::cuda::bitwise_and(hue_masks[0], hue_masks[1], hue_mask, cv::noArray(), Stream);
		cv::cuda::bitwise_and(saturation_masks[0], saturation_masks[1], saturation_mask, cv::noArray(), Stream);
		cv::cuda::bitwise_and(value_masks[0], value_masks[1], value_mask, cv::noArray(), Stream);

		cv::cuda::bitwise_and(hue_mask, saturation_mask, mask, value_mask, Stream);

		CloseFilter->apply(mask, mask, Stream);

		mask.download(BinaryPicture, Stream);

		Stream.waitForCompletion();
	}
}