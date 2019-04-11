#ifndef __CMD_ERROR__
#define __CMD_ERROR__

#define CMD_ERROR_SUCCESS			0
#define CMD_ERROR_UNKNOWN			0xEFFFFFFF
#define CMD_ERROR_NET				-1			// �������
#define CMD_ERROR_POINTER			-2			// ��Чָ��
#define CMD_ERROR_SERVICE			-3			// Զ�̷����ʼ������
#define CMD_ERROR_COMMAND			-4			// ��Ч��֧�ֵ�����
#define CMD_ERROR_JSON				-5			// ��Ч��������JSON���ݸ�ʽ
#define CMD_ERROR_SIGN				-6			// ǩ��ʧ��
#define CMD_ERROR_FORMAT			-7			// ���ݰ���ʽ����
#define CMD_ERROR_SESSION			-8			// �Ựʧ�ܻ�Ự����
#define CMD_ERROR_MACHINE			-9			// ��Ч������Ϣ
#define CMD_ERROR_PROCESS			-10			// ��Ч����
#define CMD_ERROR_USER				-11			// ��Ч�û�
#define CMD_ERROR_LOGON				-12			// �û���δ��¼
#define CMD_ERROR_ACCESS			-13			// ��ЧȨ����Ϣ
#define CMD_ERROR_PRODUCT			-14			// ��Ч��Ʒ����ʿͻ���
#define CMD_ERROR_ARG				-15			// ��������
#define CMD_ERROR_DATABASE			-16			// ���ݿ���ʴ���
#define CMD_ERROR_ACCOUNT			-17			// �˻����ʳ���������
#define CMD_ERROR_SPACE				-18			// �û��ռ䲻��
#define CMD_ERROR_FILEDAYS			-19			// �û��ļ�ʱ�����

#define CMD_MSG_NOTCONNECT			"��δ���ӵ�Զ�̷�����"
#define CMD_MSG_NET					"�����쳣"
#define CMD_MSG_UNKNOWN				"δ֪����"
#define CMD_MSG_POINTER				"��Чָ����ָ��"
#define CMD_MSG_SERVICE				"Զ�̷����ʼ������"
#define CMD_MSG_COMMAND				"��Ч��֧�ֵ�����ؼ���"
#define CMD_MSG_JSON				"��Ч���ݰ����JSON��ʽ����"
#define CMD_MSG_SIGN				"��Ч����ǩ��"
#define CMD_MSG_FORMAT				"���ݰ���ʽ����"
#define CMD_MSG_SESSION				"�Ựʧ�ܻ�Ự����"
#define CMD_MSG_MACHINE				"��Ч��������Ϣ"
#define CMD_MSG_PROCESS				"��Ч����"
#define CMD_MSG_USER				"��Ч�û�"

#endif // end of define __CMD_ERROR__