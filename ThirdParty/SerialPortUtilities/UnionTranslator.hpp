#pragma once

namespace RoboPioneers::SerialPort::Utilities
{
	/**
	 * @brief 联合体翻译器模板
	 * @tparam StructureType 数据结构体类型
	 * @author Vincent
	 * @details
	 *  ~ 注意，结构体存在结构体对齐，故结构体占用内存大小有可能大于成员占用内存大小之和。
	 */
	template<typename StructureType>
	union UnionTranslator
	{
		/// 该成员用于访问数据
		StructureType Data;
		/// 该成员用于访问字节码
		unsigned char Bytes[sizeof(StructureType)];
	};
}
