#pragma once

#include <cstddef>

namespace RoboPioneers::SerialPort::Utilities
{
	/**
	 * @brief 字节访问器
	 * @author Vincent
	 * @details
	 *  ~ 该类提供了对字节的访问能力。
	 */
	class BytesAccessor
	{
	protected:
		/// 数据起始地址
		void* Address;
		/// 数据大小
		std::size_t Size;

	public:
		/**
		 * @brief 构造函数
		 * @param address 地址
		 * @param size 大小
		 */
		BytesAccessor(void* address, std::size_t size) : Address(address), Size(size)
		{}

		/**
		 * @brief 将数据视为整体并获取地址
		 * @return 数据地址
		 */
		[[nodiscard]] inline void* GetData() const
		{
			return Address;
		}

		/**
		 * @brief 将数据视为字节数组并获取地址
		 * @return 字节数组地址
		 */
		[[nodiscard]] inline unsigned char* GetBytes() const
		{
			return static_cast<unsigned char*>(Address);
		}

		/**
		 * @brief 获取数据大小
		 * @return 数据大小，单位为字节
		 */
		[[nodiscard]] inline std::size_t GetSize() const
		{
			return Size;
		}

		/**
		 * @brief 访问成员
		 * @tparam Type 成员解读类型
		 * @param start_index 起始索引
		 * @return 指定范围内字节的引用
		 */
		template<typename Type>
		Type& Access(unsigned int start_index)
		{
			return *reinterpret_cast<Type*>(&(GetBytes()[start_index]));
		}
	};
}
