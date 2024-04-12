#include "CuttingChooser.hpp"

namespace RoboPioneers::Prometheus
{
	/// 执行
	void CuttingChooser::Execute()
	{
		// 准许裁剪旗标，为true则将在函数的末尾发生裁剪
		bool approval_cutting {false};

		if (*Found)
		{
			// 如果找到目标

			if (LockingRemainTimes != 0)
			{
				// 如果已经处于追踪状态

				LockingRemainTimes = LockingStartupTimes;
				approval_cutting = true;
			}
			else
			{
				// 未处于追踪态，则进行判断

				auto intersection_area = static_cast<float>((*InterestedArea & LastInterestedArea).area());
				if (intersection_area / InterestedArea->area() > MinIntersectionAreaRatio)
				{
					// 认定为同一区域，准许计数减一
					--ApprovalRequiredTimes;
				}
				else
				{
					// 不是相同局域，重置准许计数
					ApprovalRequiredTimes = LockingApprovalThreshold;
				}

				if (ApprovalRequiredTimes == 0)
				{
					// 需要的推荐次数归零，则进入锁定状态
					LockingRemainTimes = LockingStartupTimes;
					approval_cutting = true;
					ApprovalRequiredTimes = LockingApprovalThreshold;
				}
			}

			// 记录该次兴趣区
			LastInterestedArea = *InterestedArea;
		}
		else
		{
			// 未找到

			if (LockingRemainTimes != 0)
			{
				// 已经处于锁定态

				// 更新锁定计数
				--LockingRemainTimes;
			}

			if (LockingRemainTimes != 0)
			{
				// 锁定计数归零前依然进行裁剪
				approval_cutting = true;
			}
		}

		if (approval_cutting)
		{
			// 进行裁剪
			(*OriginalPicture)(*InterestedArea).copyTo(CuttingPicture);
			PositionOffset.x = InterestedArea->x;
			PositionOffset.y = InterestedArea->y;
		}
		else
		{
			// 不进行裁剪
			CuttingPicture = (*OriginalPicture);
			PositionOffset.x = 0;
			PositionOffset.y = 0;
		}
	}
}
