#ifndef __CMD_ERROR__
#define __CMD_ERROR__

#define CMD_ERROR_SUCCESS			0
#define CMD_ERROR_UNKNOWN			0xEFFFFFFF
#define CMD_ERROR_NET				-1			// 网络错误
#define CMD_ERROR_POINTER			-2			// 无效指针
#define CMD_ERROR_SERVICE			-3			// 远程服务初始化错误
#define CMD_ERROR_COMMAND			-4			// 无效或不支持的命令
#define CMD_ERROR_JSON				-5			// 无效包，不是JSON数据格式
#define CMD_ERROR_SIGN				-6			// 签名失败
#define CMD_ERROR_FORMAT			-7			// 数据包格式错误
#define CMD_ERROR_SESSION			-8			// 会话失败或会话超期
#define CMD_ERROR_MACHINE			-9			// 无效主机信息
#define CMD_ERROR_PROCESS			-10			// 无效程序
#define CMD_ERROR_USER				-11			// 无效用户
#define CMD_ERROR_LOGON				-12			// 用户尚未登录
#define CMD_ERROR_ACCESS			-13			// 无效权限信息
#define CMD_ERROR_PRODUCT			-14			// 无效产品或访问客户端
#define CMD_ERROR_ARG				-15			// 参数错误
#define CMD_ERROR_DATABASE			-16			// 数据库访问错误
#define CMD_ERROR_ACCOUNT			-17			// 账户访问出错，或余额不足
#define CMD_ERROR_SPACE				-18			// 用户空间不足
#define CMD_ERROR_FILEDAYS			-19			// 用户文件时间错误

#define CMD_MSG_NOTCONNECT			"尚未连接到远程服务器"
#define CMD_MSG_NET					"网络异常"
#define CMD_MSG_UNKNOWN				"未知错误"
#define CMD_MSG_POINTER				"无效指针或空指针"
#define CMD_MSG_SERVICE				"远程服务初始化错误"
#define CMD_MSG_COMMAND				"无效或不支持的命令关键字"
#define CMD_MSG_JSON				"无效数据包或非JSON格式数据"
#define CMD_MSG_SIGN				"无效数据签名"
#define CMD_MSG_FORMAT				"数据包格式错误"
#define CMD_MSG_SESSION				"会话失败或会话超期"
#define CMD_MSG_MACHINE				"无效的主机信息"
#define CMD_MSG_PROCESS				"无效程序"
#define CMD_MSG_USER				"无效用户"

#endif // end of define __CMD_ERROR__