#include "stdafx.h"
#include "wechatcode.h"
#include "mhook-lib/mhook.h"
#include "log.h"


#include "png/lodepng.h"

#include "BarcodeFormat.h"
#include "DecodeHints.h"
#include "MultiFormatReader.h"
#include "GenericLuminanceSource.h"
#include "HybridBinarizer.h"
#include "Result.h"
#include "ResultMetadata.h"
#include "TextUtfEncoding.h"

#include "MultiFormatWriter.h"
#include "BitMatrix.h"
#include "TextUtfEncoding.h"
#include "ZXStrConvWorkaround.h"

using namespace ZXing;
using std::string;

#if 0
using Binarizer = ZXing::GlobalHistogramBinarizer;
#else
using Binarizer = ZXing::HybridBinarizer;
#endif

#ifdef _DEBUG
#pragma comment(lib, "libzxing/lib/ZXingCoreD.lib")
#else
#pragma comment(lib, "libzxing/lib/ZXingCore.lib")
#endif // DEBUG


typedef enum {
	QR_ECLEVEL_L = 0, ///< lowest
	QR_ECLEVEL_M,
	QR_ECLEVEL_Q,
	QR_ECLEVEL_H      ///< highest
} QRecLevel;


#define UtlAlloc(size)    HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (size))
#define UtlFree(p)        HeapFree(GetProcessHeap(), 0, (p))


// Hook��QRCODE��ԭ��
typedef VOID(_stdcall* QRCODE)();
QRCODE        RelQRCODE = NULL;


VOID __stdcall QRCODEProxy(LPBYTE lpdata, DWORD dwdatalen, LPVOID lpthis)
{
	std::vector<unsigned char> buffer;
	unsigned width, height;
	unsigned error = lodepng::decode(buffer, width, height, lpdata, dwdatalen, LCT_RGBA, 8);
	if (error) 
	{
		MyAttW(L"[%s] ======------******����PngͼƬʧ��******------====== \n", __FUNCTIONW__);
		return;
	}

	DecodeHints hints;
	hints.setShouldTryHarder(false);
	MultiFormatReader reader(hints);

	GenericLuminanceSource source((int)width, (int)height, buffer.data(), width * 4, 4, 0, 1, 2);
	Binarizer binImage(std::shared_ptr<LuminanceSource>(&source, [](void*) {}));

	auto result = reader.read(binImage);

	if (result.isValid()) 
	{
		MyAttW(L"[%s] ======------******������ά���������%s******------====== \n", __FUNCTIONW__, result.text().c_str());
	}
	else
	{
		MyAttW(L"[%s] ======------******����ԭʼ��ά�������ʧ��******------====== \n", __FUNCTIONW__);
		return;
	}


	MultiFormatWriter writer(BarcodeFormat::QR_CODE);
	writer.setEccLevel(QR_ECLEVEL_H);

	auto matrix = writer.encode(result.text(), 200, 200);

	std::vector<unsigned char> b1uffer(matrix.width() * matrix.height() * 4, '\0');
	//unsigned char black = 0xffffff;
	DWORD white = 0xffffffff;

	int halfW = matrix.width() / 2;
	int halfH = matrix.height() / 2;
	
	LPDWORD tmp1 = (LPDWORD)b1uffer.data();

	for (int y = 0; y < matrix.height(); ++y) {
		for (int x = 0; x < matrix.width(); ++x) {

			DWORD tmp = 0;
			if (x < matrix.width() / 2 && y < matrix.height() / 2)
			{
				int num1 = (int)(122 - (122.0 - 13.0) / matrix.height()
					* (y + 1));
				int num2 = (int)(194 - (194.0 - 72.0) / matrix.height()
					* (y + 1));
				int num3 = (int)(252 - (252.0 - 107.0)
					/ matrix.height() * (y + 1));

				tmp = RGB(num1, num2, num3) | 0xff << 24;
			}

			if (x < matrix.width() / 2 && y >= matrix.height() / 2)
			{
				int num1 = (int)(54 - (54.0 - 13.0) / matrix.height()
					* (y + 1));
				int num2 = (int)(176 - (176.0 - 72.0) / matrix.height()
					* (y + 1));
				int num3 = (int)(157 - (157 - 107.0)
					/ matrix.height() * (y + 1));

				tmp = RGB(num1, num2, num3) | 0xff << 24;
			}

			if (x >= matrix.width() / 2 && y < matrix.height() / 2)
			{
				int num1 = (int)(66 - (66.0 - 13.0) / matrix.height()
					* (y + 1));
				int num2 = (int)(87 - (87.0 - 72.0) / matrix.height()
					* (y + 1));
				int num3 = (int)(115 - (115 - 107.0)
					/ matrix.height() * (y + 1));

				tmp = RGB(num1, num2, num3) | 0xff << 24;
			}

			if (x >= matrix.width() / 2 && y >= matrix.height() / 2)
			{
				int num1 = (int)(226 - (226.0 - 13.0) / matrix.height()
					* (y + 1));
				int num2 = (int)(129 - (129.0 - 72.0) / matrix.height()
					* (y + 1));
				int num3 = (int)(133 - (133 - 107.0)
					/ matrix.height() * (y + 1));

				tmp = RGB(num1, num2, num3) | 0xff << 24;
			}

			// �޸������ɫ�Ϳ�����
			// tmp = RGB(54, 176, 157) | 0xff << 24;
			// 66 87 115
			// 226 129 133
			tmp1[y * matrix.width() + x] = matrix.get(x, y) ? tmp : white;
		}
	}

	std::vector<unsigned char> buffer2;
	error = lodepng::encode(buffer2, b1uffer, matrix.width(), matrix.height(), LCT_RGBA, 8);

	if (error) 
	{
		MyAttW(L"[%s] ======------******�����ɫ��ά������ʧ��******------====== \n", __FUNCTIONW__);
		return ;
	}
	MyAttW(L"[%s] ======------******�����ɫ��ά�����ݳɹ�******------====== \n", __FUNCTIONW__);

	LPVOID tmp = UtlAlloc(buffer2.size());
	CopyMemory(tmp, &buffer2[0], buffer2.size());

	// �滻��ָ��
	*(LPDWORD(lpthis) + 301) = buffer2.size();
	*(LPDWORD(lpthis) + 300) = (DWORD)tmp;
}


__declspec(naked) VOID __stdcall WechatQRCODE()
{
    // �����Ĵ���
	__asm
	{
		sub esp, 0x20;
		PUSHAD;	
	}
	__asm
	{
		mov     edx, [esi + 4B4h] // edx��ʱ�Ƕ�ά���buffer�ĳ���
		mov     ecx, [esi + 4B0h] // ecx��ʱ�Ƕ�ά��bufferָ��
		push    esi               // esi����ָ��,������QRCODEProxy����Ҫ���¶�esi + 4B4h esi + 4B0h�����ݸ�ֵ
		push    edx
		push    ecx
		call    QRCODEProxy;
	}
	// �ָ��Ĵ���
	__asm
	{
		POPAD;
		add esp, 0x20;
	}
	__asm jmp RelQRCODE;
}


VOID WechatCore::Start()
{  
	BOOL bfIND = FALSE;
	for (int i = 0; i < 10; i++)
	{
		m_hModule = GetModuleHandle(L"WECHATWIN.dll");
		if (m_hModule)
		{
			bfIND = TRUE;
			break;
		}
		else
		{
			Sleep(200);
		}
	}

	if (!bfIND)
	{
		MyAttW(L"[%s] 2s��û���ҵ� WECHATWIN \n", __FUNCTIONW__ );
		return;
	}
	
	MyAttW(L"[%s] WECHATWINģ���ַ��%0x \n", __FUNCTIONW__, DWORD(m_hModule));
	
	_StartImpl();
}


static BOOL PatchMemoryDWORD(
	__in const PVOID pAddr,
	__in const DWORD uDword)
{
	DWORD dwOldProtect = 0;
	BOOL bRetVal = FALSE;

	if (VirtualProtect(pAddr, sizeof(ULONG), PAGE_EXECUTE_READWRITE, &dwOldProtect))
	{
		*(PDWORD)pAddr = uDword;
		bRetVal = VirtualProtect(pAddr, sizeof(ULONG), dwOldProtect, &dwOldProtect);
	}

	return bRetVal;
}


// ��Xlog����
VOID EnableXlogOpen()
{
	// ���ȵ�����log��level
	PatchMemoryDWORD(PVOID((ULONG_PTR)WechatCore::Instance()->m_hModule + 0x111F8280 - 0x10000000), 0);

	// �������log��dbgview
	PatchMemoryDWORD(PVOID((ULONG_PTR)WechatCore::Instance()->m_hModule + 0x1125298D - 0x10000000), 1);
}


VOID WechatCore::_StartImpl()
{
    MyAttW(L"[%s] ��ʼ������......\n", __FUNCTIONW__);
	{
		// ���2.6.7.57 ֱ�ӵ��,��������������,͵����,ֱ��Ӳƫ����
		RelQRCODE = (QRCODE)((ULONG_PTR)m_hModule + 0x1014CEC4 - 0x10000000);
		MyAttW(L"[%s] RelQRCODEλ����%x", __FUNCTIONW__,
			(ULONG_PTR)RelQRCODE);
        m_Init = TRUE;
	}
	
	if (m_Init)
	{
		// ��ʼִ��Hook
		StartHook();
	}
    
	EnableXlogOpen();
}


VOID WechatCore::StartHook()
{
	if (RelQRCODE)
	{
		if (!Mhook_SetHook((PVOID*)&RelQRCODE, WechatQRCODE))
		{

		}
	}
}
