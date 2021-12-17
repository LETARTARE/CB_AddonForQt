/*************************************************************
 * Name:      addonforqt.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2021-12-17
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */
#include <sdk.h> 	// Code::Blocks SDK
// for linux
#include <cbproject.h>
#include <projectmanager.h>
// <-
#include <cbeditor.h>
#include <editormanager.h>
#include <loggers.h>
// local files
#include "addonforqt.h"
#include "build.h"
// not place change !
#include "defines.h"
//------------------------------------------------------------------------------
// Register the plugin with Code::Blocks.
// We are using an anonymous namespace so we don't litter the global one.
namespace
{
	/**  name to register this plugin with Code::Blocks
  	 */
  	wxString NamePlugin = "AddOnForQt";
	PluginRegistrant<AddOnForQt> reg(NamePlugin);
}

/// ----------------------------------------------------------------------------
/// menu entries constante identificator
///-----------------------------------------------------------------------------
	// projects
	const long AddOnForQt::idMenuQt						= wxNewId();

	const long AddOnForQt::idFirstState						= wxNewId();
	const long AddOnForQt::idActionStop	 				= wxNewId();

	const long AddOnForQt::idBuildAddons 				= wxNewId();
	const long AddOnForQt::idBuildAddonsAlone		= wxNewId();
	const long AddOnForQt::idCleanAddonsAlone 	= wxNewId();
	const long AddOnForQt::idRebuildAddons 			= wxNewId();
	const long AddOnForQt::idRebuildAddonsAlone 	= wxNewId();
	// file
    const long AddOnForQt::idBuildOneAddin			= wxNewId();
    const long AddOnForQt::idCleanOneAddin 			= wxNewId();
	// workspace
	const long AddOnForQt::idBuildWSAddons 			= wxNewId();
	const long AddOnForQt::idCleanWSAddons 		= wxNewId();
	const long AddOnForQt::idRebuildWSAddons 		= wxNewId();
	// for search item "Abort"
	const int AddOnForQt::idKillProcess  = XRCID("idCompilerMenuKillProcess");

/// events handling
BEGIN_EVENT_TABLE(AddOnForQt, cbPlugin)
    // add any events you want to handle here
	EVT_MENU (idBuildAddons, AddOnForQt::OnMenuComplements)
	EVT_MENU (idBuildAddonsAlone, AddOnForQt::OnMenuComplements)
	EVT_MENU (idCleanAddonsAlone, AddOnForQt::OnMenuComplements)
	EVT_MENU (idRebuildAddons, AddOnForQt::OnMenuComplements)
	EVT_MENU (idRebuildAddonsAlone, AddOnForQt::OnMenuComplements)

	EVT_MENU (idBuildOneAddin, AddOnForQt::OnMenuComplements)
	EVT_MENU (idCleanOneAddin, AddOnForQt::OnMenuComplements)

	EVT_MENU (idBuildWSAddons, AddOnForQt::OnMenuComplements)
	EVT_MENU (idCleanWSAddons, AddOnForQt::OnMenuComplements)
	EVT_MENU (idRebuildWSAddons, AddOnForQt::OnMenuComplements)

	EVT_UPDATE_UI(idBuildAddons, AddOnForQt::OnUpdateUI)
	EVT_UPDATE_UI(idCleanAddonsAlone, AddOnForQt::OnUpdateUI)
	EVT_UPDATE_UI(idRebuildAddons, AddOnForQt::OnUpdateUI)

	EVT_UPDATE_UI(idBuildOneAddin, AddOnForQt::OnUpdateUI)
	EVT_UPDATE_UI(idCleanOneAddin, AddOnForQt::OnUpdateUI)

	EVT_UPDATE_UI(idBuildWSAddons, AddOnForQt::OnUpdateUI)
	EVT_UPDATE_UI(idCleanWSAddons, AddOnForQt::OnUpdateUI)
	EVT_UPDATE_UI(idRebuildWSAddons, AddOnForQt::OnUpdateUI)

	EVT_UPDATE_UI(idKillProcess, AddOnForQt::OnUpdateUI)

END_EVENT_TABLE()

///-----------------------------------------------------------------------------
///	Load ressource 'AddOnForQt.zip'
///
AddOnForQt::AddOnForQt()
{
	wxString zip = NamePlugin + ".zip";
	if(!Manager::LoadResource(zip))
		NotifyMissingFile(zip);
}
///-----------------------------------------------------------------------------
/// Create handlers event and creating the pre-builders
///
void AddOnForQt::OnAttach()
{
// register event sinks
    Manager* pm = Manager::Get();

//1- plugin manually loaded
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorPluginLoaded =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnPluginLoaded);
	pm->RegisterEventSink(cbEVT_PLUGIN_INSTALLED, functorPluginLoaded);

//2- handle loading plugin complete ( comes before 'cbEVT_APP_START_SHUTDOWN' )
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorPluginLoadingComplete =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnPluginLoadingComplete);
	pm->RegisterEventSink(cbEVT_PLUGIN_LOADING_COMPLETE, functorPluginLoadingComplete);

//3- handle begin shutdown application
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorBeginShutdown =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnAppBeginShutDown);
	pm->RegisterEventSink(cbEVT_APP_START_SHUTDOWN, functorBeginShutdown);

//4- handle project activate
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorActivateProject =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnActivateProject);
	pm->RegisterEventSink(cbEVT_PROJECT_ACTIVATE, functorActivateProject);

//5- handle target selected
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorTargetSelected =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnActivateTarget);
	pm->RegisterEventSink(cbEVT_BUILDTARGET_SELECTED, functorTargetSelected);

//6- handle new project  (indicated by PECAN http://forums.codeblocks.org, 2017-12-18)
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorNewProject =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnNewProject);
	pm->RegisterEventSink(cbEVT_PROJECT_NEW, functorNewProject);

///7- handle build stop
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorAbortGen =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnAbortAdding);
	///pm->RegisterEventSink(cbEVT_COMPILER_FINISHED, functorAbortGen);

//8- before the first file was removed !
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorBeginFileRemoved =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnBeginFileRemoved);
	pm->RegisterEventSink(cbEVT_PROJECT_BEGIN_REMOVE_FILES, functorBeginFileRemoved);
//9- a file was removed !
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorFileRemoved =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnProjectFileRemoved);
	pm->RegisterEventSink(cbEVT_PROJECT_FILE_REMOVED, functorFileRemoved);
//10 - after the lastfile was removed !
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorEndFileRemoved =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnEndFileRemoved);
	pm->RegisterEventSink(cbEVT_PROJECT_END_REMOVE_FILES, functorEndFileRemoved);

//11- a current workspace is begin closing :
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorWorkspaceCLosed =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnWorkspaceClosed);
	pm->RegisterEventSink(cbEVT_WORKSPACE_CLOSING_BEGIN, functorWorkspaceCLosed);

//11.2- a new workspace is complete loading :
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorWorkspaceComplete =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnWorkspaceComplete);
	pm->RegisterEventSink(cbEVT_WORKSPACE_LOADING_COMPLETE, functorWorkspaceComplete);

///11- a project is renommed :  !! DISABLED !!
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorRenameProject =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnRenameProjectOrTarget);
	///pm->RegisterEventSink(cbEVT_PROJECT_RENAMED, functorRenameProject);

//12- a target is renommed
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorRenameTarget =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnRenameProjectOrTarget);
	 pm->RegisterEventSink(cbEVT_BUILDTARGET_RENAMED, functorRenameTarget);

//13 editor save a file
	/// USED HERE for debug
	cbEventFunctor<AddOnForQt, CodeBlocksEvent>* functorSaveFileEditor =
		new cbEventFunctor<AddOnForQt, CodeBlocksEvent>(this, &AddOnForQt::OnSaveFileEditor);
	//pm->RegisterEventSink(cbEVT_EDITOR_SAVE, functorSaveFileEditor);

//14- construct a new log
	m_LogMan = pm->GetLogManager();
	if(m_LogMan)
    {
    // add 'Prebuild log'
        m_AddonLog = new TextCtrlLogger(FIX_LOG_FONT);
        m_LogPageIndex = m_LogMan->SetLog(m_AddonLog);
        m_LogMan->Slot(m_LogPageIndex).title = _("Addons");
        CodeBlocksLogEvent evtAdd1(cbEVT_ADD_LOG_WINDOW, m_AddonLog,
									m_LogMan->Slot(m_LogPageIndex).title);
        pm->ProcessEvent(evtAdd1);
    // memorize last log
        CodeBlocksLogEvent evtGetActive(cbEVT_GET_ACTIVE_LOG_WINDOW);
        pm->ProcessEvent(evtGetActive);
        m_Lastlog   = evtGetActive.logger;
        m_LastIndex = evtGetActive.logIndex;
    // display 'm_AddonLog'
        SwitchToMyLog();
    }

//15- construct the builder
	// construct a new 'm_pBuild', m_pProject == nullptr
	m_pBuild = new Build(NamePlugin, m_LogPageIndex);
	if (m_pBuild)
	{
		Mes = m_pBuild->platForm();
		Mes += ", sdk => " + quoteNS(m_pBuild->GetVersionSDK());
		Mes += ",  'AddOnForQt' " + _(" version") + " : " + VERSION_WXT + Lf ;
		printWarn(Mes);
	}
	else
	{
		Mes = _("Error to create") + Space + "m_pBuild" ;
		Mes += Lf + _("The plugin is not operational") + " !!"; printError(Mes);
	///  release plugin
	    OnRelease(false);
	}

	Mes.Clear();
}
///-----------------------------------------------------------------------------
///	Called by :
/// Call to :
///
bool AddOnForQt::IsRunning() const
{
    return m_isRunning ;
}

/// ----------------------------------------------------------------------------
///	Insert menu 'Qt' in 'Build' menubar
///	Called by :
///		1. Code::Bock "menuBar->..." at begin
///
void AddOnForQt::BuildMenu(wxMenuBar* _menuBar)
{
printD("=> Begin 'AddOnForQt::BuildMenu(...)'");

	if (!IsAttached())		return;
printD("	is attached ...");
	if (! _menuBar)		return;
printD("	_menubar exist ...");
	m_menuBar = _menuBar;
	// locate "&Build" menu and item ' and insert after it
	//wxString label =  _("&Build");
	wxString label =  _("Build");
	int menupos = _menuBar->FindMenu( label);
/// menupos dans [0...]
/// au chargement de CB : menupos = -1 !!
/// au chargement manuel de l'extension : menupos = 5
/// à l'activation de l'extension :  menupos = 5
printD("'" + label + "' menupos :" + strInt(menupos) );
	m_buildFinded = menupos != wxNOT_FOUND;
	if (!m_buildFinded)	return;
	// get the buildMenu
	m_buildMenu = _menuBar->GetMenu(menupos);
	if (m_buildMenu)
	{
printD("	m_BuilMenu exist !");
		// find item 'Build'
		label = _("Build") ;
		int idBuild = m_buildMenu->FindItem( label);
		// max position
		size_t posi = m_buildMenu->GetMenuItemCount();
		if (idBuild != wxNOT_FOUND )
		{
			// child position
			m_buildMenu->FindChildItem(idBuild, &posi);
//	print("pos 'Build' = " + strInt(posi) );
		}
		wxMenuItem* item;
		label =  _("For Qt project") + " =>"; Mes =  _("For projects that use 'Qt' libraries");
		m_buildMenu->InsertCheckItem(posi, idMenuQt, label, Mes);
		m_buildMenu->Enable(idMenuQt, false); m_buildMenu->Check(idMenuQt, true);
		// item others
		label = _("Build add-ons only") ; Mes =  _("Build all add-on files");
		//m_buildMenu->Insert(++posi, idBuildAddonsAlone, Mes, _("All build the complement files");
		item = new wxMenuItem(m_buildMenu,  idBuildAddonsAlone, label, Mes);
			item->SetBitmap(wxBitmap("images/build.png", wxBITMAP_TYPE_PNG));
		m_buildMenu->Insert(++posi,  item);
		//
		label = _("Rebuild add-ons only") ; Mes =  _("Rebuild all add-on files");
		//m_buildMenu->Insert(++posi, idRebuildAddonsAlone, label, Mes);
		item = new wxMenuItem(m_buildMenu,  idRebuildAddonsAlone, label, Mes);
			item->SetBitmap(wxBitmap("images/rebuild.png", wxBITMAP_TYPE_PNG));
		m_buildMenu->Insert(++posi,  item);
		//
		// label = _("Clean add-ons only") ; Mes = _("All remove the complement files in project");
		//m_buildMenu->Insert(++posi, idCleanAddonsAlone, label, Mes);
		//
		label = _("Build add-ons + Build") ; Mes = _("Build all the add-on files then build the project");
		m_buildMenu->Insert(++posi, idBuildAddons, label, Mes);
		//
		label = _("Rebuild add-ons + Build") ; Mes =  _("Rebuild all the add-on files then build the project");
		m_buildMenu->Insert(++posi, idRebuildAddons, label, Mes);
		/// item others
		label = _("Stop add-ons") ; Mes =  _("Stop building add-on files");
			wxMenuItem* itemstop  =  new wxMenuItem(m_buildMenu, idActionStop, label, Mes);
			// mandatory : wxBITMAP_TYPE_PNG
			itemstop->SetBitmap(wxBitmap("images/stop.png", wxBITMAP_TYPE_PNG));
		m_buildMenu->Insert(++posi,  itemstop);
		//
		m_buildMenu->InsertSeparator(++posi);
	}

printD("	<= End 'AddOnForQt::BuildMenu(...)'");
}

///-----------------------------------------------------------------------------
/// Enable the menu items of '&Build->ForQt'
///
///	Called by :
///		1. BuildModuleMenu(...):2,
///		2.  OnMenuComplements(wxCommandEvent& _event):2,
///		3. OnActivateProject(CodeBlocksEvent& _event):1,
///		4. :OnActivateTarget(CodeBlocksEvent& _event):1,
///
void AddOnForQt::enableAllMenubarQt(const bool _valid)
{
printD("=> Begin 'AddOnForQt::enableAllMenubarQt(" + strBool( _valid) + ")' ");

	if(m_buildMenu)
	{
printD("	m_BuilMenu exist !");
		bool valid = _valid;
		if (m_isRunning) valid = false;
//print"enableAllMenubarQt => " + strBool(valid) );
		m_buildMenu->Enable(idBuildAddons,  valid);
		m_buildMenu->Enable(idRebuildAddons,  valid);
		//m_buildMenu->Enable(idCleanAddonsAlone,  valid);
		m_buildMenu->Enable(idBuildAddonsAlone,  valid);
		m_buildMenu->Enable(idRebuildAddonsAlone,  valid);
	}

//printError("m_buildMenu->IsEnabled => " + strBool(m_buildMenu->IsEnabled(idBuildAddons)) );

printD("	<= End 'AddOnForQt::enableAllMenubarQt(...)'");
}
/// ----------------------------------------------------------------------------
/// Enable the menu items of '&Build->ForQt'
///
///	Called by :
/// 	1. OnMenuComplements(wxCommandEvent& _event):1
///
void  AddOnForQt::enabledAllMenuBuildCB(const bool _valid)
{
printD("	=> Begin  AddOnForQt::enabledAllMenuBuildCB(" + strBool( _valid) + ")");

    if(m_buildMenu)
	{
	    // find item 'Build'
		int idBuild = m_buildMenu->FindItem( _("Build" ) );
		m_buildMenu->Enable(idBuild, _valid);
		// max position
		size_t posi = m_buildMenu->GetMenuItemCount();
		if (idBuild != wxNOT_FOUND )    m_buildMenu->FindChildItem(idBuild, &posi);
        // Build CB  item
        wxMenuItem* pnext;
        for (int u = 0; u <= 5 ; u++)
        {
            pnext = m_buildMenu->FindItemByPosition(posi + u);
            if (pnext)   pnext->Enable(_valid);
        }
	}

printD("		<= End  AddOnForQt::enabledAllMenuBuildCB(...)" );
}
///-------------------------------------------------------------------------
///
///	Call by :
///
void AddOnForQt::enableMenus(qtState _state)
{
printD("=> Begin 'AddOnForQt::enableAllMenus(" + strInt(_state) + ")'" );

	if ( m_buildMenu)
	{
	/// all disable
		m_buildMenu->Enable(idFirstState,  false);
		m_buildMenu->Enable(idActionStop,  false);

		m_buildMenu->Enable(idBuildAddons,  false);
		m_buildMenu->Enable(idRebuildAddons,  false);
		m_buildMenu->Enable(idBuildAddonsAlone,  false);
		m_buildMenu->Enable(idRebuildAddonsAlone,  false);
		m_buildMenu->Enable(idCleanAddonsAlone,  false);
		m_buildMenu->Enable(idBuildOneAddin,  false);
		m_buildMenu->Enable(idCleanOneAddin, false);
		m_buildMenu->Enable(idBuildWSAddons, false);
		m_buildMenu->Enable(idCleanWSAddons, false);
		m_buildMenu->Enable(idRebuildWSAddons, false);

	/// 'Wx' or 'Qt' only
		bool valid = m_isQtProject;
		m_buildMenu->Enable(idFirstState, valid);

	/// state
		m_State = _state;
	///
		switch (m_State)
		{
			case fbStop:
				m_buildMenu->Enable(idActionStop,  true);
				break;
			case fbWaitBuild:
				m_buildMenu->Enable(idBuildAddons,  true);
				//m_buildMenu->Enable(idBuildAddonsAlone, true);
				m_buildMenu->Enable(idBuildWSAddons,  true);
				break;
			case fbActionBuild:
				m_buildMenu->Enable(idActionStop,  true);
			//	m_buildMenu->Enable(idListAndExtractProject, false);
				break;
			case fbWaitClean:
			///	m_buildMenu->Enable(idCleanTemp,  true);
				break;
			case fbActionClean:
			//	m_buildMenu->Enable(idActionStop,  true);
				break;
			case fbWaitRebuild:
			//	m_buildMenu->Enable(idExtractProject,  true);
				break;
			case fbActionRebuild:
			//	m_buildMenu->Enable(idActionStop,  true);
				break;
			case fbWaitBuildWS:
			//	m_buildMenu->Enable(idListWorkSpace,  true);
				break;
			case fbActionBuildWS:
			//	m_buildMenu->Enable(idActionStop,  true);
				break;
			case fbWaitRebuildWS:
			//	m_buildMenu->Enable(idExtractWorkSpace,  true);
				break;
			case fbActionRebuildWS:
			//	m_buildMenu->Enable(idActionStop,  true);
				break;
    /// TODO ...
            case fbWait:

                break;
            case fbWaitCleanWS:

                break;
            case fbActionCleanWS:

                break;
		}
	}

printD("	<= End 'AddOnForQt::enableAllMenus(...)"  );
}


/// ----------------------------------------------------------------------------
///	Insert menu 'Qt' in toolbar
///	Called by : Code::Bock "toolBar->..." at begin
///
bool AddOnForQt::BuildToolBar(wxToolBar* _toolBar)
{
	if (!IsAttached() || !_toolBar)    return false;

	bool ok = false;
	m_toolBar = _toolBar;

    /// TODO ...

	return ok ;
}

///-----------------------------------------------------------------------------
/// Construct popup menu on 'Project', 'File', 'WorkSpace'
/// callback for context menu items clicking
///
///	Called by : 'ProjectsManager' on right clic mouse for popup menu
/// Call to :
///		1. Pre::filenameOk(wxString & _namefile):1,
///
void AddOnForQt::BuildModuleMenu(const ModuleType _type, wxMenu* _menu, const FileTreeData* _data)
{
printD("=> Begin 'AddOnForQt::BuildModuleMenu(...)'");

	if (!IsAttached())     return;

/// Only on 'ProjectManager'
	if (_type != mtProjectManager || ! _menu) 	return ;

/// others running
	cbPlugin * pOtherRunning = Manager::Get()->GetProjectManager()->GetIsRunning();
	bool otherRunning = pOtherRunning &&  pOtherRunning != this ;
//printError("otherRunning = " + strBool(otherRunning)  );

/// popup menu in empty space in ProjectManager
	if (!_data || _data->GetKind() == FileTreeData::ftdkUndefined)
    {
//printError"BuildModuleMenu::_data is NOT here !" );
	/// worspace or empty space
		if (m_isQtProject && m_isQtActiveTarget)
		{
		/// menu entries
			_menu->AppendSeparator();
			wxMenu *submenu = new wxMenu;
			// sub_menu
			Mes =  _("For 'Qt' workspace") + " =>" ;
			wxMenuItem* item = _menu->AppendSubMenu(submenu, Mes, _("For 'Qt' worlspace"));
			if (item)
			{
				// checked, disable
				item->Enable(false); item->Check(true);
				//
				Mes = _("Build add-ons workspace");
				submenu->Append(idBuildWSAddons , Mes, _("All projects in worskpace"));
				//
				Mes = _("Rebuil add-ons workspace") ;
				submenu->Append(idRebuildWSAddons , Mes, _("All rebuild projects") );
				//
			//	Mes = _"Clean add-ons workspace" ;
			//	submenu->Append(idCleanWSAddons, Mes, _"All remove the complement files!"));
			/// enable popup menu
				submenu->Enable(idBuildWSAddons, !(m_isRunning || otherRunning) );
				submenu->Enable(idRebuildWSAddons, !(m_isRunning || otherRunning));
			//	submenu->Enable(idCleanWSAddons, !(m_isRunning || otherRunning));
			}
		}
		else
		{
		/// disable menubar items
			enableAllMenubarQt(false);
		}
    }
	else
	if (_data)
	{
printD("BuildModuleMenu::_data is here !" );
		/// project* exist
		cbProject * prj = _data->GetProject();
		if (prj)
		{
		/// verify that active project is same as clicked project
			wxString nameprj = prj->GetTitle();
// print("nameprj:" + quote(nameprj));
			if ( ! nameprj.Matches(m_activeNameProject)  )	return ;

//printD("BuildModuleMenu::m_activeNameProject:" + quote(m_activeNameProject));
//printD("BuildModuleMenu::m_isQtProject = " + strBool(m_isQtProject) );
//printD("BuildModuleMenu::m_activeNameTarget = " + quote(m_activeNameTarget) );

		/// change to 'Addons' pane
			SwitchToMyLog();
		/// popup menu on a project
			if (_data->GetKind() == FileTreeData::ftdkProject)
			{
//print("FileTreeData::ftdkProject ...");
				if (m_pBuild->m_Win)
				{
				/// test illegal characters
					int  ncar = m_pBuild->filenameOk(nameprj);
//print("ncar:" + strInt(ncar) );
					if (ncar)
					{
						wxString title = quote("AddOnForQt ") + VERSION_WXT ;
						Mes = _("The project name ") + quote(nameprj);
						Mes += Lf + _(" contains illegal character(s) ");
						Mes += Lf + _("Do you want to replace all with \"\" ?");
						int choice = cbMessageBox(Mes, title, wxICON_QUESTION | wxYES_NO);
						if (choice == wxID_YES)
						{
							Mes = m_activeNameProject + " -> " + nameprj + " : ";
							Mes += strInt(ncar) + " " + _("illegalcharacters  changed");
							print(Mes);
						/// name changed
//print("nameprj:" + quote(nameprj));
							Manager::Get()->GetProjectManager()->GetActiveProject()->SetTitle(nameprj);
							Manager::Get()->GetProjectManager()->GetUI().RebuildTree() ;
						}
					} // ncar
				} // m_Win

			/// test if project and target uses 'Qt'
				if (m_isQtProject && m_isQtActiveTarget)
				{
//print("m_isQtProject:" + strBool(m_isQtProject) );
					// max position
					size_t posi = _menu->GetMenuItemCount();
					// "Build" id );
					int idBuild = _menu->FindItem( _("Build") );
					if (idBuild != wxNOT_FOUND )
					{
					// children ?
						_menu->FindChildItem(idBuild, &posi);
		//print("pos 'Build option...' = " + strInt(posi) );
					}
				/// menu entries an item
					wxMenuItem* item;
					// menu insertion
					wxString label =  _("For 'Qt' project") + " =>";
					_menu->InsertCheckItem(posi, idMenuQt, label);
						_menu->Enable(idMenuQt, false); _menu->Check(idMenuQt, true);
					// childs
					label = _("Build add-ons + Build") ; Mes = _("All build the complement files and compile all files");
						//_menu->Insert(++posi, idBuildAddons, label, Mes );
						item = new wxMenuItem(_menu, idBuildAddons,  label, Mes);
						item->SetBitmap(wxBitmap("images/build.png", wxBITMAP_TYPE_PNG));
						_menu->Insert(++posi, item);
					label = _("Rebuild add-ons + Build") ; Mes = _("All rebuild the complement files and compile all files");
						//_menu->Insert(++posi, idRebuildAddons, label, Mes );
						item = new wxMenuItem(_menu, idRebuildAddons, label, Mes);
						item->SetBitmap(wxBitmap("images/rebuild.png", wxBITMAP_TYPE_PNG));
						_menu->Insert(++posi, item);
					label = _("Build add-ons only") ;  Mes = _("All build the complement files");
						_menu->Insert(++posi, idBuildAddonsAlone, label, Mes);
					label = _("Rebuild add-ons only") ;  Mes =  _("All rebuild the complement files");
						_menu->Insert(++posi, idRebuildAddonsAlone, label, Mes);
					//label = _("Clean add-ons only") ;  Mes =
						//_menu->Insert(++posi, idCleanAddonsAlone, Mes, _("All remove the complement files in project"));
					label = _("Stop build add-ons") ;  Mes = _("Stop build complement files in project");
						//_menu->Insert(++posi, idActionStop, label, Mes);
						item = new wxMenuItem(_menu, idActionStop,  label, Mes);
						item->SetBitmap(wxBitmap("images/stop.png", wxBITMAP_TYPE_PNG));
						_menu->Insert(++posi, item);
					//
					_menu->InsertSeparator(posi+1);
				/// enable menu
					_menu->Enable(idBuildAddons,!(m_isRunning || otherRunning));
					_menu->Enable(idRebuildAddons, !(m_isRunning || otherRunning));
					_menu->Enable(idBuildAddonsAlone,!(m_isRunning || otherRunning));
					_menu->Enable(idRebuildAddonsAlone, !(m_isRunning || otherRunning));
					//_menu->Enable(idCleanAddonsAlone,!(m_isRunning || otherRunning));
				} // m_isQtProject
				else
				{
				/// disable menubar items
					enableAllMenubarQt(false);
				}
			}
			else
		/// popup menu on a file
			if (_data->GetKind() == FileTreeData::ftdkFile )
			{
				if (m_isQtActiveTarget)
				{
				/// change to 'Addons' log
					SwitchToMyLog();
				/// parse file
					wxString file = _data->GetProjectFile()->relativeFilename;
//print("*** file to parse :" + quote(file) );
				/// verify file extension
					if (m_pBuild->isElegible(file))
					{
					/// save file : when click on "Build" or "Clean" : file is loss !!
						m_fileCreator = file;
//print("it's a creator file : => " + quote(m_fileCreator) );
					/// menu entries
						_menu->AppendSeparator();
						wxMenu *submenu = new wxMenu;
						Mes =  _("For Qt") + " =>";
						// inactive menu
						_menu->Append(idMenuQt, Mes);
						_menu->Enable(idMenuQt, false); _menu->Check(idMenuQt, true);
						//
						Mes = _("Build add-in file") ;
						_menu->Append(idBuildOneAddin, Mes, _("Build the complement file in project"));
						//
						//Mes = _("Clean add-in file") ;
						//_menu->Append(idCleanOneAddin, Mes, _("Clean the complement file"));
					/// enable menu
						_menu->Enable(idBuildOneAddin, !(m_isRunning || otherRunning));
						//_menu->Enable(idCleanOneAddin, !(m_isRunning || otherRunning));
					} // is Elegible()
					else
					{
						Mes = Tab + " => " + quote(file)  +  _("it's not a creator file") ;
						printWarn(Mes);
					}
				} // m_isQtActiveTarget
			} // ftdkFile
		}// prj
	} // _data

printD("	<= End 'AddOnForQt::BuildModuleMenu(...)'");
}
///-----------------------------------------------------------------------------
/// Read the state of 'menuBar->Abort' item
///
///	Called by :
///		1.
/// Call to :
///
bool AddOnForQt::getAbort()
{
printD("=> Begin 'AddOnForQt::getAbort()' ");
	bool isEnabled = false;
/// get bar
	//m_menuBar = Manager::Get()->GetAppFrame()->GetMenuBar();
	// enable 'Abort'
	if (m_buildMenu)
	{
//print("m_bar exists : " +  m_mbar->GetLabel() );
		// find item 'Abort'
		Mes = _("Abort");
		int idAbort = m_buildMenu->FindItem( Mes );
		// max position
		size_t posi = m_buildMenu->GetMenuItemCount();
		if (idAbort != wxNOT_FOUND )
		{
			m_buildMenu->FindChildItem(idAbort, &posi);
//print("Abort position :" + strInt(posi) );
		}
		isEnabled = m_buildMenu->IsEnabled(posi);
	}

printD("	<= End 'AddOnForQt::getAbort()' => " + strBool(isEnabled) );
	return isEnabled;
}
///-----------------------------------------------------------------------------
/// Enabled the 'menuBar->Abort' item
///
///	Called by :
///		1.
/// Call to :
///
void AddOnForQt::setAbort(bool _enabled)
{
printD("=> Begin 'AddOnForQt::setAbort(" + strBool(_enabled) + ")'" );
/// get bar
	//m_menuBar = Manager::Get()->GetAppFrame()->GetMenuBar();
	// enable 'Abort'
	if (m_buildMenu)
	{
//print("m_bar exists : " +  m_mbar->GetLabel() );
		// find item 'Abort'
		Mes = _("Abort");
		int idAbort = m_buildMenu->FindItem( Mes );
		// max position
		size_t posi = m_buildMenu->GetMenuItemCount();
		if (idAbort != wxNOT_FOUND )
		{
			// child
			m_buildMenu->FindChildItem(idAbort, &posi);
		}
		m_buildMenu->Enable(posi, _enabled);

printD("Abort position :" + strInt(posi) );
printD("	<= End 'AddOnForQt::setAbort(...) =>" + strBool(m_buildMenu->IsEnabled(posi)) );
	}
}

///-----------------------------------------------------------------------------
/// Dispatch the command
///
///	Called by : 'ProjectsManager'
/// Call to :
///		1. AddOnForQt::OnComplements(CodeBlocksEvent& _event):1,
///
void AddOnForQt::OnMenuComplements(wxCommandEvent& _event)
{
printD("=> Begin 'AddOnForQt::OnMenuComplements(...)'  with 'checked' :" + strBool(_event.IsChecked()));

	bool checked = _event.IsChecked(), ok = false;
	m_isRunning = false;
	//
	int id = _event.GetId();
/// --------------------------------------------------
/// when click on "Build" or "Clean" : file is loss !!
/// --------------------------------------------------
	wxString filename = _event.GetString();
//print("id =" + strInt(id) );
//print("filename =" + quote(filename) );
//print("m_fileCreator =" + quote(m_fileCreator) );
//print("check => " + quote(strBool(checked)) );
/// 'checked'
	if (checked)
	{
		m_isRunning = true;
	// display 'm_AddonLog'
		SwitchToMyLog();

	/// only if target uses 'Qt'
		if (m_isQtActiveTarget )
		{
//print("m_activeNameTarget :" + quote(m_activeNameTarget) );
			cbFutureBuild domake = fbNone;
			filename = wxEmptyString ;
			wxString menuItem = wxEmptyString ;
			bool only = false;

		// fbBuild : 11, all files
			if (id == idBuildAddons)
			{
				domake = fbBuild;
				menuItem = _("/&Build/Build");
			}
		// fbBuild : 11, all files, only
			else
			if (id == idBuildAddonsAlone)
			{
				domake = fbBuild;
				menuItem = _("/&Build/Build");
				only = true;
			}
			else
		// fbBuild : 11, one file
			if (id == idBuildOneAddin)
			{
				domake = fbBuild;
				menuItem = _("/&Build/Build");
				filename = m_fileCreator;
			}
			else
		// fbClean = 12, all files, only
			if (id == idCleanAddonsAlone)
			{
				domake = fbClean;
				menuItem = _("/&Build/Clean");
				only = true;
			}
			else
		// fbClean = 12, one file, only
			if (id == idCleanOneAddin)
			{
//print("Clean One => 12");
				domake = fbBuild;
				menuItem = _("/&Build/Clean");
				filename = m_fileCreator;
				only = true;
			}
			else
		// fbRebuild = 13,
			if (id == idRebuildAddons)
			{
				domake = fbRebuild;
				menuItem = _("/&Build/Rebuild");
			}
			else
		// fbRebuild = 13, only
			if (id == idRebuildAddonsAlone)
			{
				domake = fbRebuild;
				menuItem = _("/&Build/Rebuild");
				only = true;
			}
			else
		// fbWorkspaceBuild = 14
			if (id == idBuildWSAddons)
			{
				domake = fbWorkspaceBuild;
				menuItem = _("/&Build/Build workspace");
			}
			else
		// fbWorkspaceClean = 15
			if (id == idCleanWSAddons)
			{
				domake = fbWorkspaceClean;
				menuItem = _("/&Build/Clean workspace");
			}
			else
		// fbWorkspaceReBuild = 16
			if (id == idRebuildWSAddons)
			{
				domake = fbWorkspaceReBuild;
				menuItem = _("/&Build/Rebuild workspace");
			}
//print("menuItem :" + quote(menuItem) );

		/// update mbar, tbar
		//	wxUpdateUIEvent uievent;
		//	OnUpdateUI(uievent);
		/// all menus enabled
			enableAllMenubarQt(false);
		/// build complements
			setAbort(true);
//print("abort :" + strBool(getAbort()) );

			ok = doComplements(domake);

			enabledAllMenuBuildCB(true);
			//setAbort(false);

/// ============================================================================
///         FOR TEST
			if (ok)
			{
			/// menu item Qt + menu CB
				if (! only)
				{
				/// call compiler
					if (!menuItem.IsEmpty() )
					{
//	print("menuItem called =>" + quote(menuItem) );
						int id = m_pBuild->CallMenu(menuItem);
						printD("menu id called :" + strInt(id) );
					}
					else
					{
						Mes = _("The menu item provided was empty," );
						Mes += _("Calling 'CallMenu(...)' is not possible") +  " !!" ;
						printError(Mes);
						_event.Skip(); return;
					}
				}
				else
				{
					SwitchToMyLog();
				}
			}
/// ============================================================================
			else
			{
			    SwitchToMyLog();
Mes = _("Call to 'AddOnForQt::doComplements(" + strInt(domake) + ")' ") ;
Mes += _("is failed") + " !!!";
				printError(Mes);
			}
		}
	}
/// all menus (menuBar) enabled
	enableAllMenubarQt(true);

	_event.Skip();

printD("	<= End AddOnForQt::OnMenuComplements(...)" );
}


///-----------------------------------------------------------------------------
/// Build all (or one) complement files for Qt
///
/// Called by :
///		1. popup : 'AddOnForQt::OnMenuComplements(wxCommandEvent& _event)'
///
///	'CompilerGCC::AddComplementFiles()' with
///	- mbar-menu for project-target
///		1. 'Build'
///		2. 'Run'
///		3. 'Build and Run'
///		4. 'ReBuild'
///
///	- project menu for project-target and one file
///		1. 'Build->Build'
///		2. 'Build->Compile current file'
///		3. 'Build->Run'
///		4. 'Build and Run'
///		5. 'Build->Rebuild'
///		6. 'Build->Clean'
///		7. 'Build->Build workspace'
///		8. 'Build->Rebuild workspace'
///		9. 'Build->Clean workspace'
///
///	- project popup for project-target
///		1. 'Build'
///		2. 'ReBuild'
///		3. 'Clean'
///
///	- file popup for one file
///		1. 'Build file'
///		2. 'Clean file'
///
/// Calls to :
///		-# qtpre::detectQtProject(prj, withreport):1,
///		-# Build::buildAllFiles(prj, Ws, Rebuild):1,
///		-# Build::buildOneFile(prj, file):1,
///		-# AddOnForQt::compilingStop(int idAbort):1,
///		-# Pre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):1,
///

bool AddOnForQt::doComplements(const cbFutureBuild&  _domake)
{
printD("=> Begin 'AddOnForQt::doComplements(" + strInt(_domake) + ")'" );

	if (!m_pProject) 	return false;

	if (!m_isQtProject) return false;

	if (_domake < fbNone) return false;
// print("domake :" + domakeToStr(_domake) ) ;

	if (!m_isQtActiveTarget )	return false;

	bool ok = false;

/// DEBUG
//* ********************************************************
//  m_pBuild->beginDuration("doComplements(...)");
//* ********************************************************
	Mes = NamePlugin + "::doComplements(cbFutureBuild  _domake, ) -> ";
// just project created ?
	wxString nametarget = m_pProject->GetActiveBuildTarget();
//print("nametarget :" + quote(nametarget) );
	if (nametarget.IsEmpty() )
	{
		Mes += _("no target supplied !!"); printError(Mes);
		return false;
	}
	else
	{
        Mes =  Lf + domakeToStr(_domake) + quote("::" + nametarget) ; print(Mes);
	}
	// allright !
	bool valid = true;

/// verify if complements exists already
	// wirh report
	//m_pBuild->detectComplementsOnDisk(m_pProject, nametarget, WITH_REPORT);
	// withless report
	m_pBuild->detectComplementsOnDisk(m_pProject, nametarget, NO_REPORT);
	// it' now an old project with complement files on disk
	m_isNewProject = false;
	// init
	m_removingIsFirst = false;
// test domake
	/* definitions
		enum cbFutureBuild
		{
			fbNone = 10, fbBuild, fbClean, fbRebuild, fbWorkspaceBuild,
			fbWorkspaceClean, fbWorkspaceReBuild
		};
		-> evt.SetInt(static_cast<int>(GetFutureBuild()))
		-> 10, 11, 12, 13, 14, 15, 16
	*/
	int idMenuKillProcess = 0;
	wxString file = m_fileCreator;
//print("fileCreator =" + quote(file) );
	bool BuildOneFile = !file.IsEmpty()
		,BuildAllAddons = !BuildOneFile && (_domake > fbNone)
		;
/// debug
//Mes = Lf + "Call 'doComplements' ...";
//Mes += " _domake = " + strInt(_domake); printWarn(Mes);

//Mes =  ", BuildOneFile = " ;
//Mes += BuildOneFile ? "true ->" + quote(file) : "false";
//Mes += ", BuildAllAddons = " + strBool(BuildAllAddons); printWarn(Mes);
/// <==
	if (!BuildOneFile && !BuildAllAddons)
	{
		return false;
	}

	cbFutureBuild FBuild = _domake;
// calculate future action
	bool Build	 	= FBuild == fbBuild || FBuild == fbRebuild ; //|| FBuild == fbWorkspaceBuild ;
	bool Clean 	= FBuild == fbClean || FBuild == fbRebuild ; //|| fbWorkspaceClean;
	bool Rebuild 	= FBuild == fbRebuild  ;//|| FBuild == fbWorkspaceReBuild;
	bool WsBuild 	= FBuild == fbWorkspaceBuild || FBuild == fbWorkspaceReBuild;
/*
/// Debug
Mes = " FBuild = " + strBool(FBuild);
Mes += ", Build(11) = " + strBool(Build);
Mes += ", Clean(12) = " + strBool(Clean);
Mes += ", Rebuild(13) = " + strBool(Rebuild);
printWarn(Mes);
*/
// to 'Addons' Log
	SwitchToMyLog();

///********************************
/// Build all complement files
///********************************
	if (BuildAllAddons)
	{
	// log clear
		Mes =Tab +  _("Wait a little bit") ;Mes += " ...";
		printWarn(Mes);
	/// build
		if (Build)
		{
			if (WsBuild)
			{
				Mes =  _("Build workspace") ;
				Mes += " ..." + _("NOT YET ACHIEVED") ;
				printError(Mes);
			}
		// real target
			m_pProject->SetActiveBuildTarget(nametarget);
			ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(nametarget);
			// verify Qt target
			ok = m_pBuild->isGoodTargetQt(pBuildTarget);
		/// if a virtual target !! => '!pBuildTarget' => ok = false
			if (ok)
			{
				m_isRunning = true;
				// prebuil complements
				if (Rebuild)
					ok = m_pBuild->buildAllFiles(m_pProject, WsBuild, ALLBUILD);
				// only preBuild when date complements < date creator
				else
					ok = m_pBuild->buildAllFiles(m_pProject, WsBuild, NO_ALLBUILD);
			    if (! ok)
			    {
			    //	Mes = Tab + "m_pBuild->buildAllFiles(...) => ";
			    //  Mes += _("Error 'PreBuild' !!!");   printError(Mes);
				  valid = false;
			    }
			    m_isRunning = false;
			}
			else
			{
			    Mes = _("It's not a valid Qt target") ;
			    Mes += ", " + _("check if the target is not virtual or only a command");
			    Mes += " !!!";
			    printError(Mes);
			}
			Build = false;
		} /// end Build
		else
		if (Clean)
		{
			Mes =  _("Clean all files") ;
			Mes += " ..." + _("NOT YET ACHIEVED") ;
			printError(Mes);
		/// cleanning current file
			//ok = m_pBuild->cleanOneFile(m_pProject, file);
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
//print("One file to compile ...");
		m_isRunning = true;

		if (Clean)
		{
			Mes =  _("Clean OneFile") ;
			Mes += " ..." + _("NOT YET ACHIEVED") ;
			printError(Mes);
		/// cleanning current file
			//ok = m_pBuild->cleanOneFile(m_pProject, file);
			ok = true;
			Clean = false;
		}

		if (Build)
		{
			Mes =  _("Build OneFile") ;
			Mes += " ..." + _("NOT YET ACHIEVED");
			printError(Mes);
		/// preCompile active file
			ok = m_pBuild->buildOneFile(m_pProject, file);

			Build = false;
		}
	}
	m_isRunning = false ;
	//m_removingIsFirst = true;
	m_fileCreator = wxEmptyString;

// switch last build log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_Lastlog);
    Manager::Get()->ProcessEvent(evtSwitch);

    if (!valid)
	{
	// call compiling stop
		compilingStop(idMenuKillProcess);
	}

	// disabled abort
	//if (m_mbar)		m_mbar->Enable(idMenuKillProcess, false);

	m_isRunning = false;

/// DEBUG
//* ******************************************************
	//m_pBuild->endDuration("doComplements(...)");
//* ******************************************************
	Mes.Clear();

//print("	=> End 'AddOnForQt::OnComplements(...)' => " + strBool(ok) );

	return ok;
}

///-----------------------------------------------------------------------------
/// Loading manually plugin
///		- called ONLY when the plugin is manually loaded
///
///	Called by :
///		1. cbEVT_PLUGIN_INSTALLED
///
void AddOnForQt::OnPluginLoaded(CodeBlocksEvent& _event)
{
printD("=> Begin 'AddOnForQt::OnPluginLoaded((...)'");

/// with parsing project
    m_noParsing = false;
/// with visible messages
	m_WithMessage = true;
/// init done
	m_initDone = true;
/// just for debug
/*
	Mes = "AddOnForQt::OnPluginLoaded(...) -> ";
	Mes +=  " 'AddOnForQt' is manually loaded";
	Mes += Space + "-> m_noParsing = " + strBool(m_noParsing) ; printError(Mes) ;
*/
/// the active project
    m_pProject = Manager::Get()->GetProjectManager()->GetActiveProject();
    if (m_pProject)
    {
/// debug
/// Mes = "Notify activate project ..."; printWarn(Mes);
    // pseudo event
        m_pseudoEvent = true;
        CodeBlocksEvent evt(cbEVT_PROJECT_ACTIVATE, 0, m_pProject, 0, this);
        OnActivateProject(evt);
    }

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
}

///-----------------------------------------------------------------------------
/// The new workspace is completely loaded
///
///	Called by :
///		1. cbEVT_WORKSPACE_LOADING_COMPLETE
///
void AddOnForQt::OnPluginLoadingComplete(CodeBlocksEvent& _event)
{
printD("=> Begin 'AddOnForQt::OnPluginLoadingComplete((...)'");

/// with parsing project
    m_noParsing = false;
///
	m_WithMessage = true;
/// using plugins
	m_initDone = true;
/// just for debug
/*
	Mes = "AddOnForQt::OnPluginLoadingComplete(...) -> ";
	Mes += _("all plugins are loaded");
	Mes += Space + "m_initDone = " + strBool(m_initDone) ; printWarn(Mes) ;
*/
	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
}


///-----------------------------------------------------------------------------
/// origin : ProjectManager
///		- the current workspace is begin closed
/// Called by :
///		1. event 'cbEVT_WORKSPACE_CLOSING_BEGIN'
///
void AddOnForQt::OnWorkspaceClosed(CodeBlocksEvent& _event)
{
printD("=> Begin 'AddOnForQt::OnWorkspaceClosed((...)'");
/// stop parsing project
    m_noParsing = true;

	m_WithMessage = true;
/// just for debug
/*
	Mes = "AddOnForQt::OnWorkspaceClosed(...) -> ";
	Mes +=  " 'Current Workspace' is begin closed";
	Mes += Space + "-> m_noParsing = " + strBool(m_noParsing) ; printError(Mes) ;
*/
	Mes.Clear();
/// The event processing system continues searching
    _event.Skip();
}

///-----------------------------------------------------------------------------
/// origin : ProjectManager
///	prevent that the workspace is complete loading
/// Called by :
///		1. event 'cbEVT_WORKSPACE_LOADONG_COMPLETE'
///
void AddOnForQt::OnWorkspaceComplete(CodeBlocksEvent& _event)
{
printD("=> Begin 'AddOnForQt::OnWorkspaceComplete(...)'");

/// with parsing project
    m_noParsing = false;
///
	m_WithMessage = true;
/// just for debug

	Mes = "AddOnForQt::OnWorkspaceComplete(...) -> ";
	Mes +=  " 'Workspace' is completely loaded";
	Mes += Space + "-> m_noParsing = " + strBool(m_noParsing) ; printError(Mes) ;

	Mes.Clear();
/// The event processing system continues searching
    _event.Skip();
}

///-----------------------------------------------------------------------------
/// Invalid the messages to 'Prebuild log'
///
/// Called by :
///		1. event 'cbEVT_APP_START_SHUTDOWN'
///
void AddOnForQt::OnAppBeginShutDown(CodeBlocksEvent& _event)
{
Mes = "AddOnForQt::OnAppBeginShutDown(...) ..."; printError(Mes) ;
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
///	Delete the pre-builders and do de-initialization for plugin
///
/// Called by :
///
void AddOnForQt::OnRelease(bool appShutDown)
{
//1- delete builder"
	if (m_pBuild)
	{
		delete m_pBuild;
		m_pBuild = nullptr;
	}
//3-  delete log
	if(m_LogMan)
    {
        if(m_AddonLog)
        {
            CodeBlocksLogEvent evt(cbEVT_REMOVE_LOG_WINDOW, m_AddonLog);
            Manager::Get()->ProcessEvent(evt);
        }
    }
    m_AddonLog = nullptr;
    m_menuBar = nullptr;
    m_buildMenu = nullptr;

//3- do de-initialization for your plugin
    // if appShutDown is true, the plugin is unloaded because Code::Blocks is being shut down,
    // which means you must not use any of the SDK Managers
    // NOTE: after this function, the inherited member variable
    // m_IsAttached will be FALSE...
    if(!appShutDown)
		Manager::Get()->RemoveAllEventSinksFor(this);
}

///-----------------------------------------------------------------------------
/// Append text to log
///
/// Called by :
///		-# all printxxx(wxString)
///
void AddOnForQt::AppendToLog(const wxString& _Text, Logger::level _lv)
{
    if(m_AddonLog && m_WithMessage)
    {
        CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_AddonLog);
        Manager::Get()->ProcessEvent(evtSwitch);
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
// display a log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, _indexLog);
	Manager::Get()->ProcessEvent(evtSwitch);
}
void AddOnForQt::SwitchToMyLog()
{
// display a log
	CodeBlocksLogEvent evtSwitch(cbEVT_SWITCH_TO_LOG_WINDOW, m_AddonLog);
	Manager::Get()->ProcessEvent(evtSwitch);
}

///-----------------------------------------------------------------------------
/// Activate a project
///		called before a 'cbEVT_PROJECT_NEW' !!
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
// print("=> Begin 'AddOnForQt::OnActivateProject(...)" );
/// current workspace is begin closed or new workspace is completely loaded ?
	if (m_noParsing)
	{
		_event.Skip() ; return;
	}
/// plugins are loaded ?
	if (!m_initDone)
	{
		_event.Skip() ; return;
	}

// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(!m_pseudoEvent);
		m_pseudoEvent = false;
		return;
	}

// the active project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied") ;
		Mes += " !!"; printError(Mes);
		_event.Skip(!m_pseudoEvent);
		m_pseudoEvent = false;
		return;
	}
// actualize the project here
	m_pProject = prj;
	m_activeNameProject = m_pProject->GetTitle();
	m_activeNameTarget = m_pProject->GetActiveBuildTarget();

//printD("OnActivateProject::m_activeNameProject = " + quote(m_activeNameProject + "::" + m_activeNameTarget ) );
/// DEBUG
//* *********************************************************
//	m_pBuild->beginDuration("OnActivateProject(...)");
//* *********************************************************
// detect Qt project ... with report : feed 'qtpre::m_pProject'
	m_isQtProject = m_pBuild->detectQtProject(m_pProject, WITH_REPORT);
//Mes = "m_qtproject = " + strBool(m_isQtProject); printWarn(Mes);
	/// momentarily to create the Qt menus at CB startup !
	if (m_isQtProject && ! m_buildFinded)		BuildMenu(m_menuBar);
	// enable menubar items
	enableAllMenubarQt(m_isQtProject);
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
	printWarn(Mes);
	if (!m_isQtProject)
	{
		_event.Skip(!m_pseudoEvent);
		m_pseudoEvent = false ;
		return;
	}

/// here : project has a Qt target
	// virtual target ?
//	m_activeNameTarget = m_pProject->GetActiveBuildTarget();
	m_isQtActiveTarget = m_pBuild->detectQtTarget(m_activeNameTarget);

//Mes = "AddOnForQt::OnActivateProject() : m_isQtActiveTarget = " + strBool(m_isQtActiveTarget); print(Mes);
/// search if complements exists already on disk ?
		// ok = true => files Qt exists on disk
	bool ok = m_pBuild->detectComplementsOnDisk(m_pProject, m_activeNameTarget, !m_isNewProject);
	m_removingIsFirst = true;
		// it' now an old project
	//	m_isNewProject = false;

/// The event processing system continues searching if not a pseudo event
	_event.Skip(!m_pseudoEvent);
	m_pseudoEvent = false ;

/// DEBUG
//* *******************************************************
//	m_pBuild->endDuration("OnActivateProject(...)");
//* *******************************************************
	Mes.Clear();

printD("	<= End 'AddOnForQt::OnActivateProject(...)");
}

///-----------------------------------------------------------------------------
/// Activate a target :
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
printD("=> Begin 'AddOnForQt::OnActivateTarget(...)");
/// if stop parsing project
    if (m_noParsing)
    {
		_event.Skip() ; return;
	}

/// At startup, this method is called before any project is active !!
	if (!m_pProject)
	{
		_event.Skip(); return;
	}

/// a Qt target can exist in a non-Qt project!
// not a Qt current project
	if (!m_isQtProject)
	{
printD("m_isQtProject =" + strBool(m_isQtProject) );
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
		Mes += Space + _("no project supplied") + " !!"; printError(Mes);
		_event.Skip(); return;
	}
/// is it possible ??
// it's not the current project !
	if ( m_pProject != prj)
	{
/// just for debug
		Mes = quote(prj->GetTitle()) + " : " ;
		Mes += _("event project is not the current project") + " =>";
		if (m_pProject) Mes +=  quote(m_pProject->GetTitle());
		printWarn(Mes);
/// <=
		_event.Skip(); return;
	}

/// DEBUG
//* *********************************************************
//	m_pBuild->beginDuration("OnActivateTarget(...)");
//* *********************************************************

/// the active target
	wxString nametarget  =  _event.GetBuildTargetName() ;
	if (nametarget.IsEmpty() )
	{
	/// test if the project is open
		if (Manager::Get()->GetProjectManager()->IsProjectStillOpen(m_pProject))
		{
			Mes += Space + _("no target supplied") + " !!"; printError(Mes);
		}
		_event.Skip();
		return;
	}
	// is a command target
	if (m_pBuild-> isCommandTarget(nametarget))
	{
		Mes =  Tab + quote("::" + nametarget);
		Mes += Tab + _("is a command target") + " !!" ; printWarn(Mes);
		_event.Skip(); return;
	}
	m_activeNameTarget = nametarget;
//print("OnActivateTarget => m_activeNameTarget = " + quote(m_activeNameProject + "::" + m_activeNameTarget ) );
	// all targets : real and virtual ...
	m_isQtActiveTarget = m_pBuild->detectQtTarget(nametarget, WITH_REPORT);
//Mes = "AddOnForQt::OnActivateTarget() : m_isQtActiveTarget = " + strBool(m_isQtActiveTarget); print(Mes);
	// enable menubar items
	enableAllMenubarQt(m_isQtActiveTarget);

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* *********************************************************
//	m_pBuild->beginDuration("OnActivateTarget(...)");
//* *********************************************************
printD("	<= End 'AddOnForQt::OnActivateTarget(...)");
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

/// just for debug
	//Mes = NamePlugin + "::OnNewProject(CodeBlocksEvent& event) -> ";
	//printError(Mes);
// the new project
	cbProject *pProject = _event.GetProject();
	if(!pProject)
	{
		Mes += _("no project supplied");
		Mes + " !!"; printError(Mes);
		_event.Skip(); return;
	}

// actualize the project
	m_pProject = pProject;
	m_isNewProject = true;
// just project created ?
	wxString nametarget = m_pProject->GetActiveBuildTarget();
	if (nametarget.IsEmpty() )
	{
		Mes += _("no target supplied") ;
		Mes += " !!"; printError(Mes);
		_event.Skip(); return;
	}
//Mes = "project name" + quote(m_pProject->GetTitle()) ;
//Mes +=  Space + "nametarget =" + quote(nametarget); printWarn(Mes);

// detect Qt project ... with report
	m_isQtProject = m_pBuild->detectQtProject(m_pProject, WITH_REPORT);
	// advice
	Mes = _("The New project") + quote(m_pProject->GetTitle());
	if (m_isQtProject)
	{
		Mes += _("has at least one target using Qt libraries") ;
		Mes += "...";
	}
	else
	{
		Mes += _("is NOT a Qt project") ;
		Mes += " !!";
	}
	printWarn(Mes);

// detect Qt active Target ...
	m_isQtActiveTarget = m_pBuild->detectQtTarget(nametarget, NO_REPORT);
	// advice
	Mes = Tab + quote("::" + nametarget ) + Space ;
	Mes += _("is") + Space;
	if(!m_isQtActiveTarget)		Mes += _("NOT") + Space;
	Mes += _("a Qt target");
	print(Mes);

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
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
///		1. qtpre::detectQtProject(const wxString& _nametarget, cbProject * _pProject):1,
///		2. qtpre::detectQtTarget(const wxString& _nametarget, cbProject * _pProject):1,
///		3. Pre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):2,
///		4. Pre::isTargetWithComplement(cbProject * _pProject, const wxString & _target):1,
///		5. Pre::newNameComplementDirTarget(wxString & _newname, wxString & _oldname):1,
///
void AddOnForQt::OnRenameProjectOrTarget(CodeBlocksEvent& _event)
{
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
//Mes += " but 'm_pProject' is null !! " ; printWarn(Mes);
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
/// Debugdebug
//	Mes += quote(prj->GetTitle()) + " : " ;
//		Mes += _("event project is not the current project !!");
//		printError(Mes);
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
//Mes += ", oldtarget =" + quote(oldnametarget); printWarn(Mes);

	// it's a command target
	if (m_pBuild-> isCommandTarget(nametarget))
	{
		Mes =  Tab + quote( nametarget ) + _("is a command target") + " !!" ;
		printWarn(Mes);
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
		m_isQtProject = m_pBuild->detectQtProject(m_pProject, WITH_REPORT);
//Mes = "==> m_isQtProject = " + strBool(m_isQtProject); printWarn(Mes);
		if (!m_isQtProject)
		{
			_event.Skip(); return;
		}
		Mes = "=== " ;
		Mes += _("A new name project") + quote(m_pProject->GetTitle());
		printWarn(Mes);
		Mes = quote(prj->GetTitle());
		if (m_isQtProject)
		{ // advice
			Mes += _("has at least one target using Qt libraries") ;
			Mes += "...";
			printWarn(Mes);
			// complements exists already ?
			m_pBuild->detectComplementsOnDisk(m_pProject, nametarget, !m_isNewProject);
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
		m_pBuild->setProject(m_pProject);

	// new active target ?
		wxString activetarget = m_pProject->GetActiveBuildTarget() ;
//Mes += "!! activetarget =" + quote(activetarget);
//Mes += ", nametarget =" + quote(nametarget); printWarn(Mes);

		// advice
		Mes = "=== " ;
		Mes += _("Old target name") + quote(oldnametarget)  ;
		Mes += "=> " ;
		Mes += _("New target name") + quote(nametarget);
		printWarn(Mes);

		if (!activetarget.Matches(nametarget))
		{
		// detect Qt target ...
			okqt = m_pBuild->detectQtTarget(nametarget, NO_REPORT);
			Mes = quoteNS("::" + nametarget) + Space;
			Mes += _("is") + Space;
			if(!okqt)		Mes += _("NOT") + Space;
			Mes += _("a Qt target");
			printWarn(Mes);
		}
	/// updates of the new target :
		// new build target
		ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(nametarget);
		bool ok = m_pBuild->updateNewTargetName(pBuildTarget, oldnametarget);
		/// ... TO FINISH

	}
	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
}

///-----------------------------------------------------------------------------
/// Stop compiling request to 'CompilerGCC'
///
///	Called by :
///		1. AddOnForQt::OnComplements(CodeBlocksEvent& _event):1,

void AddOnForQt::compilingStop(int _idAbort)
{
Mes = "AddOnForQt::compilingStop(" + strInt(_idAbort) + ""; printError(Mes);
	CodeBlocksEvent evtMenu(wxEVT_COMMAND_MENU_SELECTED, _idAbort);
	// if comes from 'AddOnForQt' for 'CompilerGCC' !
	evtMenu.SetInt(_idAbort);
	Manager::Get()->ProcessEvent(evtMenu);
// Not mandatory
//	Manager::Yield();
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
/// origin : compilergxx.cpp
///
/// Called by :
///
///	Call to :
///		1.
///
void AddOnForQt::OnUpdateUI(wxUpdateUIEvent& _event)
{
/*
    const int id = _event.GetId();

    if  (id == idMenuKillProcess)
    {
        _event.Enable(IsRunning());
        return;
    }

    if (IsRunning())
    {
        _event.Enable(false);
        return;
    }

    ProjectManager *projectManager = Manager::Get()->GetProjectManager();
    cbPlugin *runningPlugin = projectManager->GetIsRunning();
    if (runningPlugin && runningPlugin != this)
    {
        _event.Enable(false);
        return;
    }

    cbProject* prj = projectManager->GetActiveProject();
   // cbEditor* ed = Manager::Get()->GetEditorManager()->GetBuiltinActiveEditor();

	if (id == idMenuBuildWSComplements || id == idMenuRebuildComplements
		|| id == idMenuRebuildWSComplements
        || id == idCleanAddonsAlone || id == idMenuCleanWSComplements
		)
    {
        _event.Enable(prj);
    }
    */

    _event.Skip();
}

///-----------------------------------------------------------------------------
///  Abort compiling complement files from 'CompilerGCC'
///
/// Called by :
///		1. event 'cbEVT_COMPILER_FINISH' from 'CompilerGCC::AbortPreBuild()'
///	       with button-menu
///		2. 'setAbort'
///
///	Call to :
///		1. Build::setAbort():1,
///
void AddOnForQt::OnAbortAdding(CodeBlocksEvent& _event)
{
/// if stop parsing project
    if (m_noParsing)
    {
		_event.Skip() ; return;
	}
// not a Qt current project
	if (!m_isQtProject)
	{
		_event.Skip(); return;
	}
// it's for AddOnForQt ?
	if (_event.GetId() != 1)
	{
		_event.Skip(); return;
	}
	//Mes = wxEmptyString;
/// just for debug
	Mes = NamePlugin + "::OnAbortAdding(CodeBlocksEvent& event) -> ";
///
// the active project
	cbProject *prj = _event.GetProject();
	// no project !!
	if(!prj)
	{
		Mes += _("no project supplied"); printError(Mes);
		_event.Skip();
		return ;
	}
/// just for debug
	if (m_pProject != prj)
	{
//Mes += _("event project target is not the current project !!";	printError(Mes);
		_event.Skip();
		return;
	}
// Abort complement files creating
	m_pBuild->setAbort(true);
// cleaning plugin  : TODO
	/// ...
	Mes.Clear();
}
///-----------------------------------------------------------------------------
/// Before the first removed file of the project
///
/// Called by
///		1. event 'cbEVT_PROJECT_BEGIN_REMOVE_FILES'
///
void AddOnForQt::OnBeginFileRemoved(CodeBlocksEvent& _event)
{
printD("=> Begin 'AddOnForQt::OnBeginFileRemoved(..)'" );

/// if stop parsing project
    if (m_noParsing)
    {
		_event.Skip() ; return;
	}

	if (_event.GetProject() == m_pProject)
	{
	/// the indicator
		m_removingIsFirst = true;
	}
	_event.Skip();

printD("	<= End  'AddOnForQt::OnBeginFileRemoved(..)'" );
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
///		2. Build::unregisterComplementFile(wxString & _file, bool _first):1,
///		2. Pre::isCreatorFile(const wxString & _file):1,
///		3. Build::unregisterCreatorFile(wxString & _file, bool _first):1,
///
void AddOnForQt::OnProjectFileRemoved(CodeBlocksEvent& _event)
{
printD("=> Begin 'AddOnForQt::OnProjectFileRemoved(..)'" );
/// if stop parsing project
    if (m_noParsing)
    {
		_event.Skip() ; return;
	}
///	the project used
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
//	Mes += _("event project is not the current project") + " !!";  printError(Mes);
/// <==
		_event.Skip();	return;
	}

// actualize the project
	m_pProject = prj;
	// call  setBuildTarget(...)
	m_pBuild->setProject(prj);

	// the removed file ( absolute path )
	wxString filename = _event.GetString();
	if (filename.IsEmpty())
	{
		_event.Skip(); return;
	}

	// switch  'Prebuild' log
    SwitchToMyLog();
// only name less path
	wxString file = filename.AfterLast(Slash) ;
//Mes = Lf + "filename = " + quote(filename);
//Mes += ",file =" + quote(file); print(Mes);
	bool ok = false;
	Mes = wxEmptyString;
// it's a complement file ?
	m_iscomplement = m_pBuild->isComplementFile(file);
	if (m_iscomplement)
	{
		if (m_removingIsFirst)
		{
			Mes = "==> " + _("Complement file(s) was removed from C::B and disk");
			Mes += " ..." ;
			printWarn(Mes);
			m_removingIsFirst = false;
		}
	// unregisterer one complement file
		//ok = m_pBuild->unregisterComplementFile(filename,  m_removingIsFirst) ;
		ok = m_pBuild->unregisterComplementFile(filename) ;
		if (!ok)
			Mes = Lf + _("Complement") + Space;
	}
	else
// it's a creator file ?
	if (m_pBuild->isCreatorFile(file))
	{
//Mes = "file =" + quote(file) + _("is a creator file"); print(Mes);
	// unregisterer one creator file and its complement file
		//ok = m_pBuild->unregisterCreatorFile(filename, m_removingIsFirst) ;
		ok = m_pBuild->unregisterCreatorFile(filename) ;
		if (ok)
		{
			// delete the complement
			Mes =  _("The creator")  + Space  + quote(filename) +  _(": We must delete the complement file on disk ...");
			printError(Mes);
			/// TODO ...
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
		printError(Mes);
//Mes = Tab + quote(filename); print(Mes);
	}

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();

printD("	<= End 'AddOnForQt::OnProjectFileRemoved(...)'");
}
///-----------------------------------------------------------------------------
/// After the last removed file of the project
///
/// Called by
///		1. event 'cbEVT_PROJECT_END_REMOVE_FILES'
///
void AddOnForQt::OnEndFileRemoved(CodeBlocksEvent& _event)
{
printD("=> Begin 'AddOnForQt::OnEndFileRemoved(..)'" );

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
		Mes = "==> " + _("End of complement file(s) removed from C::B and disk");
		Mes += " ..." ;
		printWarn(Mes);
	}

	_event.Skip();
}

/*
///-----------------------------------------------------------------------------
/// Validate messages to 'Prebuild log'
///
///	Called by : DISCONNECTED
///		1. event 'cbEVT_APP_STARTUP_DONE'
///
void AddOnForQt::OnAppDoneStartup(CodeBlocksEvent& _event)
{
	m_WithMessage = true;
	//m_initDone = true;
/// just for debug
//	Mes = "AddOnForQt::OnAppDoneStartup(...) ... ";
//	Mes +=  _("app done startup");
//	Mes += Space + "-> m_initDone = " + strBool(m_initDone) ;
/// <=

	// ***********************************
	// this line hang code completion ??
	// ***********************************
	m_pProject = Manager::Get()->GetProjectManager()->GetActiveProject();
	if (m_pProject)
	{
	// pseudo event
	//	m_pseudoEvent = true;
	//	CodeBlocksEvent evt(cbEVT_PROJECT_ACTIVATE, 0, m_pProject, 0, this);
	//	OnActivateProject(evt);
	}

	if (m_pProject)
		Mes +=  Lf + Tab + _("Active project") + " =" + quote(m_pProject->GetTitle() );
	else
		Mes +=  Lf + Tab + _("No active project") + "  !!";
	//printWarn(Mes) ;

	Mes.Clear();
/// The event processing system continues searching
	_event.Skip();
}
*/
/*
///-----------------------------------------------------------------------------
/// Open a project
///
/// Called by : DISCONNECTED
///		1. event 'cbEVT_PROJECT_OPEN'
///
/// Calls to : none
///

void AddOnForQt::OnOpenProject(CodeBlocksEvent& _event)
{
// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(); return;
	}
/// Debug
//	Mes = NamePlugin + "::OnOpenProject(CodeBlocksEvent& event) -> ";
//	printError(Mes);
/// <=
// the active project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied") ;
		Mes += " !!"; printError(Mes);
		_event.Skip();
		return;
	}
/// DEBUG
/// *****************************************************
//	m_pBuild->beginDuration("OnOpenProject(...)");
/// *****************************************************
// active target
	wxString nametarget = prj->GetActiveBuildTarget();
	Mes += quote(prj->GetTitle()) ;
	if (nametarget.IsEmpty() )
	{
		Mes += Space + _("no target supplied") ;
		Mes += " !!"; printError(Mes);
		_event.Skip();
		return;
	}

// messages
	Mes = quote(prj->GetTitle() + "::" + nametarget);
	Mes += _("is opened") ;
	Mes += " !!"; printWarn(Mes);

/// The event processing system continues searching
	_event.Skip();

/// DEBUG
/// ***************************************************
//	m_pBuild->endDuration("OnOpenProject(...)");
/// ***************************************************
	Mes.Clear();
}
*/
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
/// just for debug
	Mes = NamePlugin + "::OnSaveProject(CodeBlocksEvent& event) -> ";
	//printWarn(Mes);
// the project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied") ;
		Mes += " !!"; printError(Mes);
		_event.Skip();
		return;
	}
/// messages
	Mes += quote(prj->GetTitle()) + _("is saved") + " !!"; printWarn(Mes);

/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* ***************************************************
//	m_pBuild->endDuration("OnSaveProject(...)");
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
// not editor
	if (!_event.GetEditor())
	{
		Mes += _("no editor supplied") ;
		Mes += " !!"; printError(Mes);
		_event.Skip(); return;
	}
/// Debug
//	Mes = NamePlugin + "::OnSaveFileEditor(CodeBlocksEvent& event) -> ";
//	printWarn(Mes);
/// <=
// the editor
	EditorManager * em = EditorManager::Get();
	cbEditor * ed = em->GetBuiltinEditor(_event.GetEditor());
	if(!ed)
	{
		Mes += _("no editor supplied") ;
		Mes += " !!"; printError(Mes);
		_event.Skip();
		return;
	}
	wxString filename = ed->GetFilename();

/// messages
	Mes += quote(ed->GetTitle()) + _("is saved") ;
	//Mes +=  quote(filename);
	Mes += ": " + m_pBuild->date();
	printWarn(Mes);

/// The event processing system continues searching
	_event.Skip();

/// DEBUG
//* ******************************************************
//	m_pBuild->endDuration("OnSaveFileEDitor(...)");
//* ******************************************************
	Mes.Clear();
}
/*
///-----------------------------------------------------------------------------
/// Close a project
///
/// Called by : DISCONNECTED !
///		1. event 'cbEVT_PROJECT_CLOSE'
///
/// Calls to : none
///
void AddOnForQt::OnCloseProject(CodeBlocksEvent& _event)
{
// wait for message validation
	if (!m_WithMessage)
	{
		_event.Skip(); return;
	}
/// Debug
//	Mes = NamePlugin + "::OnCloseProject(CodeBlocksEvent& event) -> ";
//	printWarn(Mes);
/// <==
// the project
	cbProject *prj = _event.GetProject();
	if(!prj)
	{
		Mes += _("no project supplied") ;
		Mes += " !!"; printError(Mes);
		_event.Skip();
		return;
	}
/// messages
	Mes = quote(prj->GetTitle()) + _("is closed") + " !!"; printWarn(Mes);

/// The event processing system continues searching
	_event.Skip();

/// DEBUG
/// ****************************************************
//	m_pBuild->endDuration("OnCloseProject(...)");
/// ****************************************************
	Mes.Clear();
}
*/
///-----------------------------------------------------------------------------
