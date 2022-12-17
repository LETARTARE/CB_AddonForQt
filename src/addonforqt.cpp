/*************************************************************
 * Name:      addonforqt.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2022-12-13
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */
#include <sdk.h> 	// Code::Blocks SDK
// for linux
#include <cbproject.h>
#include <projectmanager.h>
#include <configmanager.h>
// <-
#include <toolsmanager.h>
#include <cbeditor.h>
#include <editormanager.h>
#include <loggers.h>
// local files
#include "addonforqt.h"
#include "creater.h"
// not place change !
#include "defines.h"
//------------------------------------------------------------------------------
// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
	/**  ATTENTION :
	 *   name to register this plugin with 'Code::Blocks'
	 *   THIS NAME MUST BE IDENTICAL TO
	 *	 $(NAME) IN PROJECT CUSTOM VARIABLE
	 *   AND IN 'manifest.xml' :: <Plugin name="addonforQt">
  	 */
	wxString NamePlugin = "addonforQt";
	PluginRegistrant<AddOnForQt> reg(NamePlugin);
}

/// ----------------------------------------------------------------------------
/// menu entries constante identificator
///-----------------------------------------------------------------------------

/// menu bar
	const long idMbarQt						= wxNewId();

	const long idMbarFirstState				= wxNewId();
	const long idMbarStop	 				= wxNewId();
	//const long idMbarSetting				= wxNewId();

	const long idMbarBuildAddonsPlus 		= wxNewId();
	const long idMbarBuildAddons	    	= wxNewId();
	const long idMbarCleanAddons 	    	= wxNewId();
	const long idMbarRebuildAddonsPlus 		= wxNewId();
	const long idMbarRebuildAddons 			= wxNewId();
	// file
    const long idMbarBuildOneAddin			= wxNewId();
    const long idMbarCleanOneAddin 			= wxNewId();
	// workspace
	const long idMbarBuildWSAddons 			= wxNewId();
	//const long idMbarCleanWSAddons 	    = wxNewId();
	const long idMbarRebuildWSAddons 		= wxNewId();

/// events handling
BEGIN_EVENT_TABLE(AddOnForQt, cbPlugin)
	// add any events you want to handle here
	EVT_MENU (idMbarStop, AddOnForQt::OnMenuStop)

	EVT_MENU (idMbarBuildAddonsPlus, AddOnForQt::OnMenuComplements)
	EVT_MENU (idMbarBuildAddons, AddOnForQt::OnMenuComplements)
//	EVT_MENU (idMbarCleanAddons, AddOnForQt::OnMenuComplements)
	EVT_MENU (idMbarRebuildAddonsPlus, AddOnForQt::OnMenuComplements)
	EVT_MENU (idMbarRebuildAddons, AddOnForQt::OnMenuComplements)

	EVT_MENU (idMbarBuildOneAddin, AddOnForQt::OnMenuComplements)
	EVT_MENU (idMbarCleanOneAddin, AddOnForQt::OnMenuComplements)

	EVT_MENU (idMbarBuildWSAddons, AddOnForQt::OnMenuComplements)
	//EVT_MENU (idCleanWSAddons, AddOnForQt::OnMenuComplements)
	EVT_MENU (idMbarRebuildWSAddons, AddOnForQt::OnMenuComplements)

END_EVENT_TABLE()

///-----------------------------------------------------------------------------
///	Load ressource 'AddOnForQt.zip'
///
AddOnForQt::AddOnForQt()
{
	wxString zip = m_NamePlugin + ".zip";
	if(!Manager::LoadResource(zip))
		NotifyMissingFile(zip);
}
///-----------------------------------------------------------------------------
/// Create handlers event and creating the pre-builders
/// N° 0 : the first called ( who qupports '_print(...)
///
/// Called by 'PluginManager' actually
///
void AddOnForQt::OnAttach()
{
_printD("=> Begin AddOnForQt::OnAttach()");
//1- managers
	m_pM = Manager::Get();
	if (!m_pM)  return ;
	// projects manager
	m_pMprj = m_pM->GetProjectManager();
	if (!m_pMprj)  return ;

//2- plugin manually loaded
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorPluginLoaded =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnPluginLoaded);
	m_pM->RegisterEventSink(cbEVT_PLUGIN_INSTALLED, functorPluginLoaded);

//3- handle loading plugin complete ( comes before 'cbEVT_APP_START_SHUTDOWN' )
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorPluginLoadingComplete =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnPluginLoadingComplete);
	m_pM->RegisterEventSink(cbEVT_PLUGIN_LOADING_COMPLETE, functorPluginLoadingComplete);

//4- handle begin shutdown application
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorBeginShutdown =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnAppBeginShutDown);
	m_pM->RegisterEventSink(cbEVT_APP_START_SHUTDOWN, functorBeginShutdown);

//5- handle project activate
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorActivateProject =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnActivateProject);
	m_pM->RegisterEventSink(cbEVT_PROJECT_ACTIVATE, functorActivateProject);

//6- handle target selected
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorTargetSelected =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnActivateTarget);
	m_pM->RegisterEventSink(cbEVT_BUILDTARGET_SELECTED, functorTargetSelected);

//7- handle new project  (indicated by PECAN http://forums.codeblocks.org, 2017-12-18)
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorNewProject =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnNewProject);
	m_pM->RegisterEventSink(cbEVT_PROJECT_NEW, functorNewProject);

//8- before the first file was removed !
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorBeginFileRemoved =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnBeginFileRemoved);
	m_pM->RegisterEventSink(cbEVT_PROJECT_BEGIN_REMOVE_FILES, functorBeginFileRemoved);
//9- a file was removed !
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorFileRemoved =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnProjectFileRemoved);
	m_pM->RegisterEventSink(cbEVT_PROJECT_FILE_REMOVED, functorFileRemoved);
//10 - after the lastfile was removed !
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorEndFileRemoved =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnEndFileRemoved);
	m_pM->RegisterEventSink(cbEVT_PROJECT_END_REMOVE_FILES, functorEndFileRemoved);

//11- a current workspace is begin closing :
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorWorkspaceCLosed =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnWorkspaceClosed);
	m_pM->RegisterEventSink(cbEVT_WORKSPACE_CLOSING_BEGIN, functorWorkspaceCLosed);

//12- a new workspace is complete loading :
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorWorkspaceComplete =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnWorkspaceComplete);
	m_pM->RegisterEventSink(cbEVT_WORKSPACE_LOADING_COMPLETE, functorWorkspaceComplete);

//13- a target is renommed
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorRenameTarget =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnRenameProjectOrTarget);
	 m_pM->RegisterEventSink(cbEVT_BUILDTARGET_RENAMED, functorRenameTarget);

//14 editor save a file
	/// USED HERE for debug
//	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorSaveFileEditor =
//		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnSaveFileEditor);
	//m_pM->RegisterEventSink(cbEVT_EDITOR_SAVE, functorSaveFileEditor);

//15- construct a new log
	m_LogMan = m_pM->GetLogManager();
	if(m_LogMan)
    {
    // add 'Prebuild log'
        m_AddonLog = new TextCtrlLogger(FIX_LOG_FONT);
        m_LogPageIndex = m_LogMan->SetLog(m_AddonLog);
        m_LogMan->Slot(m_LogPageIndex).title = _("Addons");
        CodeBlocksLogEvent evtAdd1(cbEVT_ADD_LOG_WINDOW, m_AddonLog,
									m_LogMan->Slot(m_LogPageIndex).title);
        m_pM->ProcessEvent(evtAdd1);
    // memorize last log
        CodeBlocksLogEvent evtGetActive(cbEVT_GET_ACTIVE_LOG_WINDOW);
        m_pM->ProcessEvent(evtGetActive);
        m_Lastlog   = evtGetActive.logger;
        m_LastIndex = evtGetActive.logIndex;
    // display 'm_AddonLog'
        SwitchToLog();
    }

//19- construct the builder
	// construct a new 'm_pCreater', m_pProject == nullptr
	m_pCreater = new Creater(m_NamePlugin, m_LogPageIndex);
	if (m_pCreater)
	{
	// version
		Mes = m_pCreater->platForm();
		Mes += ", sdk => " + quoteNS(m_pCreater->GetVersionSDK());
		Mes += ",  " + quote(m_NamePlugin) + _("version") + " : " + VERSION_WXT;
		Mes += ", " + _("built on") + quote(m_pCreater->GetDateBuildPlugin()) + Lf;
		_printWarn(Mes);
    // icons from *.zip
        m_bmLogoQt    = LoadPNG(_T("logo.png"));
            if (!m_bmLogoQt.IsOk()) _printError(_("Error with 'm_bmLogoQt'"));
        m_bmBuild   = LoadPNG(_T("build.png"));
            if (!m_bmBuild.IsOk()) _printError(_("Error with 'm_bmBuild'"));
		//	m_bmBuildOff   = LoadPNG(_T("buildoff.png"));
		//        if (!m_bmBuildOff.IsOk()) _printError("Error with 'm_bmBuildOff'");
		m_bmBuildPlus   = LoadPNG(_T("build+.png"));
            if (!m_bmBuildPlus.IsOk()) _printError(_("Error with 'm_bmBuild'"));
		//	m_bmBuildPlusOff   = LoadPNG(_T("buildPlusoff.png"));
		//        if (!m_bmBuilPlusdOff.IsOk()) _printError("Error with 'm_bmBuildPlusOff'");
        m_bmReBuild = LoadPNG(_T("rebuild.png"));
            if (!m_bmReBuild.IsOk()) _printError(_("Error with 'm_bmReBuild'"));
		//	m_bmReBuildOff   = LoadPNG(_T("rebuildoff.png"));
		//        if (!m_bmReBuildOff.IsOk()) _printError("Error with 'm_bmReBuildOff'");
		m_bmReBuildPlus = LoadPNG(_T("rebuild+.png"));
            if (!m_bmReBuildPlus.IsOk()) _printError(_("Error with 'm_bmReBuild'"));
		//	m_bmReBuildPlusOff   = LoadPNG(_T("rebuild+off.png"));
		//        if (!m_bmReBuildPlusOff.IsOk()) _printError("Error with 'm_bmReBuildPlusOff'");
        m_bmStop    = LoadPNG(_T("stop.png"));
            if (!m_bmStop.IsOk()) _printError(_("Error with 'm_bmStop'"));
       // m_bmStopOff     = LoadPNG(_T("stopoff.png"));
        //    if (!m_bmStopOff.IsOk()) _printError("Error with 'm_bmStopOff'");
		m_bmSetting     = LoadPNG(_T("options.png"));
            if (!m_bmSetting.IsOk()) _printError(_("Error with 'm_bmSetting'"));
		//m_bmSettingOff     = LoadPNG(_T("optionsoff.png"));
          //  if (!m_bmSettingOff.IsOk()) _printError("Error with 'm_bmSettingOff'");
	}
	else
	{
		Mes = _("Error to create") + Space + "m_pCreater" ;
		Mes += Lf + _("The plugin is not operational") + " !!"; _printError(Mes);
	///  release plugin
	    OnRelease(false);
	}


	Mes.Clear();

_printD("	=> End AddOnForQt::OnAttach()");
}
/// ----------------------------------------------------------------------------
///	menu 'Qt' in 'AddonsQt' menubar
/// N°1 : "AFTER OnAttach(...)"
//
///	Called by :
///		1. Code::Bock "menuBar ->..." at begin
///     2. Loading manual : 2 fois
///
void AddOnForQt::BuildMenu(wxMenuBar* _menuBar)
{
_printD("=> Begin 'AddOnForQt::BuildMenu(...)'");

	if (!IsAttached() || ! _menuBar )	return;

/// call two
	m_pMenuBar = _menuBar;
// locate "&Project" menu and insert after it
	wxString label = _("&Project");
	m_menuposX = _menuBar->FindMenu(label);
//_print("m_menuposX = " + strInt(m_menuposX));
	if (m_menuposX == wxNOT_FOUND )
	{
		_printError( quote(label) + _("cannot be found") + " !!!");
		return ;
	}
	// just after ...
	++m_menuposX ; m_buildFinded = true;
	/// value m_menuposX in [0...]
	///	When 'CB' is started, this method is called twice
	/// 1- the 1st time 										: m_menuposX = -1 !!!
	/// 2- the 2nd time after the activation of the 1st project	: m_menuposX =  5

	/// manual loading of the extension 		: m_menuposX = 5
	///	to the activation of the extension		: m_menuposX = 5

// construct all items Qt in menu bar
	buildMenuBarQt();

_printD("	<= End 'AddOnForQt::BuildMenu(...)'");
}
///-----------------------------------------------------------------------------
/// Construct items of "For a 'Qt project"
///
///	Called by :
///     1. AddOnForQt::BuildMenu(wxMenuBar* _menuBar)
///		2. AddOnForQt::OnActivateProject(m_pMenuBar);
/// Calls to : none
///
void AddOnForQt::buildMenuBarQt()
{
_printD("=> Begin 'AddOnForQt::buildMenuBarQt(...)'");
/// m_menuposX == -1 => called by first 'OnActivateProject(...)'

	if (!m_pMenuBar) return ;

///-------------------------------------------------------------------
/// for test just after '&Project'
	m_menuposX = 5;  m_buildFinded = true;
///-------------------------------------------------------------------
	wxString label;
    wxMenu * submenu = new wxMenu;
/// 1-
	label	= _("Build add-ons + Build") ;
	Mes 	= _("Build all the add-on files then 'Build' the project");
	m_pItem1 = new wxMenuItem(submenu, idMbarBuildAddonsPlus, label, Mes);
	// wx-316
	//m_pItem1->SetBitmaps(m_bmBuildPlus, m_bmBuildPlusOff);
	// wx-315
	m_pItem1->SetBitmap(m_bmBuildPlus);
	submenu->Append(m_pItem1);
/// 2-
	label 	= _("Rebuild add-ons + Build") ;
	Mes 	=  _("Rebuild all the add-on files then 'Build' the project");
	m_pItem2 = new wxMenuItem(submenu, idMbarRebuildAddonsPlus, label, Mes);
	//m_pItem2->SetBitmaps(m_bmReBuildPlus, m_bmReBuildPlusOff);
	m_pItem2->SetBitmap(m_bmReBuildPlus);
	submenu->Append(m_pItem2);
/// 3-
	label 	= _("Build add-ons only") ;
	Mes 	=  _("Build all add-on files");
	m_pItem3 = new wxMenuItem(submenu, idMbarBuildAddons, label, Mes);
	//m_pItem3->SetBitmaps(m_bmBuild, m_bmBuildOff);
	m_pItem3->SetBitmap(m_bmBuild);
	submenu->Append(m_pItem3);
/// 4-
	label 	= _("Rebuild add-ons only") ;
    Mes 	=  _("Rebuild all the add-on files");
	m_pItem4 = new wxMenuItem(submenu, idMbarRebuildAddons, label, Mes);
	//m_pItem4->SetBitmaps(m_bmReBuild, m_bmReBuildOff);
	m_pItem4->SetBitmap(m_bmReBuild);
	submenu->Append(m_pItem4);
/// 5-
    label 	= _("StopQt");
    Mes 	= _("Stop process AddOn for Qt");
	m_pItem5 = new wxMenuItem(submenu, idMbarStop, label, Mes);
	//m_pItem5->SetBitmaps( m_bmStop, m_bmStopOff);
	m_pItem5->SetBitmap( m_bmStop);
	submenu->Append(m_pItem5);
/* WAITING ...
/// 6-
    label 	= _("Setting");
    Mes 	= _("Setting for Qt");
	m_pItem6 = new wxMenuItem(submenu, idMbarSetting, label, Mes);
	//m_pItem6->SetBitmaps( m_bmSetting, m_bmSettingOff );
	m_pItem6->SetBitmap( m_bmSetting);
	submenu->Append(m_pItem6);
*/
/// the menu 'AddonsQt' in menu bar
	label 	= _("&AddonsQt") ;
	Mes 	=  _("For build projects that use 'Qt' libraries");
	m_pMenuBar->Insert(size_t(m_menuposX) , submenu, label);

/// enable or not
	//enableMenuBarQt(true);
_printD("    <= End 'AddOnForQt::buildMenuBarQt(...)'");
}

///-----------------------------------------------------------------------------
/// Enable the all items of "AddonsQt" => For a 'Qt project"
///
///	Called by :
///		1. buildMenuBarQt():1,
///		2. OnMenuComplements(wxCommandEvent& _event):2,
///		3. OnActivateProject(CodeBlocksEvent& _event):1,
///		4. OnActivateTarget(CodeBlocksEvent& _event):1,
///
void AddOnForQt::enableMenuBarQt(const bool _enable /* = true */)
{
_printD("=> Begin 'AddOnForQt::enableMenuBarQt(" + strBool( _enable) + ")' ");

	if(m_pMenuBar && m_buildFinded)
	{
        m_isBothQt = m_isQtProject && m_isQtActiveTarget;
		cbPlugin * pOtherRunning = m_pMprj->GetIsRunning();
		bool otherRunning = pOtherRunning &&  pOtherRunning != this;
		bool enable = !(m_isRunning || otherRunning) ;
		bool valid = _enable && enable && m_isBothQt ;
//_printWarn(" enableAllMenuBarQt::valid => " + strBool(valid) );
	// menu bar
		m_pItem1->Enable(valid);
		m_pItem2->Enable(valid);
		m_pItem3->Enable(valid);
		m_pItem4->Enable(valid);
		// stop
		m_pItem5->Enable(!valid && m_isBothQt);
	/// waiting
	//	m_pItem6->Enable(valid);
	}

_printD("	<= End 'AddOnForQt::enableMenuBarQt(...)'");
}

///-----------------------------------------------------------------------------
/// Construct popup menu on 'Project', 'File', 'WorkSpace' for Qt
/// callback for context menu items clicking
///
///	Called by : 'ProjectsManager' on right clic mouse for popup menu
/// Call to :
///		1. Pre::filenameOk(wxString & _namefile):1,
///
void AddOnForQt::BuildModuleMenu(const ModuleType _type, wxMenu* _menu, const FileTreeData* _data)
{
_printD("=> Begin 'AddOnForQt::BuildModuleMenu(...)'");

	if (!IsAttached())     return;
/// Only on 'ProjectManager'
	if (_type != mtProjectManager || ! _menu) 	return ;
/* Debug
/// is it always the same ?
/// when running a 'Build addon' and you stop => its a new popup !
	if(m_pPopupQt)
	{
		if(m_pPopupQt != _menu) _printError("'m_pPopupQt' is not the same !!");
		else 	_printWarn("'m_pPopupQt' is the same ...");
	}
*/
	m_pPopupQt = _menu;
/// others running
	cbPlugin * pOtherRunning = m_pMprj->GetIsRunning();
	bool otherRunning = pOtherRunning &&  pOtherRunning != this;
	bool enable = !(m_isRunning || otherRunning) ;
//_printError("otherRunning = " + strBool(otherRunning)  );
    m_isBothQt = m_isQtProject && m_isQtActiveTarget;
    wxString label;
/// popup menu in empty space in ProjectManager
	if (!_data || _data->GetKind() == FileTreeData::ftdkUndefined)
    {
//_printError"BuildModuleMenu::_data is NOT here !" );
	/// worspace or empty space for Qt project ??
//_print("BuildModuleMenu:workspace => m_isBothQt: "+ strBool(m_isBothQt) );
		if (m_isBothQt)
		{
		/// menu entries
			_menu->AppendSeparator();
			wxMenu *submenu = new wxMenu;
            wxMenuItem* item ;
            label =  _("For this 'Qt' workspace") + " =>" ;
                Mes = _("For 'Qt' workspace");
            item = _menu->AppendSubMenu(submenu, label, Mes);
			if (item)
			{
            // no bitmap !!
                item->SetBitmap(m_bmLogoQt);
				label = _("Build add-ons workspace");
                    Mes =  _("All projects in worskpace");
                    item = submenu->Append(idMbarBuildWSAddons , label, Mes);
                    item->SetBitmap(m_bmBuildPlus);
				//
				label = _("Rebuil add-ons workspace") ;
                    Mes = _("All rebuild projects");
                    item = submenu->Append(idMbarRebuildWSAddons , label, Mes);
                    item->SetBitmap(m_bmReBuildPlus);
				//
			//	Mes = _"Clean add-ons workspace" ;
			//	submenu->Append(idCleanWSAddons, Mes, _"All remove the complement files!"));
			/// enable popup menu
				submenu->Enable(idMbarBuildWSAddons, enable);
                submenu->Enable(idMbarRebuildWSAddons, enable);
			}
		}
    }
	else
	// on project
	if (_data)
	{
//_printWarn("BuildModuleMenu::_data is here !" );
		/// project* exist
		cbProject * prj = _data->GetProject();
		if (prj)
		{
		/// verify that active project is same as clicked project
			wxString nameprj = prj->GetTitle();
// _print("nameprj:" + quote(nameprj));
            if ( nameprj.Matches(m_activeNameProject) )
            {
            /// change to 'Addons' pane
                SwitchToLog();
            /// popup menu on a project
                if (_data->GetKind() == FileTreeData::ftdkProject)
                {
    //_printWarn("FileTreeData::ftdkProject ...");
				///-------------------------------------------------------------
                /// Windows alone
                    if (m_pCreater->m_Win)
                    {
                    /// test illegal characters
                        int  ncar = m_pCreater->filenameOk(nameprj);
    //_print("ncar:" + strInt(ncar) );
                        if (ncar)
                        {
                           // wxString title = quote("addonforQt ") + VERSION_WXT ;
                            wxString title = quote(m_NamePlugin) + VERSION_WXT ;
                            Mes = _("The project name ") + quote(nameprj);
                            Mes += Lf + _("contains illegal character(s)");
                            Mes += Lf + _("Do you want to replace all with") + " \"\" ?";
                            int choice = cbMessageBox(Mes, title, wxICON_QUESTION | wxYES_NO);
                            if (choice == wxID_YES)
                            {
                                Mes = m_activeNameProject + " -> " + nameprj + " : ";
                                Mes += strInt(ncar) + " " + _("illegalcharacters  changed");
                                _print(Mes);
                            /// name changed
    //_print("nameprj:" + quote(nameprj));

                                m_pMprj->GetActiveProject()->SetTitle(nameprj);
                                 m_pMprj->GetUI().RebuildTree() ;
                            }
                        } // ncar
                    } /// m_Win
					///---------------------------------------------------------
    //_printWarn("BuildModuleMenu:data => m_isBothQt: "+ strBool(m_isBothQt) );
                /// test if project and target uses 'Qt'
                    if (m_isBothQt)
                    //if (m_isQtProject && m_isQtActiveTarget)
                    {
                        // max position
                        size_t posiY = _menu->GetMenuItemCount();
                        // "Build" id );
                        int idBuild = _menu->FindItem( _("&Build") );
                        /// and "&Build" ??
                        if (idBuild != wxNOT_FOUND )
                        {
                        // children ?
                            _menu->FindChildItem(idBuild, &posiY);
            //_print("pos 'Build option...' = " + strInt(posi) );
                        }
                    // menu insertion : a separator
                        _menu->InsertSeparator(posiY);
                    /// menu entries an item
                        wxMenuItem* item;
					// 0- first for 'Qt'
                        label =  _("For this 'Qt' project") + " =>";
                            item = new wxMenuItem(_menu,  idMbarQt, label);
                            if (item)
                            {
                                item->SetBitmap(m_bmLogoQt);
                                _menu->Insert(posiY,  item);
                            }
					// 1-
                        label = _("Build add-ons + 'Build'") ;
                            Mes = _("All build the complement files and compile all files");
                            item = new wxMenuItem(_menu, idMbarBuildAddonsPlus, label, Mes);
                            if (item)
                            {
                                item->SetBitmap(m_bmBuildPlus);
                                _menu->Insert(++posiY, item);
                            }
					// 2-
                        label = _("Rebuild add-ons + 'Build'") ;
                            Mes = _("All rebuild the complement files and compile all files");
                            item = new wxMenuItem(_menu, idMbarRebuildAddonsPlus, label, Mes);
                            if (item)
                            {
                                item->SetBitmap(m_bmReBuildPlus);
                                _menu->Insert(++posiY, item);
                            }
					// 3-
                        label = _("Build add-ons only") ;
                            Mes = _("All build the complement files");
                            item = new wxMenuItem(_menu, idMbarBuildAddons, label, Mes);
                            if (item)
                            {
                            	item->SetBitmap(m_bmBuild);
                                _menu->Insert(++posiY, item);
                            }
					// 4-
                        label = _("Rebuild add-ons only") ;
                            Mes =  _("All rebuild the complement files");
                            item = new wxMenuItem(_menu, idMbarRebuildAddons, label, Mes);
                            if (item)
                            {
                            	item->SetBitmap(m_bmReBuild);
                                _menu->Insert(++posiY, item);
                            }

					// 5-
                        label = _("Stop build add-ons") ;
                            Mes = _("Stop build complement files in project");
                            item = new wxMenuItem(_menu, idMbarStop,  label, Mes);
                            if (item)
                            {
                                item->SetBitmap(m_bmStop);
                                _menu->Insert(++posiY, item);
                            }
					// ?-
                        //label = _("Clean add-ons only") ;
                            // Mes =  _("All remove the complement files in project");
                            //item = new wxMenuItem(_menu, idCleanAddons ,  label, Mes);
                            //_menu->Insert(++posiY, item );
                        //
                        _menu->InsertSeparator(posiY+1);
                    /// enable items module
                        _menu->Enable(idMbarQt, true);
                        _menu->Enable(idMbarBuildAddonsPlus, enable);
                        _menu->Enable(idMbarRebuildAddonsPlus, enable);
                        _menu->Enable(idMbarBuildAddons,enable);
                        _menu->Enable(idMbarRebuildAddons, enable);
                        /// waiting
                        //_menu->Enable(idCleanAddons,enable);
                        _menu->Enable(idMbarStop, !enable);
                    } // m_isBothQt
                }
                else
            /// popup menu on a file
                if (_data->GetKind() == FileTreeData::ftdkFile )
                {
    //_print("BuildModuleMenu:ftdkfile => m_isBothQt: "+ strBool(m_isBothQt) );
                    if (m_isBothQt)
                    {
                    /// change to 'Addons' log
                        SwitchToLog();
                    /// parse file
                        wxString file = _data->GetProjectFile()->relativeFilename;
    //_print("*** file to parse :" + quote(file) );

                    /// verify file registered to active target
                        wxString nametarget;
                        if (m_pCreater->isRegisteredToTarget(file, nametarget) )
                        {
    //_print("it's a creator file : => " + quote(m_fileCreator) );
                        /// menu entries
                            _menu->AppendSeparator();
                            wxMenuItem * item;
                            wxMenu *submenu = new wxMenu;
                            label =  _("For a creator file Qt") + " =>" ;
                                Mes = _("Create a complement file");
                                item = _menu->AppendSubMenu(submenu, label, Mes);
                                if (item)
                                {
                                    item->SetBitmap(m_bmLogoQt);
                                    label = _("Build add-in file");
                                        Mes = _("Build the complement file in project");
                                        submenu->Append(idMbarBuildOneAddin , label, Mes);
                                    /// enable menu
                                    submenu->Enable(idMbarBuildOneAddin, enable);
                                }
                            /// waiting
                            //Mes = _("Clean add-in file") ;
                            //_menu->Append(idCleanOneAddin, Mes, _("Clean the complement file"));
                            //_menu->Enable(idCleanOneAddin, !(m_isRunning || otherRunning));
                            _menu->AppendSeparator();
                        }
                        // is not Elegible()
                        else
                        {
                            Mes = Tab + " => " + quote(file) ;
                            Mes +=  _("it's not a creator file of") + quote(nametarget);
                            _printWarn(Mes);
                        }

                    } // m_isBothQT
                    // enable if Qt
					enableMenusQt(m_isBothQt);
                } // ftdkFile
			} //same nameprj
		}// prj
	} // _data

_printD("	<= End 'AddOnForQt::BuildModuleMenu(...)'");
}
///-----------------------------------------------------------------------------
/// Enable the all popup items of "=> For a 'Qt project"
///
///	Called by :
///	1. AddOnForQt::enableMenusQt(const bool _enable):1,
///
void AddOnForQt::enablePopupQt(const bool _enable)
{
_printD("=> Begin 'AddOnForQt::enablePopupQt(" + strBool( _enable) + ")' ");

    cbPlugin * pOtherRunning =  m_pMprj->GetIsRunning();
	bool otherRunning = pOtherRunning &&  pOtherRunning != this ;
    bool valid = _enable && !(m_isRunning || otherRunning);
	if (m_pPopupQt)
	{
		m_pPopupQt->Enable(idMbarStop, !valid);
    }
_printD("	<= End 'AddOnForQt::enablePopupQt(...)");
}

///-----------------------------------------------------------------------------
/// Enable all menus for a 'Qt project"
///
///	Called by :
///		1.
///		2. OnMenuComplements(wxCommandEvent& _event):2,
///		3. OnActivateProject(CodeBlocksEvent& _event):1,
///		4. OnActivateTarget(CodeBlocksEvent& _event):1,
/// Calls to:
///
void AddOnForQt::enableMenusQt(const bool _enable)
 {
_printD("=> Begin 'AddOnForQt::enableMenusQt(" + strBool( _enable) + ")' ");
	if (m_pMenuBar)
		enableMenuBarQt(_enable);

_printD("    <= End 'AddOnForQt::enabledMenusQt(...)'");
 }

///-----------------------------------------------------------------------------
/// Dispatch the command
///
///	Called by : 'ProjectsManager' by clicked menu item
/// Call to :
///		1. AddOnForQt::OnComplements(CodeBlocksEvent& _event):1,
///
void AddOnForQt::OnMenuComplements(wxCommandEvent& _event)
{
_printD("=> Begin 'AddOnForQt::OnMenuComplements(...)'  with 'checked' :" + strBool(_event.IsChecked()));

	if (! _event.IsChecked()) 	return;
/// caller id verifying
	int id = _event.GetId();
	bool good = id == idMbarBuildAddons || id == idMbarBuildAddonsPlus
				|| id == idMbarRebuildAddons || id == idMbarRebuildAddonsPlus
				|| id == idMbarBuildWSAddons || id == idMbarRebuildWSAddons
				;
	if (!good) 	return;

/// --------------------------------------------------
/// when click on "Build" or "Clean" : file is loss !!
/// --------------------------------------------------
// read file name
	wxString filename = _event.GetString();
/// debug
//_print("id =" + strInt(id) );
//_print("filename =" + quote(filename) );
//_print("m_fileCreator =" + quote(m_fileCreator) );
//_print("check => " + quote(strBool(checked)) );
///
/// only if target uses 'Qt'
	if (! m_isQtActiveTarget ) 	return;

	bool ok = false;
/// disable all menus item
	enableMenusQt(false);
/// running
	m_isRunning = true;
// display 'm_AddonLog'
	SwitchToLog();
//_print("m_activeNameTarget :" + quote(m_activeNameTarget) );
/// search menu label ...
	cbFutureBuild domake = fbNone;
	filename = wxEmptyString ;
	wxString menuItemLabel = wxEmptyString ;
	bool addOnly = false;
// fbBuild : 11, all files
	if (id == idMbarBuildAddonsPlus)
	{
		domake = fbBuild;
		menuItemLabel = _("/&Build/Build");
	}
// fbBuild : 11, all files, only
	else
	if (id == idMbarBuildAddons)
	{
		domake = fbBuild;
		menuItemLabel = _("/&Build/Build");
		addOnly = true;
	}
	else
// fbBuild : 11, one file
	if (id == idMbarBuildOneAddin)
	{
		domake = fbBuild;
		menuItemLabel = _("/&Build/Build");
		filename = m_fileCreator;
	}
	else
// fbClean = 12, all files, only
	if (id == idMbarCleanAddons)
	{
		domake = fbClean;
		menuItemLabel = _("/&Build/Clean");
		addOnly = true;
	}
	else
// fbClean = 12, one file, only
	if (id == idMbarCleanOneAddin)
	{
//_print("Clean One => 12");
		domake = fbBuild;
		menuItemLabel = _("/&Build/Clean");
		filename = m_fileCreator;
		addOnly = true;
	}
	else
// fbRebuild = 13,
	if (id == idMbarRebuildAddonsPlus)
	{
		domake = fbRebuild;
		menuItemLabel = _("/&Build/Rebuild");
	}
	else
// fbRebuild = 13, only
	if (id == idMbarRebuildAddons)
	{
		domake = fbRebuild;
		menuItemLabel = _("/&Build/Rebuild");
		addOnly = true;
	}
	else
// fbWorkspaceBuild = 14
	if (id == idMbarBuildWSAddons)
	{
		domake = fbWorkspaceBuild;
		menuItemLabel = _("/&Build/Build workspace");
	}
	else
/*
// fbWorkspaceClean = 15
	if (id == idMbarCleanWSAddons)
	{
		domake = fbWorkspaceClean;
		menuItemLabel = _("/&Build/Clean workspace");
	}
	else
*/
// fbWorkspaceReBuild = 16
	if (id == idMbarRebuildWSAddons)
	{
		domake = fbWorkspaceReBuild;
		menuItemLabel = _("/&Build/Rebuild workspace");
	}
/// build the complements Qt
	ok = doComplements(domake);
	if (ok)
	{
	/// menu item Qt + menu CB
		if (! addOnly)
		{
		/// call compiler
			if (!menuItemLabel.IsEmpty() )
			{
//	_print("menuItemLabel called =>" + quote(menuItemLabel) );
				bool good = m_pCreater->CallMenu(m_pMenuBar, menuItemLabel);
				if (! good)
				{
					Mes = _("The menu") + quote(menuItemLabel) ;
					Mes += _("is not finded") + "!";
					_printError(Mes);
				}
			}
			else
			{
				Mes = _("The menu item provided was empty," );
				Mes += _("Calling 'CallMenu(...)' is not possible") +  " !!" ;
				_printError(Mes);
				_event.Skip(); return;
			}
		}
		else
		{
		// view building
			SwitchToLog();
		}
	}
	else
	{
		SwitchToLog();
		if (m_pCreater->getAbort())
		{
			Mes = _("You had avorted building processus") + " !!!";
			_printWarn(Mes);
		}
		else
		{
			Mes = _("Call to 'AddOnForQt::doComplements(" + strInt(domake) + ")' ") ;
			Mes += _("is failed") + " !!!";
			_printError(Mes);
		}
	}
/// all menus (menuBar) enabled
	enableMenusQt(true);

	_event.Skip();

_printD("	<= End AddOnForQt::OnMenuComplements(...)" );
}

///-----------------------------------------------------------------------------
/// Build all (or one) complement files for Qt
///
/// Called by :
///		1. popup : 'AddOnForQt::OnMenuComplements(wxCommandEvent& _event)'
///
/// Calls to :
///		-# Pre::detectQtProject(prj, withreport):1,
///		-# Creater::buildAllFiles(prj, Ws, Rebuild):1,
///		-# Creater::buildOneFile(prj, file):1,
///		-# Pre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):1,
///
bool AddOnForQt::doComplements(const cbFutureBuild&  _domake)
{
_printD("=> Begin 'AddOnForQt::doComplements(" + strInt(_domake) + ")'" );

	if (!m_pProject) 	return false;

	if (!m_isQtProject) return false;

	if (_domake < fbNone) return false;

	if (!m_isQtActiveTarget )	return false;

	bool ok = false;

/// DEBUG
//* ********************************************************
//  m_pCreater->beginDuration("doComplements(...)");
//* ********************************************************
	//Mes = m_NamePlugin + "::doComplements(cbFutureBuild  _domake, ) -> ";
// just project created ?
	wxString nametarget = m_pProject->GetActiveBuildTarget();
	if (nametarget.IsEmpty() )
	{
		Mes += _("no target supplied") + " !!"; _printError(Mes);
		return false;
	}
	else
	{
        Mes =  Lf + domakeToStr(_domake) + quote("::" + nametarget) ; _print(Mes);
	}

/// verify if complements exists already
	// wirh report
	//ok = m_pCreater->detectComplementsOnDisk(m_pProject, nametarget, WITH_REPORT);
	// withless report
	ok = m_pCreater->detectComplementsOnDisk(m_pProject, nametarget, NO_REPORT);
//_printError("ok = " + strBool(ok));

	// it' now an old project with complement files on disk
	m_isNewProject = false;
	// init
	m_removingIsFirst = false;

	wxString file = m_fileCreator;
//_print("fileCreator =" + quote(file) );
	bool BuildOneFile = !file.IsEmpty()
		,BuildAllAddons = !BuildOneFile && (_domake > fbNone)
		;
/// debug
//Mes = Lf + "Call 'doComplements' ...";
//Mes += " _domake = " + strInt(_domake); _printWarn(Mes);

//Mes =  ", BuildOneFile = " ;
//Mes += BuildOneFile ? "true ->" + quote(file) : "false";
//Mes += ", BuildAllAddons = " + strBool(BuildAllAddons); _printWarn(Mes);
/// <==
	if (!BuildOneFile && !BuildAllAddons)
	{
		return false;
	}

	cbFutureBuild FBuild = _domake;
// calculate future action
	bool Build	 	= FBuild == fbBuild || FBuild == fbRebuild ; //|| FBuild == fbWorkspaceBuild ;
	bool Clean 	    = FBuild == fbClean || FBuild == fbRebuild ; //|| fbWorkspaceClean;
	bool Rebuild 	= FBuild == fbRebuild  ;//|| FBuild == fbWorkspaceReBuild;
	bool WsBuild 	= FBuild == fbWorkspaceBuild || FBuild == fbWorkspaceReBuild;
/*
/// Debug
Mes = " FBuild = " + strBool(FBuild);
Mes += ", Build(11) = " + strBool(Build);
Mes += ", Clean(12) = " + strBool(Clean);
Mes += ", Rebuild(13) = " + strBool(Rebuild);
_printWarn(Mes);
*/
// to 'Addons' Log
	SwitchToLog();

///********************************
/// Creater all complement files
///********************************
	if (BuildAllAddons)
	{
	// log clear
		Mes =Tab +  _("Wait a little bit") ;Mes += " ...";
		_printWarn(Mes);
	/// build
		if (Build)
		{
			if (WsBuild)
			{
				Mes =  _("Build workspace") ;
				Mes += " ..." + _("NOT YET ACHIEVED") ;
				_printError(Mes);
			}
		// real target
			m_pProject->SetActiveBuildTarget(nametarget);
			ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(nametarget);
			// verify Qt target
			ok = m_pCreater->isGoodTargetQt(pBuildTarget);
		/// if a virtual target !! => '!pBuildTarget' => ok = false
			if (ok)
			{
				m_isRunning = true;
			// prebuil complements
				if (Rebuild)
					ok = m_pCreater->buildAllFiles(m_pProject, WsBuild, ALLBUILD);
				// only preBuild when date complements < date creator
				else
					ok = m_pCreater->buildAllFiles(m_pProject, WsBuild, NO_ALLBUILD);
			    if (! ok)
			    {
			    	if (!m_pCreater->getAbort())
					{
						Mes = Tab + "m_pCreater->buildAllFiles(...) => ";
						Mes += _("Error 'PreBuild'") + " !!!";   _printError(Mes);
					}
			    }
			    m_isRunning = false;
			}
			else
			{
			    Mes = _("It's not a valid Qt target") ;
			    Mes += ", " + _("check if the target is not virtual or only a command");
			    Mes += " !!!";
			    _printError(Mes);
			}
			Build = false;
		} /// end Build
		else
		if (Clean)
		{
			Mes =  _("Clean all files") ;
			Mes += " ..." + _("NOT YET ACHIEVED") ;
			_printError(Mes);
		/// cleanning current file
			//ok = m_pCreater->cleanOneFile(m_pProject, file);
			ok = true;
			Clean = false;
		}
	}
	else
///********************************
/// Build one file
///********************************
	if (BuildOneFile)
	{
//_print("One file to compile ...");
		m_isRunning = true;

		if (Clean)
		{
			Mes =  _("Clean OneFile") ;
			Mes += " ..." + _("NOT YET ACHIEVED") ;
			_printError(Mes);
		/// cleanning current file
			//ok = m_pCreater->cleanOneFile(m_pProject, file);
			ok = true;
			Clean = false;
		}

		if (Build)
		{
		/// preCompile active file
			ok = m_pCreater->buildOneFile(m_pProject, file);
			if (ok)
			{
                Build = false;
            }
		}
	}
	m_isRunning = false ;
	m_fileCreator = wxEmptyString;

// switch last build log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_Lastlog);
    m_pM->ProcessEvent(evtSwitch);

/// DEBUG
//* ******************************************************
	//m_pCreater->endDuration("doComplements(...)");
//* ******************************************************
	Mes.Clear();

_printD("	=> End 'AddOnForQt::doComplements(...)' => " + strBool(ok) );

	return ok;
}

///-----------------------------------------------------------------------------
/// Execute 'Stop' item menu bar
///
/// Called by :
/// 	1. AddOnForQt::idMbarStop:1,

/// Call to
///		1. AddOnForQt::SwitchToLog():1,
///  	2. Creater::setAbort(bool _enabled):1,
///
void AddOnForQt::OnMenuStop(wxCommandEvent& _event)
{
_printD("=> Begin 'AddOnForQt::OnMenuStop(...)' with 'checked':" + strBool(_event.IsChecked()));

// origin
	wxInt64 id = _event.GetId();
	if (id == idMbarStop)
	{
	// display log
		SwitchToLog();
		m_pCreater->setAbort(true);
	}
_printD("	=> End 'AddOnForQt::OnMenuStop(...)'");
}

///-----------------------------------------------------------------------------
/// Loading manually plugin
///		- called ONLY when the plugin is manually loaded
/// N°  : n'apparait pas ??
///	Called by :
///		1. cbEVT_PLUGIN_INSTALLED
///
void AddOnForQt::OnPluginLoaded(CodeBlocksEvent& _event)
{
_printD("=> Begin 'AddOnForQt::OnPluginLoaded((...)'");

/// with parsing project
    m_noParsing = false;
/// with visible messages
	m_WithMessage = true;
/// init done
	m_initDone = true;
/// debug
/*
	Mes = "AddOnForQt::OnPluginLoaded(...) -> ";
	Mes +=  " 'AddOnForQt' is manually loaded";
	Mes += Space + "-> m_noParsing = " + strBool(m_noParsing) ; _printError(Mes) ;
*/
/// the active project
    m_pProject =  m_pMprj->GetActiveProject();
    if (m_pProject)
    {
/// debug
/// Mes = "Notify activate project ..."; _printWarn(Mes);
    // pseudo event
        m_pseudoEvent = true;
        CodeBlocksEvent evt(cbEVT_PROJECT_ACTIVATE, 0, m_pProject, 0, this);
        OnActivateProject(evt);
    }

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();

_printD("    <= End 'AddOnForQt::OnPluginLoaded((...)'");
}

///-----------------------------------------------------------------------------
/// The new workspace is completely loaded
/// N°2 : after "'BuildMenu(...)"
///	Called by :
///		1. cbEVT_WORKSPACE_LOADING_COMPLETE
///
void AddOnForQt::OnPluginLoadingComplete(CodeBlocksEvent& _event)
{
_printD("=> Begin 'AddOnForQt::OnPluginLoadingComplete((...)'");

/// with parsing project
    m_noParsing = false;
///
	m_WithMessage = true;
/// using plugins
	m_initDone = true;
/// debug
/*
	Mes = "AddOnForQt::OnPluginLoadingComplete(...) -> ";
	Mes += _("all plugins are loaded");
	Mes += Space + "m_initDone = " + strBool(m_initDone) ; _printWarn(Mes) ;
*/
	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
_printD("    <= End 'AddOnForQt::OnPluginLoadingComplete((...)'");
}

///-----------------------------------------------------------------------------
/// origin : ProjectManager
///		- the current workspace is begin closed
/// N°3 : "after OnPluginComplete(...)"
/// Called by :
///		1. event 'cbEVT_WORKSPACE_CLOSING_BEGIN'
///
void AddOnForQt::OnWorkspaceClosed(CodeBlocksEvent& _event)
{
_printD("=> Begin 'AddOnForQt::OnWorkspaceClosed((...)'");
/// stop parsing project
    m_noParsing = true;

	m_WithMessage = true;
/// debug
/*
	Mes = "AddOnForQt::OnWorkspaceClosed(...) -> ";
	Mes +=  " 'Current Workspace' is begin closed";
	Mes += Space + "-> m_noParsing = " + strBool(m_noParsing) ; _printError(Mes) ;
*/
	Mes.Clear();
/// The event processing system continues searching
    _event.Skip();
_printD("    <= End 'AddOnForQt::OnWorkspaceClosed((...)'");
}

///-----------------------------------------------------------------------------
/// origin : ProjectManager
/// N°6 :  AFTER 'ActivateProject(...)' !!
///	prevent that the workspace is complete loading
///
/// Called by :
///		1. event 'cbEVT_WORKSPACE_LOADONG_COMPLETE'
///
void AddOnForQt::OnWorkspaceComplete(CodeBlocksEvent& _event)
{
_printD("=> Begin 'AddOnForQt::OnWorkspaceComplete(...)'");

/// with parsing project
    m_noParsing = false;
///
	m_WithMessage = true;
/// debug
/*
	Mes = "AddOnForQt::OnWorkspaceComplete(...) -> ";
	Mes +=  " 'Workspace' is completely loaded";
	Mes += Space + "-> m_noParsing = " + strBool(m_noParsing) ; _printError(Mes) ;
*/
	Mes.Clear();
/// The event processing system continues searching
    _event.Skip();
_printD("    <= End 'AddOnForQt::OnWorkspaceComplete(...)'");
}
///-----------------------------------------------------------------------------
///	Called by :
/// Call to :
///
bool AddOnForQt::IsRunning() const
{
    return m_isRunning ;
}
///-----------------------------------------------------------------------------
/// Invalid the messages to 'Prebuild log'
///
/// Called by :
///		1. event 'cbEVT_APP_START_SHUTDOWN'
///
void AddOnForQt::OnAppBeginShutDown(CodeBlocksEvent& _event)
{
_printD("=> Begin 'AddOnForQt::OnAppBeginShutDown(...)'");
	m_WithMessage = false;

/// stop project parsing
    m_noParsing = true;

/// using plugins
	m_initDone = false;

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
}

///-----------------------------------------------------------------------------
///	Delete the pre-builders and do de-initialization for plugin : it's a destructor
///
/// Called by : 'PluginManager' actually
///
void AddOnForQt::OnRelease(bool _appShutDown)
{
/// !! No _print here !!
//1- delete builder"
	if (m_pCreater)
	{
		delete m_pCreater;
		m_pCreater = nullptr;
	}
//3-  delete log
	if(m_LogMan)
    {
        if(m_AddonLog)
        {
            CodeBlocksLogEvent evt(cbEVT_REMOVE_LOG_WINDOW, m_AddonLog);
            m_pM->ProcessEvent(evt);
        }
    }
    m_AddonLog 	= nullptr;
    m_pMenuBar 	= nullptr;
    m_pAddonsQtMenu = nullptr;

//3- do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...
    if(!_appShutDown)
		m_pM->RemoveAllEventSinksFor(this);
}

///-----------------------------------------------------------------------------
/// Append text to log
///
/// Called by :
///		-# all _printxxx(wxString)
///
void AddOnForQt::AppendToLog(const wxString& _Text, Logger::level _lv)
{
_printD("=> Begin AddOnForQt::AppendToLog(...)");
    if(m_AddonLog && m_WithMessage)
    {
        CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_AddonLog);
        m_pM->ProcessEvent(evtSwitch);
        m_AddonLog->Append(_Text, _lv);
    }
}
///-----------------------------------------------------------------------------
/// Switch to a log
///
/// Called by :
///		1. AddOnForQt::OnAttach():1,
///		2. AddOnForQt::OnComplements(CodeBlocksEvent& _event):1,
///		3. AddOnForQt:: OnProjectFileRemoved(CodeBlocksEvent& _event):1,
///
void AddOnForQt::SwitchToLog(int _indexLog)
{
_printD("=> Begin AddOnForQt::SwitchToLog(...)");
// display a log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, _indexLog);
	m_pM->ProcessEvent(evtSwitch);
}
void AddOnForQt::SwitchToLog()
{
_printD("=> Begin AddOnForQt::SwitchToLog(...)");
// display a log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_AddonLog);
	m_pM->ProcessEvent(evtSwitch);
}

///-----------------------------------------------------------------------------
/// Activate a project
/// N°5
///		called BEFORE a 'cbEVT_PROJECT_NEW' !!
///     called AFTER 'cbEVT_WORKSPACE_LOADONG_COMPLETE' !!
///
/// Called by :
///		1. event 'cbEVT_PROJECT_ACTIVATE'
///
/// Calls to :
///		1. Pre::detectQtProject(cbProject * _pProject, bool _report):1,
///		2. Pre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):1,
///
void AddOnForQt::OnActivateProject(CodeBlocksEvent& _event)
{
_printD("=> Begin 'AddOnForQt::OnActivateProject(...)" );

/// enabled menu Qt
	if (!m_buildFinded)
		buildMenuBarQt();
//_printError("call to false");
    enableMenuBarQt(false);
/// current workspace is begin closed or new workspace is completely loaded ?
	if (m_noParsing)
	{
/// at CB startup  => m_noParsing = true
_printD("m_noParsing = true");
		//_event.Skip() ; return;
	}

//_print("m_noParsing is false");
/// plugins are loaded ?
	if (!m_initDone)
	{
//_print("m_initDone is false");
		_event.Skip() ; return;
	}
//_print("m_initDone is true");
// wait for message validation
	if (!m_WithMessage)
	{
//_print("m_WithMessage is false");
		_event.Skip(!m_pseudoEvent);
		m_pseudoEvent = false;
		return;
	}
//_print("m_WithMessage is true");
// the active project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied") + " !!";
		_printError(Mes);
		_event.Skip(!m_pseudoEvent);
		m_pseudoEvent = false;
		return;
	}

// actualize the project here
	m_pProject = prj;
	m_activeNameProject = m_pProject->GetTitle();
	m_activeNameTarget = m_pProject->GetActiveBuildTarget();
//_print("m_activeNameProject : " + m_activeNameProject);
//_print("m_activeNameTarget : " + m_activeNameTarget);

////_printD("OnActivateProject::m_activeNameProject = " + quote(m_activeNameProject + "::" + m_activeNameTarget ) );
/// DEBUG
//* *********************************************************
//	m_pCreater->beginDuration("OnActivateProject(...)");
//* *********************************************************
// detect Qt project ... with report : feed 'qtpre::m_pProject'
	m_isQtProject = m_pCreater->detectQtProject(m_pProject, WITH_REPORT);
//Mes = "m_qtproject = " + strBool(m_isQtProject); _printWarn(Mes);
	// advice
	Mes = _("The project") +  quote(m_pProject->GetTitle());
	if (m_isQtProject)
	{
		Mes += _("has at least one target using Qt libraries");
		Mes += "...";
	}
	else
	{
		Mes += _("is NOT a Qt project") ;
		Mes += " !!";
	}
	_printWarn(Mes);
	if (!m_isQtProject)
	{
    //_printError("call to false");
        enableMenuBarQt(false); //enableToolBarQt(false);
		_event.Skip(!m_pseudoEvent);
		m_pseudoEvent = false ;

		return;
	}

/// here : project has a Qt target
	// virtual target ?
	m_isQtActiveTarget = m_pCreater->detectQtTarget(m_activeNameTarget);
///
	m_isBothQt = m_isQtProject && m_isQtActiveTarget;
	// enable menubar items
//_print("OnActivateProject(...)::m_isBothQt = " + strBool(m_isBothQt) );
	enableMenuBarQt(m_isBothQt);

//Mes = "AddOnForQt::OnActivateProject() : m_isQtActiveTarget = " + strBool(m_isQtActiveTarget); _print(Mes);
/// search if complements exists already on disk ?
		// ok = true => files Qt exists on disk
	bool ok = m_pCreater->detectComplementsOnDisk(m_pProject, m_activeNameTarget, NO_REPORT);
	if (!ok)
    {
        /// ...
    }
	m_removingIsFirst = true;

/// The event processing system continues searching if not a pseudo event
	_event.Skip(!m_pseudoEvent);
	m_pseudoEvent = false ;

/// DEBUG
//* *******************************************************
//	m_pCreater->endDuration("OnActivateProject(...)");
//* *******************************************************
	Mes.Clear();

_printD("	<= End 'AddOnForQt::OnActivateProject(...)");
}

///-----------------------------------------------------------------------------
/// Activate a target :
/// N°4 : the fist target of all projects !!
///		called before a project is opened  !!
///		called after a project is closed !!
///
/// Called by :
///		1. event 'cbEVT_BUILDTARGET_SELECTED'
///
/// Calls to :
///		1. adviceTypeTarget(nametarget):1,
///
void AddOnForQt::OnActivateTarget(CodeBlocksEvent& _event)
{
_printD("=> Begin 'AddOnForQt::OnActivateTarget(...)");

/// enabled menu Qt
//_printError("call to false");
    enableMenuBarQt(false);
/// if stop parsing project
    if (m_noParsing)
    {
/// at CB startup  => m_noParsing = true
_printD("m_noParsing = true");
		_event.Skip() ; return;
	}

/// At startup, this method is called before any project is active !!
	if (!m_pProject)
	{
//_printD("m_pProject = nullptr");
		_event.Skip(); return;
	}

/// a Qt target can exist in a non-Qt project!
// not a Qt current project
	if (!m_isQtProject)
	{
//_printD("m_isQtProject =" + strBool(m_isQtProject) );
		_event.Skip(); return;
	}
/// Only for message validation
	if (!m_WithMessage)
	{
		_event.Skip(); return;
	}

/// the active project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += Space + _("no project supplied") + " !!"; _printError(Mes);
		_event.Skip(); return;
	}
_printD("prj is good ...");
/// is it possible ??
// it's not the current project !
	if ( m_pProject != prj)
	{
/// just for debug
		Mes = quote(prj->GetTitle()) + " : " ;
		Mes += _("event project is not the current project") + " =>";
		if (m_pProject) Mes +=  quote(m_pProject->GetTitle());
		_printWarn(Mes);
/// <=
		_event.Skip(); return;
	}

/// DEBUG
//* *********************************************************
//	m_pCreater->beginDuration("OnActivateTarget(...)");
//* *********************************************************

/// the active target
	wxString nametarget  =  _event.GetBuildTargetName() ;
	if (nametarget.IsEmpty() )
	{
	/// test if the project is open
		if (m_pMprj->IsProjectStillOpen(m_pProject))
		{
			Mes += Space + _("no target supplied") + " !!"; _printError(Mes);
		}
		_event.Skip();
		return;
	}
	// is a command target
	if (m_pCreater-> isCommandTarget(nametarget))
	{
		Mes =  Tab + quote("::" + nametarget);
		Mes += Tab + _("is a command target") + " !!" ; _printWarn(Mes);
		m_isQtProject = false ; m_isQtActiveTarget = false; m_isBothQt = false;
		_event.Skip(); return;
	}
	m_activeNameTarget = nametarget;
//_print("OnActivateTarget => m_activeNameTarget = " + quote(m_activeNameProject + "::" + m_activeNameTarget ) );
	// all targets : real and virtual ...
	m_isQtActiveTarget = m_pCreater->detectQtTarget(nametarget, WITH_REPORT);
	m_isBothQt = m_isQtProject && m_isQtActiveTarget;
	// enable menubar items
	enableMenuBarQt(m_isBothQt);  //enableToolBarQt(m_isBothQt);

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* *********************************************************
//	m_pCreater->beginDuration("OnActivateTarget(...)");
//* *********************************************************
_printD("	<= End 'AddOnForQt::OnActivateTarget(...)");
}

///-----------------------------------------------------------------------------
/// Create a new project
///
/// Called by :
///		1. event 'cbEVT_PROJECT_NEW'
///
/// Calls to :
///		1. qtpre::detectQtProject(const wxString& _nametarget, cbProject * _pProject):1,
///		2. qtpre::detectQtTarget(const wxString& _nametarget, cbProject * _pProject):1,
///
void AddOnForQt::OnNewProject(CodeBlocksEvent& _event)
{
_printD("   => Begin AddOnForQt::OnNewProject(...)");
/// if stop parsing project
    if (m_noParsing)
    {
		_event.Skip() ; return;
	}
/// wait for message validation
	if (!m_initDone || !m_WithMessage)
	{
		_event.Skip(); return;
	}
/// debug
//Mes = NamePlugin + "::OnNewProject(CodeBlocksEvent& event) -> ";
//_printError(Mes);

// the new project
	cbProject *pProject = _event.GetProject();
	if(!pProject)
	{
		Mes += _("no project supplied") + " !!";
		_printError(Mes);
		_event.Skip(); return;
	}

// actualize the project
	m_pProject = pProject;
	m_isNewProject = true;
// just project created ?
	wxString nametarget = m_pProject->GetActiveBuildTarget();
	if (nametarget.IsEmpty() )
	{
		Mes += _("no target supplied") + " !!";
		_printError(Mes);
		_event.Skip(); return;
	}
//Mes = "project name" + quote(m_pProject->GetTitle()) ;
//Mes +=  Space + "nametarget =" + quote(nametarget); _printWarn(Mes);

// detect Qt project ... with report
	m_isQtProject = m_pCreater->detectQtProject(m_pProject, WITH_REPORT);
	// advice
	Mes = _("The New project") + quote(m_pProject->GetTitle());
	if (m_isQtProject)
	{
		Mes += _("has at least one target using Qt libraries") + "...";
	}
	else
	{
		Mes += _("is NOT a Qt project") + " !!";
	}
	_printWarn(Mes);

// detect Qt active Target ...
	m_isQtActiveTarget = m_pCreater->detectQtTarget(nametarget, NO_REPORT);
	// advice
	Mes = Tab + quote("::" + nametarget ) + Space ;
	Mes += _("is") + Space;
	if(!m_isQtActiveTarget)		Mes += _("NOT") + Space;
	Mes += _("a Qt target");
	_print(Mes);

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();

_printD("    <= End AddOnForQt::OnNewProject(...)");
}

///-----------------------------------------------------------------------------
/// Receive a renaming of a project or a target
///		at CB start we receive "cbEVT_PROJECT_RENAMED" for each registered project !!!
///		after all plugins are loaded and before the first project activation
///
/// Called by :
///		1. event 'cbEVT_PROJECT_RENAMED'
///		2. event 'cbEVT_BUILDTARGET_RENAMED'
///		3- before 'cbEVT_PROJECT_TARGETS_MODIFIED'
///
/// Calls to :
///		1. Pre::detectQtProject(const wxString& _nametarget, cbProject * _pProject):1,
///		2. Pre::detectQtTarget(const wxString& _nametarget, cbProject * _pProject):1,
///		3. Pre::detectComplementsOnDisk(cbProject * _pProject
///										,const wxString & _nametarget,  bool _report):2,
///
void AddOnForQt::OnRenameProjectOrTarget(CodeBlocksEvent& _event)
{
_printD("=> Begin AddOnForQt::OnRenameProjectOrTarget(...)");
/// if stop parsing project
    if (m_noParsing)
    {
		_event.Skip() ; return;
	}
	// no project is activated yet
	if (!m_pProject)
	{
/// Debug
//Mes = NamePlugin + "::OnRenameProjectOrTarget(CodeBlocksEvent& event) -> ";
//Mes +=  quote(_event.GetProject()->GetTitle()) + " : " ;
//Mes += " but 'm_pProject' is null !! " ; _printWarn(Mes);
/// <=
		_event.Skip(); return;
	}

// the project used
	cbProject *prj = _event.GetProject();
	if (!prj)
	{
		_event.Skip(); return; ;
	}

/// is it possible ?? it's just a new name
	// it's not the current project !
	if ( m_pProject != prj)
	{
/// debug
//	Mes += quote(prj->GetTitle()) + " : " ;
//		Mes += _("event project is not the current project !!");
//		_printError(Mes);
/// <==
		_event.Skip();
		return;
	}

// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(); return;
	}

// global
	Mes = wxEmptyString;
/// virtual target ??
	// the name target :  new name ?
	wxString nametarget = _event.GetBuildTargetName()
			,oldnametarget = _event.GetOldBuildTargetName()
			;
//Mes  = "!!! nameProject =" + quote(prj->GetTitle());
//Mes += ", newtarget =" + quote(nametarget);
//Mes += ", oldtarget =" + quote(oldnametarget); _printWarn(Mes);

	// it's a command target
	if (m_pCreater-> isCommandTarget(nametarget))
	{
		Mes =  Tab + quote( nametarget ) + _("is a command target") + " !!" ;
		_printWarn(Mes);
		_event.Skip();
		return;
	}

	bool okqt = false;
/// //////////////////////////
/// it's a new name project
/// //////////////////////////
	if (nametarget.IsEmpty() && oldnametarget.IsEmpty())
	{
		// detect Qt project ... with report
		m_isQtProject = m_pCreater->detectQtProject(m_pProject, WITH_REPORT);
//Mes = "==> m_isQtProject = " + strBool(m_isQtProject); _printWarn(Mes);
		if (!m_isQtProject)
		{
			_event.Skip(); return;
		}

		Mes = "=== " ;
		Mes += _("A new name project") + quote(m_pProject->GetTitle());
		_printWarn(Mes);
		Mes = quote(prj->GetTitle());
		if (m_isQtProject)
		{
		// advice
			Mes += _("has at least one target using Qt libraries") ;
			Mes += "...";
			_printWarn(Mes);
		// complements exists already ?
			m_pCreater->detectComplementsOnDisk(m_pProject, nametarget, !m_isNewProject);
		// init
			m_removingIsFirst = true;
		}
	}
/// ////////////////////////
/// it's a new name target
/// ////////////////////////
	else
	if (!nametarget.IsEmpty() )
	{
		m_pProject = prj;
	// verify target
		wxString actualtarget = m_pProject->GetActiveBuildTarget() ;
//Mes = "!! actualtarget = " + quote(actualtarget);
	// update m_pProject, ..., m_pBuildTarget
		m_pCreater->setProject(m_pProject);

	// new active target ?
		wxString activetarget = m_pProject->GetActiveBuildTarget() ;
//Mes += "!! activetarget =" + quote(activetarget);
//Mes += ", nametarget =" + quote(nametarget); _printWarn(Mes);

	// advice
		Mes = "=== " ;
		Mes += _("Old target name") + quote(oldnametarget)  ;
		Mes += "=> " ;
		Mes += _("New target name") + quote(nametarget);
		_printWarn(Mes);

		if (!activetarget.Matches(nametarget))
		{
		// detect Qt target ...
			okqt = m_pCreater->detectQtTarget(nametarget, NO_REPORT);
			Mes = quoteNS("::" + nametarget) + Space;
			Mes += _("is") + Space;
			if(!okqt)		Mes += _("NOT") + Space;
			Mes += _("a Qt target");
			_printWarn(Mes);
		}
	// updates of the new target :
		// new build target
		ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(nametarget);
		bool ok = m_pCreater->updateNewTargetName(pBuildTarget, oldnametarget);
		if (!ok)
        {
			/// ... TO FINISH
		}

	}
	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
_printD("    <= End AddOnForQt::OnRenameProjectOrTarget(...)");
}

///-----------------------------------------------------------------------------
/// Return a string for a 'cbFutureBuild'
///
/// Called by :
///     1. AddOnForQt::doComplements(const cbFutureBuild&  _domake)

wxString AddOnForQt::domakeToStr(const cbFutureBuild&  _domake)
{
    wxString str = _("None");
    switch (_domake)
    {
        case fbNone             : str = _("None"); break;
        case fbBuild            : str = _("Build"); break;
        case fbClean            : str = _("Clean"); break;
        case fbRebuild          : str = _("ReBuild"); break;
        case fbWorkspaceBuild   : str = _("WsBuild"); break;
        case fbWorkspaceClean   : str = _("WsClean"); break;
        case fbWorkspaceReBuild : str = _("WsReBuild"); break;
    }

    return str;
}

///-----------------------------------------------------------------------------
/// Before the first removed file of the project
///
/// Called by
///		1. event 'cbEVT_PROJECT_BEGIN_REMOVE_FILES'
///
void AddOnForQt::OnBeginFileRemoved(CodeBlocksEvent& _event)
{
_printD("=> Begin 'AddOnForQt::OnBeginFileRemoved(..)'" );

// if stop parsing project
    if (m_noParsing)
    {
		_event.Skip() ; return;
	}

	if (_event.GetProject() == m_pProject)
	{
	// the indicator
		m_removingIsFirst = true;
	}
	_event.Skip();

_printD("	<= End  'AddOnForQt::OnBeginFileRemoved(..)'" );
}

///-----------------------------------------------------------------------------
/// Unregister complement file for Qt
///	==> The file is already removed of the project
///
/// Called by
///		1. event 'cbEVT_PROJECT_FILE_REMOVED'
///
///		by project popup menu or menu
///		2. 'Remove file from project'
///		3. 'Remove files ...'
///		4. 'Remove  dir\\*'
///
///	Calls to
///		1. Pre::isComplementFile(const wxString & _file):1,
///		2. Creater::unregisterComplementFile(wxString & _file, bool _first):1,
///		2. Pre::isCreatorFile(const wxString & _file):1,
///		3. Creater::unregisterCreatorFile(wxString & _file, bool _first):1,
///
void AddOnForQt::OnProjectFileRemoved(CodeBlocksEvent& _event)
{
_printD("=> Begin 'AddOnForQt::OnProjectFileRemoved(..)'" );
// if stop parsing project
    if (m_noParsing)
    {
		_event.Skip() ; return;
	}
//	the project used
	cbProject *prj = _event.GetProject();
	if (!prj)
	{
		_event.Skip(); return; ;
	}
// it's not the current project !
	if ( m_pProject != prj)
	{
/// Debug
//	Mes = quote(prj->GetTitle()) + ": " ;
//	Mes += _("event project is not the current project") + " !!";  _printError(Mes);
/// <==
		_event.Skip();	return;
	}

// actualize the project
	m_pProject = prj;
	// call  setBuildTarget(...)
	m_pCreater->setProject(prj);

	// the removed file ( absolute path )
	wxString filename = _event.GetString();
	if (filename.IsEmpty())
	{
		_event.Skip(); return;
	}

// switch  'Prebuild' log
    SwitchToLog();
// only name less path
	wxString file = filename.AfterLast(cSlash) ;
//Mes = Lf + "filename = " + quote(filename);
//Mes += ",file =" + quote(file); _print(Mes);
	bool ok = false;
	Mes = wxEmptyString;
// it's a complement file ?
	m_iscomplement = m_pCreater->isComplementFile(file);
	if (m_iscomplement)
	{
		if (m_removingIsFirst)
		{
			Mes = "==> " + _("Complement file(s) was removed from C::B and disk");
			Mes += " ..." ;
			_printWarn(Mes);
			m_removingIsFirst = false;
		}
	// unregisterer one complement file
		//ok = m_pCreater->unregisterComplementFile(filename,  m_removingIsFirst) ;
		ok = m_pCreater->unregisterComplementFile(filename) ;
		if (!ok)
			Mes = Lf + _("Complement") + Space;
	}
	else
// it's a creator file ?
	if (m_pCreater->isCreatorFile(file))
	{
//Mes = "file =" + quote(file) + _("is a creator file"); _print(Mes);
	// unregisterer one creator file and its complement file
		//ok = m_pCreater->unregisterCreatorFile(filename, m_removingIsFirst) ;
		ok = m_pCreater->unregisterCreatorFile(filename) ;
		if (ok)
		{
		// delete the complement
			Mes =  _("The creator") + quote(filename)  + ": ";
			Mes += _("We must delete the complement file on disk") + "...";
			_printError(Mes);
			/// TO FINISH ...
		}
		else
			Mes = Lf + _("Creator") + Space;
	}
// it's an another file
	else
	{
		m_removingIsFirst = true;
		_event.Skip();	return;
	}

	if (!ok)
	{
		Mes += quote(filename) + _("file was NOT removed from project") + " !!!" ;
		_printError(Mes);
//Mes = Tab + quote(filename); _print(Mes);
	}

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();

_printD("	<= End 'AddOnForQt::OnProjectFileRemoved(...)'");
}
///-----------------------------------------------------------------------------
/// After the last removed file of the project
///
/// Called by
///		1. event 'cbEVT_PROJECT_END_REMOVE_FILES'
///
void AddOnForQt::OnEndFileRemoved(CodeBlocksEvent& _event)
{
_printD("=> Begin 'AddOnForQt::OnEndFileRemoved(..)'" );

/// if stop parsing project
    if (m_noParsing)
    {
		_event.Skip() ; return;
	}

	if (_event.GetProject() != m_pProject)
	{
		_event.Skip(); ; return;
	}

	if (m_iscomplement)
	{
	/// the indicator
		m_removingIsFirst = false;
	///
		Mes = "==> " + _("End of complement file(s) removed from 'C::B' and disk");
		Mes += " ..." ;
		_printWarn(Mes);
	}

	_event.Skip();

_printD("    <= End 'AddOnForQt::OnEndFileRemoved(..)'" );
}

///-----------------------------------------------------------------------------
/// Save a project
///
/// Called by :
///		1. event 'cbEVT_PROJECT_SAVE'
///
/// Calls to : none
///
void AddOnForQt::OnSaveProject(CodeBlocksEvent& _event)
{
/// if stop parsing project
    if (m_noParsing)
    {
		_event.Skip() ; return;
	}
// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(); return;
	}
/// debug
//Mes = m_NamePlugin + "::OnSaveProject(CodeBlocksEvent& event) -> ";
// _printWarn(Mes);
/// <==
// the project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied") ;
		Mes += " !!"; _printError(Mes);
		_event.Skip();
		return;
	}
// messages
	Mes += quote(prj->GetTitle()) + _("is saved") + " !!"; _printWarn(Mes);

// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* ***************************************************
//	m_pCreater->endDuration("OnSaveProject(...)");
//* ***************************************************
	Mes.Clear();
}
///-----------------------------------------------------------------------------
/// Editor save a file
///
/// Called by :
///		1. event 'cbEVT_EDITOR_SAVE'
///
/// Calls to : none
///
void AddOnForQt::OnSaveFileEditor(CodeBlocksEvent& _event)
{
// if stop parsing project
    if (m_noParsing)
    {
		_event.Skip() ; return;
	}
// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(); return;
	}
// not editor
	if (!_event.GetEditor())
	{
		Mes += _("no editor supplied") + " !!";
		_printError(Mes);
		_event.Skip(); return;
	}
/// Debug
//	Mes = NamePlugin + "::OnSaveFileEditor(CodeBlocksEvent& event) -> ";
//	_printWarn(Mes);
/// <=
// the editor
	EditorManager * em = EditorManager::Get();
	cbEditor * ed = em->GetBuiltinEditor(_event.GetEditor());
	if(!ed)
	{
		Mes += _("no editor supplied") + " !!";
		_printError(Mes);
		_event.Skip();
		return;
	}
	wxString filename = ed->GetFilename();

// messages
	Mes += quote(ed->GetTitle()) + _("is saved") ;
	//Mes +=  quote(filename);
	Mes += ": " + m_pCreater->date();
	_printWarn(Mes);

// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* ******************************************************
//	m_pCreater->endDuration("OnSaveFileEDitor(...)");
//* ******************************************************
	Mes.Clear();
}
///-----------------------------------------------------------------------------
///	Extract image ressource from 'AddOnForQt.zip'
///
///	Called by :
///		1. AddOnForQt::OnAttach():7,
///
wxBitmap AddOnForQt::LoadPNG(const wxString & _name)
{
    wxFileSystem filesystem;
    wxString filename = ConfigManager::ReadDataPath();
    filename += wxFILE_SEP_PATH + NamePlugin +  _T(".zip#zip:") + _name;
    wxFSFile *file = filesystem.OpenFile(filename, wxFS_READ | wxFS_SEEKABLE);
    wxString Mes ;
    if (file == nullptr)
    {
        Mes = _("File not found") + " :" + quote(filename) ;
    /// !! to 'Code::Blocks' log !!
        PrintError(Mes);

        return wxBitmap();
    }

    wxImage img;
    img.LoadFile(*file->GetStream(), wxBITMAP_TYPE_PNG);
    img.ConvertAlphaToMask();
    wxBitmap bmp(img);
/// TODO ...
	// const double scaleFactor = cbGetContentScaleFactor(*m_pToolbar);
	// wxString prefix = ...
	// wxBitmap bmp =cbLoadBitmapScaled(prefix + _name , wxBITMAP_TYPE_PNG, scaleFactor),
    if (!bmp.IsOk())
    {
        Mes = _("File not loaded correctly") + " :" + quote(filename) ;
        PrintError(Mes);
        // to 'Code::Blocks' log
        return wxBitmap();
    }

    return bmp;
}
///-----------------------------------------------------------------------------
