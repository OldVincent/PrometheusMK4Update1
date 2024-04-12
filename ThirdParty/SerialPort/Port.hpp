#pragma once

#include <string>
#include <boost/asio.hpp>
#include <vector>

namespace RoboPioneers::SerialPort
{
	/**
	 * @brief 串口类
	 * @author Vincent
	 * @details
	 *  ~ 该类与一个物理串口相对应。
	 */
	class Port
	{
	public:
		/// 硬件流控制类型
		using FlowControlEnum = boost::asio::serial_port::flow_control;
		/// 奇偶校验类型`
		using ParityEnum = boost::asio::serial_port::parity;
		/// 停止位类型
		using StopBitsEnum = boost::asio::serial_port::stop_bits;

	private:
		//==============================
		// 设定部分
		//==============================

		/// 默认缓冲区大小
		unsigned long DefaultBufferSize {256};

		/**
		 * @brief 设备文件名
		 * @details
		 *  ~ 操作系统将串口设备的读写封装为文件读写，故需提供串口的映射文件的名称。
		 *  ~ 若该映射文件不存在，则打开将出错；
		 *    若该映射文件存在，且未开启流控，则由于串口的特性，无论另一端是否连接，都将打开成功。
		 */
		std::string DeviceFileName;

		/**
		 * @brief 波特率设定
		 * @details 该选项控制串口传输速率，需要通信双方保持一致。
		 */
		unsigned int BaudRateSetting;

		/**
		 * @brief 字符大小
		 * @details 该选项控制单次传输的比特位数，需要通信双方保持一致。
		 */
		unsigned int CharacterSizeSetting;

		/**
		 * @brief 硬件流控制设定
		 * @details 硬件流控制可以设定双方的串口读写顺序，需要通信双方保持一致。
		 */
		FlowControlEnum::type FlowControlSetting;
		/**
		 * @brief 奇偶校验设定
		 * @details 开启奇偶校验可以确保残损的数据包被拦截，需要通信双方保持一致。
		 */
		ParityEnum::type ParitySetting;
		/**
		 * @brief 停止位设定
		 * @details 停止位决定了单词传输得中止位，需要通信双方保持一致。
		 */
		StopBitsEnum::type StopBitsSetting;

	protected:
		//==============================
		// 读写对象部分
		//==============================

		/// 上下文对象
		boost::asio::io_context Context;
		/// 串口设备对象
		boost::asio::serial_port Device;

	public:
		//==============================
		// 构造与析构部分
		//==============================

		/**
		 * @brief 构造并绑定串口文件
		 * @param file_name 串口设备文件名
		 */
		explicit Port(std::string file_name,
	         unsigned int baud_rate = 115200, unsigned int character_size = 8,
	         FlowControlEnum::type flow_control = FlowControlEnum::none,
	         ParityEnum::type parity = ParityEnum::none,
	         StopBitsEnum::type stop_bits = StopBitsEnum::one
	        );

		/// 析构，若串口设备未关闭，则将自动关闭
		virtual ~Port();

	public:

		//==============================
		// 基本控制部分
		//==============================

		/// 打开串口
		virtual void Open();

		/// 关闭串口
		virtual void Close();

		/**
		 * @brief 是否已经开启
		 * @retval true 设备处于开启状态
		 * @retval false 设备处于关闭状态
		 */
		[[nodiscard]] inline bool IsOpened()
		{
			return Device.is_open();
		}

		//==============================
		// 属性设置部分
		//==============================

		/// 设置默认缓冲区大小
		void SetDefaultBufferSize(unsigned long buffer_size);

		/// 设置波特率
		virtual void SetBaudRate(unsigned int baud_rate);

		/// 设置字符大小
		virtual void SetCharacterSize(unsigned int size);

		/// 设置流控制类型
		virtual void SetFlowControlType(FlowControlEnum::type flow_control);

		/// 设置奇偶校验类型
		virtual void SetParityType(ParityEnum::type parity);

		/// 设置停止位类型
		virtual void SetStopBitsType(StopBitsEnum::type stop_bits);

		//==============================
		// 读写部分
		//==============================

		/**
		 * @brief 向串口写入内存中的数据
		 * @param pointer 指针
		 * @param size 大小
		 */
		void Write(void* pointer, std::size_t size);

		/**
		 * @brief 向串口写入文本
		 * @param text 文本内容
		 */
		void Write(const std::string& text)
		{
			Write(const_cast<char*>(text.data()), text.size());
		}

		/**
		 * @brief 向串口写入字节向量
		 * @param bytes 字节向量
		 */
		void Write(const std::vector<unsigned char>& bytes)
		{
			Write(const_cast<unsigned char*>(bytes.data()), bytes.size());
		}

		/**
		 * @brief 写入某个对象的数据
		 * @tparam Type 类型
		 * @param target 目标对象
		 */
		template<typename Type>
		void Write(const Type& target)
		{
			Write(static_cast<void*>(&target), sizeof(Type));
		}

		/**
		 * @brief 读取字节码
		 * @param target_size 若为正数，读取到指定字节数后返回；若为负数，则不设定目标字节数，缓存区大小为DefaultBufferSize的值
		 * @return 字节向量
		 */
		std::vector<unsigned char> ReadBytes(long target_size = -1);

		/**
		 * @brief 读取文本
		 * @param target_size 若为正数，读取到指定字节数后返回；若为负数，则不设定目标字节数，缓存区大小为DefaultBufferSize的值
		 * @return 按照文本的方式读取到的数据
		 */
		std::string ReadText(long target_size = -1);

		/**
		 * @brief 读取数据到指定的地址
		 * @param buffer 地址
		 * @param buffer_size 缓存大小
		 * @param target_size 若为正数，读取到指定字节数后返回；若为负数，则不设定目标字节数，缓存区大小为DefaultBufferSize的值
		 * @return 读取的字节数
		 */
		unsigned long ReadTo(void* buffer, std::size_t buffer_size, long target_size = -1);

		/**
		 * @brief 将数据读取到指定目标
		 * @tparam Type 类型
		 * @param target 目标
		 */
		template<typename Type>
		void ReadTo(Type& target)
		{
			ReadTo(&target, sizeof(Type), sizeof(Type));
		}

		/**
		 * @brief 将数据读取为指定类型
		 * @tparam Type 类型
		 * @return 存储读取的数据的对象
		 */
		template<typename Type>
		Type ReadAs()
		{
			Type target;
			ReadTo(&target, sizeof(Type), sizeof(Type));
			return target;
		}

		//==============================
		// 操作符重载
		//==============================

		/// 流传入操作符，用于写数据
		template<typename Type>
		Port& operator<<(const Type& data)
		{
			Write(data);
			return *this;
		}

		/// 流传出操作符，用于读数据
		template<typename Type>
		Port& operator>>(Type& data)
		{
			ReadTo(data);
			return *this;
		}
	};
}