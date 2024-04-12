#pragma once

#include <vector>
#include <stdexcept>

namespace RoboPioneers::SerialPort::Utilities
{
	/**
	 * @brief 字节栈
	 * @author Vincent
	 * @details
	 *  ~ 字节栈维护一个字节向量，提供向内添加元素或弹出元素的操作。
	 */
	class BytesStack
	{
	public:
		/// 字节向量
		std::vector<unsigned char> Bytes;

	private:
		/// 当前索引
		unsigned int CurrentIndex {0};

	public:
		/**
		 * @brief 存入元素，其将被存入在末尾
		 * @tparam Type 元素类型
		 * @param element 元素对象
		 */
		template<typename Type>
		void Push(const Type& element)
		{
			(*reinterpret_cast<Type*>(&Bytes[CurrentIndex])) = element;
			CurrentIndex += sizeof(Type);
		}

		/**
		 * @brief 弹出元素，栈尾将收缩至该元素的头部
		 * @tparam Type
		 * @return 元素的值
		 * @details
		 *  ~ 元素的内存会被后续的入栈操作覆盖，但析构函数不会被调用。
		 */
		template<typename Type>
		Type Pop()
		{
			if (CurrentIndex - sizeof(Type) >= 0)
			{
				CurrentIndex -= sizeof(Type);
				return (*reinterpret_cast<Type*>(&Bytes[CurrentIndex]));
			}
			else
			{
				throw std::out_of_range("[BytesStack::Pop] Stack Size is Smaller Than Element Size.");
			}
		}
	};
}