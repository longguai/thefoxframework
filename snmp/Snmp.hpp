#ifdef _THEFOX_SNMP_H_
#define _THEFOX_SNMP_H_

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

namespace thefox
{

class Snmp
{
public:
	Snmp()
		:_sessionPtr(NULL)
	{}
	
	~Snmp()
	{}
	
	bool open1(const std::string &ipAddr, const std::string &community, const int port = 161, const int retries = 0, const int timeout = 3 * 1000 * 1000)
	{
		return open(ipAddr, community, port, retries, timeout, SNMP_VERSION_1);
	}
	
	bool open2(const std::string &ipAddr, const std::string &community, const int port = 161, const int retries = 0, const int timeout = 3 * 1000 * 1000)
	{ 
		return open(ipAddr, community, port, retries, timeout, SNMP_VERSION_2c);
	}
	
	/// @TODO bool open3(...)
	
	void close()
	{
		if (NULL == _sessionPtr)
		{
			snmp_close(_sessionPtr);
			_sessionPtr = NULL;
		}
	}
	
	bool get(const std::string &oids, std::vector<string> &values)
	{
		if (0 == oids.length())
			return false;
			
		netsnmp_pdu *pdu = NULL;
		netsnmp_pdu *response = NULL;
		netsnmp_variable_list *vars = NULL;
		
		if (NULL == (pdu = snmp_pdu_create(SNMP_MSG_GET)))
			return false;
		
		// ����OID
		size_t pos = 0;
		size_t lastPos = 0;
		while (true)
		{
			pos = oids.find_first_of(',', lastPos);
			if (std::string::npos == pos)
			{
				pos = oids.length();
				if (pos != lastPos)
				{
					String oidBuf = oids.substr(lastPos, pos - lastPos);
					snmp_add_null_var(pdu, oidBuf.c_str(), oidBuf.length());
				}
				break;
			}
			else
			{
				if (pos != lastPos)
				{
					String oidBuf = oids.substr(lastPos, pos - lastPos);
					snmp_add_null_var(pdu, oidBuf.c_str(), oidBuf.length());
				}
			}
			lastPos = pos + 1;
		}
		
		int status = snmp_synch_response(_sessionPtr, pdu, response);
		if (STAT_SUCCESS == status)
		{
			if (SNMP_ERR_NOERROR == response->errstat)
			{
				values.clear();
				for (vars = response->variables; vars; vars = vars->next_variable)
				{
					values.push_back(asn2String(vars));
				}
			}
			else
			{
				// PDU����
				return false;
			}
		}
		else if (STAT_TIMEOUT == status)
		{
			// ��ʱ
			
		}
		else // status == STAT_ERROR
		{
			// ʧ��
			//TRACE(snmp_sess_perror(_sessionPtr));
			
		}
		
		if (response)
			snmp_free_pdu(response);
		
		return (STAT_SUCCESS == status) ? true : false;
	}
	
	bool getNext(const std::string &oids, std::string &newOids, std::vector<string> &values)
	{
		if (0 == oids.length())
			return false;
			
		netsnmp_pdu *pdu = NULL;
		netsnmp_pdu *response = NULL;
		netsnmp_variable_list *vars = NULL;
		
		if (NULL == (pdu = snmp_pdu_create(SNMP_MSG_GET)))
			return false;
		
		// ����OID
		size_t pos = 0;
		size_t lastPos = 0;
		while (true)
		{
			pos = oids.find_first_of(',', lastPos);
			if (std::string::npos == pos)
			{
				pos = oids.length();
				if (pos != lastPos)
				{
					String oidBuf = oids.substr(lastPos, pos - lastPos);
					snmp_add_null_var(pdu, oidBuf.c_str(), oidBuf.length());
				}
				break;
			}
			else
			{
				if (pos != lastPos)
				{
					String oidBuf = oids.substr(lastPos, pos - lastPos);
					snmp_add_null_var(pdu, oidBuf.c_str(), oidBuf.length());
				}
			}
			lastPos = pos + 1;
		}
		
		int status = snmp_synch_response(_sessionPtr, pdu, response);
		if (STAT_SUCCESS == status)
		{
			if (SNMP_ERR_NOERROR == response->errstat)
			{
				newOids = "";
				values.clear();
				for (vars = response->variables; vars; vars = vars->next_variable)
				{
					
					values.push_back(asn2String(vars));
				}
			}
			else
			{
				// PDU����
				return false;
			}
		}
		else if (STAT_TIMEOUT == status)
		{
			// ��ʱ
			
		}
		else // status == STAT_ERROR
		{
			// ʧ��
			//TRACE(snmp_sess_perror(_sessionPtr));
			
		}
		
		if (response)
			snmp_free_pdu(response);
		
		return (STAT_SUCCESS == status) ? true : false;
	}
	
private:
	bool open(const std::string &ipAddr, 
				const std::string &community,
				const int port,
				const int retries,
				const int timeout,
				const int version)
	{
		char addr[128];
		snprintf(addr, sizeof(addr), "%s:%d", ipAddr.c_str(), port);
		
		snmp_sess_init(&_session);
		_session.version = version; // �汾
		_session.retries = retries; // ���Դ���
		_session.timeout =  timeout; // ��ʱʱ�� ΢���net-snmpĬ��ֵΪ1s��
		_session.peername = addr;
		_session.remote_port = port; // �°治��ʹ����
		_session.community = community.c_str();
		_session.community_len = community.length();
		
		if (NULL == (ss = snmp_open(&session)))
			return false;
		return true;
	}
	
	const String asn2String(netsnmp_variable_list *var)
	{
		
		switch (var->type)
		{
			case ASN_INTEGER:
			{
				char str[32];
				snprintf(str, sizeof(str), "%ld", *var->val.integer);
				return str;
			}
			case ASN_OCTET_STR:
			{
				String str;
				for(int i = 0; i < var->val_len; ++i)
				{
					str += var->val.string[i];
				}
				return str;
			}
			case ASN_BIT_STR:
				
			case ASN_OPAQUE:
				
			case ASN_OBJECT_ID:
				
			case ASN_TIMETICKS:
				
			case ASN_GAUGE:
				
			case ASN_COUNTER:
				
			case ASN_IPADDRESS:
				
			case ASN_NULL:
			
			case ASN_UINTEGER:
				
			case ASN_COUNTER64:
			case ASN_OPAQUE_U64:
			case ASN_OPAQUE_I64:
			case ASN_OPAQUE_COUNTER64:
			
			case ASN_OPAQUE_FLOAT:
				
			case ASN_OPAQUE_DOUBLE:
				
			default:
			
		}
	}
	
	struct snmp_session _session;
	struct snmp_session *_sessionPtr;
};

} // nameapace thefox

#endif // _THEFOX_SNMP_H_