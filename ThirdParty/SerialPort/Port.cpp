#include "Port.hpp"

#include <utility>
#include <stdexcept>

namespace RoboPioneers::SerialPort
{
	/// 构造并绑定参数
	Port::Port(std::string file_name, unsigned int baud_rate, unsigned int character_size,
	           boost::asio::serial_port_base::flow_control::type flow_control,
	           boost::asio::serial_port_base::parity::type parity,
	           boost::asio::serial_port_base::stop_bits::type stop_bits) :
			DeviceFileName(std::move(file_name)),
			BaudRateSetting(baud_rate), CharacterSizeSetting(character_size),
			FlowControlSetting(flow_control), ParitySetting(parity), StopBitsSetting(stop_bits),
			Context(), Device(Context)
	{}

	/// 析构，若设备未关闭则关闭设备
	Port::~Port()
	{
		if (Device.is_open())
		{
			Device.close();
		}
	}

	/// 开启方法
	void Port::Open()
	{
		if (Device.is_open())
		{
			Device.close();
		}
		Device.open(DeviceFileName);

		Device.set_option(boost::asio::serial_port_base::baud_rate(BaudRateSetting));
		Device.set_option(boost::asio::serial_port_base::character_size(CharacterSizeSetting));
		Device.set_option(boost::asio::serial_port_base::flow_control(FlowControlSetting));
		Device.set_option(boost::asio::serial_port_base::parity(ParitySetting));
		Device.set_option(boost::asio::serial_port_base::stop_bits(StopBitsSetting));
	}

	/// 关闭方法
	void Port::Close()
	{
		if (Device.is_open())
		{
			Device.close();
		}
	}

	/// 设置默认缓冲区大小
	void Port::SetDefaultBufferSize(unsigned long buffer_size)
	{
		DefaultBufferSize = buffer_size;
	}

	/// 设置波特率
	void Port::SetBaudRate(unsigned int baud_rate)
	{
		BaudRateSetting = baud_rate;
		if (Device.is_open())
		{
			Device.set_option(boost::asio::serial_port_base::baud_rate(baud_rate));
		}
	}

	/// 设置字符带大小
	void Port::SetCharacterSize(unsigned int size)
	{
		CharacterSizeSetting = size;
		if (Device.is_open())
		{
			Device.set_option(boost::asio::serial_port_base::character_size(size));
		}
	}

	/// 设置流控制
	void Port::SetFlowControlType(boost::asio::serial_port_base::flow_control::type flow_control)
	{
		FlowControlSetting = flow_control;
		if (Device.is_open())
		{
			Device.set_option(boost::asio::serial_port_base::flow_control(flow_control));
		}
	}

	/// 设置奇偶校验类型
	void Port::SetParityType(boost::asio::serial_port_base::parity::type parity)
	{
		ParitySetting = parity;
		if (Device.is_open())
		{
			Device.set_option(boost::asio::serial_port_base::parity(parity));
		}
	}

	/// 设置停止位类型
	void Port::SetStopBitsType(boost::asio::serial_port_base::stop_bits::type stop_bits)
	{
		StopBitsSetting = stop_bits;
		if (Device.is_open())
		{
			Device.set_option(boost::asio::serial_port_base::stop_bits(stop_bits));
		}
	}

	/// 写入原始内存
	void Port::Write(void *pointer, std::size_t size)
	{
		if (Device.is_open())
		{
			boost::asio::write(Device, boost::asio::buffer(pointer, size));
		}
		else
		{
			throw std::runtime_error("[Port::Write] Device is Not Opened.");
		}
	}

	/// 读取字节到指定位置
	unsigned long Port::ReadTo(void *buffer, std::size_t buffer_size, long target_size)
	{
		if (Device.is_open())
		{
			if (target_size < 0)
			{
				return Device.read_some(boost::asio::buffer(buffer, buffer_size));
			}
			else
			{
				if (buffer_size < target_size)
				{
					throw std::runtime_error("[Port::Write] Buffer Size is Smaller Than Target Size.");
				}
				return boost::asio::read(Device, boost::asio::buffer(buffer, target_size));
			}
		}
		else
		{
			throw std::runtime_error("[Port::Write] Device is Not Opened.");
		}
	}

	/// 读取文本
	std::string Port::ReadText(long target_size)
	{
		std::string text;

		unsigned long buffer_size = DefaultBufferSize;
		if (target_size > 0)
		{
			buffer_size = target_size;
		}
		text.resize(buffer_size);

		text.resize(ReadTo(text.data(), buffer_size, target_size));

		return text;
	}

	/// 读取字节
	std::vector<unsigned char> Port::ReadBytes(long target_size)
	{
		std::vector<unsigned char> buffer;

		unsigned long buffer_size = DefaultBufferSize;
		if (target_size > 0)
		{
			buffer_size = target_size;
		}
		buffer.resize(buffer_size);

		buffer.resize(ReadTo(buffer.data(), buffer_size, target_size));

		return buffer;
	}
}