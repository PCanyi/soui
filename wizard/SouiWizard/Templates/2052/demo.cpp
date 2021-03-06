// dui-demo.cpp : main source file
//

#include "stdafx.h"
#include "MainDlg.h"
[!if CHECKBOX_SHELLNOTIFYICON]
#include "trayicon\SShellNotifyIcon.h"
[!endif]
//从PE文件加载，注意从文件加载路径位置
#define RES_TYPE [!output ResLoaderType]
//#define SYSRES_TYPE [!output ResLoaderType]
// #define RES_TYPE 0   //PE
// #define RES_TYPE 1   //ZIP
// #define RES_TYPE 2   //7z
// #define RES_TYPE 2   //文件
//去掉多项支持，以免代码显得混乱
#if (RES_TYPE==1)
#include "resprovider-zip\zipresprovider-param.h"
#else 
#if (RES_TYPE==2)
#include "resprovider-7zip\zip7resprovider-param.h"
#endif
#endif
#ifdef _DEBUG
#define SYS_NAMED_RESOURCE _T("soui-sys-resourced.dll")
#else
#define SYS_NAMED_RESOURCE _T("soui-sys-resource.dll")
#endif
[!if CHECKBOX_USE_LUA]
#ifdef _DEBUG
#pragma comment(lib,"lua-52d")
#pragma comment(lib,"scriptmodule-luad")
#else
#pragma comment(lib,"lua-52")
#pragma comment(lib,"scriptmodule-lua")
#endif
[!endif]


//定义唯一的一个R,UIRES对象,ROBJ_IN_CPP是resource.h中定义的宏。
ROBJ_IN_CPP

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE /*hPrevInstance*/, LPTSTR lpstrCmdLine, int /*nCmdShow*/)
{
	HRESULT hRes = OleInitialize(NULL);
	SASSERT(SUCCEEDED(hRes));

	int nRet = 0;

	SComMgr *pComMgr = new SComMgr;
	TCHAR szCurrentDir[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, szCurrentDir, sizeof(szCurrentDir));

	LPTSTR lpInsertPos = _tcsrchr(szCurrentDir, _T('\\'));
#ifdef _DEBUG
	_tcscpy(lpInsertPos + 1, _T("..\\[!output PROJECT_NAME]"));
#else
	_tcscpy(lpInsertPos + 1, _T("\0"));
#endif
	SetCurrentDirectory(szCurrentDir);
	{
		BOOL bLoaded = FALSE;
		CAutoRefPtr<SOUI::IImgDecoderFactory> pImgDecoderFactory;
		CAutoRefPtr<SOUI::IRenderFactory> pRenderFactory;
[!if RADIO_RANDER_GDI]
		bLoaded = pComMgr->CreateRender_GDI((IObjRef**)&pRenderFactory);
[!else]
		bLoaded = pComMgr->CreateRender_Skia((IObjRef**)&pRenderFactory);
[!endif]
		SASSERT_FMT(bLoaded, _T("load interface [render] failed!"));
		bLoaded = pComMgr->CreateImgDecoder((IObjRef**)&pImgDecoderFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("imgdecoder"));

		pRenderFactory->SetImgDecoderFactory(pImgDecoderFactory);
		SApplication *theApp = new SApplication(pRenderFactory, hInstance);
[!if CHECKBOX_RES_PACK_ONLAY_RELEASE]
#ifdef _DEBUG
		//选择了仅在Release版本打包资源则系统资源在DEBUG下始终使用DLL加载
		{
			HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
			if (hModSysResource)
			{
				CAutoRefPtr<IResProvider> sysResProvider;
				CreateResProvider(RES_PE, (IObjRef**)&sysResProvider);
				sysResProvider->Init((WPARAM)hModSysResource, 0);
				theApp->LoadSystemNamedResource(sysResProvider);
				FreeLibrary(hModSysResource);
			}
			else
			{
				SASSERT(0);
			}
		}
#else		
[!if CHECKBOX_SYSRES_BUILTIN]
		//钩选了复制系统资源选项
		{
			CAutoRefPtr<IResProvider> pSysResProvider;
[!if ResLoaderType == 0]
			CreateResProvider(RES_PE, (IObjRef**)&pSysResProvider);
			bLoaded = pSysResProvider->Init((WPARAM)hInstance, 0);
			SASSERT(bLoaded);
			bLoaded = theApp->LoadSystemNamedResource(pSysResProvider);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 1]
			bLoaded = pComMgr->CreateResProvider_ZIP((IObjRef**)&pSysResProvider);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
			ZIPRES_PARAM param;
			param.ZipFile(pRenderFactory, _T("uires.zip"), "[!output ZIP_PSW]",_T("theme_sys_res"));
			bLoaded = pSysResProvider->Init((WPARAM)&param, 0);
			SASSERT(bLoaded);
			bLoaded = theApp->LoadSystemNamedResource(pSysResProvider);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 2]
			bLoaded = pComMgr->CreateResProvider_7ZIP((IObjRef**)&pSysResProvider);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
			ZIP7RES_PARAM param;
			param.ZipFile(pRenderFactory, _T("uires.7z"), "[!output ZIP_PSW]", _T("theme_sys_res"));
			bLoaded = pSysResProvider->Init((WPARAM)&param, 0);
			SASSERT(bLoaded);
			bLoaded = theApp->LoadSystemNamedResource(pSysResProvider);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 3]
			CreateResProvider(RES_FILE, (IObjRef**)&pSysResProvider);
			bLoaded = pSysResProvider->Init((LPARAM)_T("uires\\theme_sys_res"), 0);
			SASSERT(bLoaded);
			bLoaded = theApp->LoadSystemNamedResource(pSysResProvider);
			SASSERT(bLoaded);
[!endif]
		}
[!else]
		//从DLL加载系统资源
		{
			HMODULE hModSysResource = LoadLibrary(SYS_NAMED_RESOURCE);
			if (hModSysResource)
			{
				CAutoRefPtr<IResProvider> sysResProvider;
				CreateResProvider(RES_PE, (IObjRef**)&sysResProvider);
				sysResProvider->Init((WPARAM)hModSysResource, 0);
				theApp->LoadSystemNamedResource(sysResProvider);
				FreeLibrary(hModSysResource);
			}
			else
			{
				SASSERT(0);
			}
		}
[!endif]
#endif
[!endif]

		CAutoRefPtr<IResProvider>   pResProvider;

[!if CHECKBOX_RES_PACK_ONLAY_RELEASE]
#ifdef _DEBUG		
		//选择了仅在Release版本打包资源则在DEBUG下始终使用文件加载
		{
			CreateResProvider(RES_FILE, (IObjRef**)&pResProvider);
			bLoaded = pResProvider->Init((LPARAM)_T("uires"), 0);
			SASSERT(bLoaded);
		}
#else
		{
[!if ResLoaderType == 0]
			CreateResProvider(RES_PE, (IObjRef**)&pResProvider);
			bLoaded = pResProvider->Init((WPARAM)hInstance, 0);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 1]
			bLoaded = pComMgr->CreateResProvider_ZIP((IObjRef**)&pResProvider);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
			ZIPRES_PARAM param;
			param.ZipFile(pRenderFactory, _T("uires.zip"), "[!output ZIP_PSW]");
			bLoaded = pResProvider->Init((WPARAM)&param, 0);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 2]
			bLoaded = pComMgr->CreateResProvider_7ZIP((IObjRef**)&pResProvider);
			SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("resprovider_zip"));
			ZIP7RES_PARAM param;
			param.ZipFile(pRenderFactory, _T("uires.7z"), "[!output ZIP_PSW]");
			bLoaded = pResProvider->Init((WPARAM)&param, 0);
			SASSERT(bLoaded);
[!endif]
[!if ResLoaderType == 3]
			CreateResProvider(RES_FILE, (IObjRef**)&pResProvider);
			bLoaded = pResProvider->Init((LPARAM)_T("uires"), 0);
			SASSERT(bLoaded);
[!endif]
		}
#endif
[!endif]
		theApp->InitXmlNamedID(namedXmlID, ARRAYSIZE(namedXmlID), TRUE);
		theApp->AddResProvider(pResProvider);
[!if CHECKBOX_USE_LUA]
		//加载LUA脚本模块。
#if (defined(DLL_CORE) || defined(LIB_ALL)) && !defined(_WIN64)
		//加载LUA脚本模块，注意，脚本模块只有在SOUI内核是以DLL方式编译时才能使用。
		CAutoRefPtr<SOUI::IScriptFactory> pScriptLuaFactory;
		bLoaded = pComMgr->CreateScrpit_Lua((IObjRef**)&pScriptLuaFactory);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("scirpt_lua"));
		theApp->SetScriptFactory(pScriptLuaFactory);
#endif//DLL_CORE
[!endif]

[!if CHECKBOX_TRANSLATOR_SUPPORT]
		//加载多语言翻译模块。
		CAutoRefPtr<ITranslatorMgr> trans;
		bLoaded = pComMgr->CreateTranslator((IObjRef**)&trans);
		SASSERT_FMT(bLoaded, _T("load interface [%s] failed!"), _T("translator"));
		if (trans)
		{//加载语言翻译包
			theApp->SetTranslator(trans);
			pugi::xml_document xmlLang;
			if (theApp->LoadXmlDocment(xmlLang, _T("lang_cn"), _T("translator")))
			{
				CAutoRefPtr<ITranslator> langCN;
				trans->CreateTranslator(&langCN);
				langCN->Load(&xmlLang.child(L"language"), 1);//1=LD_XML
				trans->InstallTranslator(langCN);
			}
		}
[!endif]
[!if CHECKBOX_SHELLNOTIFYICON]
		theApp->RegisterWindowClass<SShellNotifyIcon>();
[!endif]
		// BLOCK: Run application
		{
			CMainDlg dlgMain;
			dlgMain.Create(GetActiveWindow());
			dlgMain.SendMessage(WM_INITDIALOG);
			dlgMain.CenterWindow(dlgMain.m_hWnd);
[!if CHECKBOX_MAXIMIZED]
			dlgMain.ShowWindow(SW_SHOWMAXIMIZED);
[!else]
			dlgMain.ShowWindow(SW_SHOWNORMAL);
[!endif]
			nRet = theApp->Run(dlgMain.m_hWnd);
		}

		delete theApp;
	}

	delete pComMgr;

	OleUninitialize();
	return nRet;
}
