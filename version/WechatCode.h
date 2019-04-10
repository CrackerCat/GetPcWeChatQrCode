#ifndef __DEF_WECHATCORE_MON__
#define __DEF_WECHATCORE_MON__

#include "windows.h"

#include <vector> 
using namespace std;
#include <atlstr.h>

class WechatCore
{
public:	
	VOID Start();
   

public:
	static WechatCore* Instance()
	{
		static WechatCore s_Instance;
		return &s_Instance;
	}
	
private:
	WechatCore::WechatCore() :
		// ԭʼģ����
		m_hModule(NULL),
		m_Init(0)
	{
		;
	}

	VOID _StartImpl();
	VOID StartHook();
    
public:
	HMODULE m_hModule;        // ע���ģ��ĵ�ַ

private:
	BOOL  m_Init;              // HOOK������ʼ��Ok
};






#endif