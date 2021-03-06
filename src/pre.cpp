﻿/***************************************************************
 * Name:      pre.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-02-27
 * Modified:  2020-10-05
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#include "pre.h"

#include <sdk.h>
#include <cbplugin.h>      // sdk version
#include <manager.h>
#include <cbproject.h>
#include <compiletargetbase.h>
#include <projectmanager.h>
#include <macrosmanager.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <wx/datetime.h>
// for linux
#include <wx/dir.h>
#include <projectfile.h>
#include "infowindow.h"     // InfoWindow::Display(
// not place change !
#include "defines.h"
///-----------------------------------------------------------------------------
///	Constructor
///
/// Called by :
///		1. Build::Build(cbProject * _pProject, int _logindex, wxString & _nameplugin):1,
///
Pre::Pre(const wxString & _nameplugin, int _logindex)
	: m_namePlugin(_nameplugin),
	  m_pm(Manager::Get()),
	  m_pMam(Manager::Get()->GetMacrosManager() ),
	  m_LogPageIndex(_logindex)
{
#if   defined(__WXMSW__)
	m_Win = true; m_Linux = m_Mac = false;
#elif defined(__WXGTK__)
	m_Linux = true ; m_Win = m_Mac = false;
#elif defined(__WXMAC__)
	m_Mac = true; m_Win = m_Linux = false;
#endif
	//platForm();
	Mes = wxEmptyString;
}
///-----------------------------------------------------------------------------
///	Destructor
///
/// Called by
///		1. Build::~Build():1,
///
Pre::~Pre()
{
}

#include <wx/menu.h>
/// ----------------------------------------------------------------------------
/// Locate and call a menu from string (e.g. "/Valgrind/Run Valgrind::MemCheck")
/// it's a copy of 'CodeBlocks::sc_globals.cpp::CallMenu(const wxString& menuPath)'
/// return menu identificateur or -1 il failed
///
/// called by;
///		1. AddOnForQt::OnMenuComplements(wxCommandEvent& _event):1,
///
/// ----------------------------------------------------------------------------
int Pre::CallMenu(const wxString& _menuPath)
{
printD("=> Begin 'Pre::CallMenu(" + _menuPath + ")'" );
    int idMenu = wxNOT_FOUND;
    // this code is partially based on MenuItemsManager::CreateFromString()
    wxMenuBar* mbar = Manager::Get()->GetAppFrame()->GetMenuBar();
    wxMenu* menu = nullptr;
    size_t pos = 0;
    while (true)
    {
        // ignore consecutive slashes
        while (pos < _menuPath.Length() && _menuPath.GetChar(pos) == '/')
            ++pos;

        // find next slash
        size_t nextPos = pos;
        while (nextPos < _menuPath.Length() && _menuPath.GetChar(++nextPos) != '/' ) ;
//print(" => menuPath =" + quote(menuPath) );
        wxString current = _menuPath.Mid(pos, nextPos - pos);
//print(" => current =" + quote(current) );
        if (current.IsEmpty())       break;

        bool isLast = nextPos >= _menuPath.Length();
        // current holds the current search string
        if (!menu) // no menu yet? look in menubar
        {
            int menuPos = mbar->FindMenu(current);
            if (menuPos == wxNOT_FOUND)
                break; // failed
            else
                menu = mbar->GetMenu(menuPos);
        }
        else
        {
            if (isLast)
            {
                int id = menu->FindItem(current);
printD("id => " + strInt(id) );
                if (id != wxNOT_FOUND)
                {
                    wxCommandEvent evt(wxEVT_COMMAND_MENU_SELECTED, id);
			//	#if wxCHECK_VERSION(3, 0, 0)
                    mbar->GetEventHandler()->ProcessEvent(evt);
		/*
				#else
                    if ( !mbar->ProcessEvent(evt) )
                    {
                        wxString msg; msg.Printf(_("Calling the menu '%s' with ID %d failed."), menuPath.wx_str(), id);
                        cbMessageBox(msg, _("Script error"), wxICON_WARNING);
                    }
				#endif
		*/
                    // done
                    idMenu = id;
                }
                break;
            }
            int existing = menu->FindItem(current);
            if (existing != wxNOT_FOUND)
                menu = menu->GetMenuItems()[existing]->GetSubMenu();
            else
                break; // failed
        }
        pos = nextPos; // prepare for next loop
    }

printD("    <= End 'Pre::CallMenu(...) =>" + strInt(idMenu) );

    return idMenu;
}
///-----------------------------------------------------------------------------
/// Get date
///
/// Called by  :
///		1. Build::beginMesBuildCreate():1,
///		2. Build::endMesBuildCreate():1
///
wxString Pre::date()
{
	wxDateTime now = wxDateTime::Now();
	//wxString str = now.FormatISODate() + "-" + now.FormatISOTime();
	wxString str = now.FormatDate() + "-" + now.FormatTime();

    return str ;
}
///-----------------------------------------------------------------------------
/// Execution time
///
/// Called by  :
///		1. Build::endMesBuildCreate():1
///		2. Build::endMesFileCreate():1,
///
wxString Pre::duration()
{
    clock_t dure = clock() - m_start;
    if (m_Linux)
        dure /= 1000;

    return wxString::Format("%ld ms", dure);
}
///-----------------------------------------------------------------------------
///  Begin duration for Debug
///	Called by :
///		1.
/// Call to :
///		1. date
///
void Pre::beginDuration(const wxString & _namefunction)
{
// target name
/*
	Mes = Lf + "--------------> " ;
	Mes += _("PreBuild");
	Mes += " :" ;
	Mes += quote( m_nameActiveTarget );
	printWarn(Mes);
	*/
// date
	Mes = Lf + "==> ";
	Mes += _("Start of") + quote(_namefunction) ;
	Mes += ": " + date();
	printWarn(Mes);
	m_start = clock();
	Mes.Clear();
}

///-----------------------------------------------------------------------------
///  End duration for Debug
///	Called by :
///		1.
/// Call to :
///		1. date
///
void Pre::endDuration(const wxString & _namefunction)
{
	Mes = "<== " ;
	Mes += _("End of") + quote( _namefunction  ) ;
	Mes += " : " + date();
	Mes += " -> " + duration();
	//printError(Mes);
	printWarn(Mes);
	Mes.Clear();
}
///-----------------------------------------------------------------------------
/// Give internal plugin name
///
/// Called by : none
///
wxString Pre::namePlugin()
{
	return m_namePlugin;
}
///-----------------------------------------------------------------------------
/// Determines the system platform
///
/// Called by :
///		1.  AddOnForQt::OnAttach():1,
///
wxString Pre::platForm()
{
// choice platform
	Mes = _("Platform") ;
	if (! m_Win)
	{
		#undef SepD
		#define SepD 13
		#undef SizeSep
		#define SizeSep 1
	// tables  ( >= Qt-5.9 )
       //   TODO ????
		m_TablibQt.Add("qtmain",1) ;
		m_TablibQt.Add("qtmaind",1) ;
		// qt5
		m_TablibQt.Add("qt5gui",1) ;
		m_TablibQt.Add("qt5core",1) ;
		m_TablibQt.Add("qt5widgets",1) ;
		m_TablibQt.Add("qt5opengl",1) ;
		m_TablibQt.Add("qt5printsupport",1);
		m_TablibQt.Add("qt5serialport",1) ;
		m_TablibQt.Add("qt5xml",1) ;
		// qt5
		m_TablibQt.Add("qt5guid",1) ;
		m_TablibQt.Add("qt5cored",1) ;
		m_TablibQt.Add("qt5widgetsd",1);
		m_TablibQt.Add("qt5opengld",1) ;
		m_TablibQt.Add("qt5printsupportd",1);
		m_TablibQt.Add("qt5serialportd",1) ;
		m_TablibQt.Add("qt5xmld",1) ;
	}

	if (m_Win)
	{
		Mes += " : 'Win";
		#undef Eol
		#define Eol CrLf
	// tables
		m_TablibQt.Add("qtmain",1) ;
		m_TablibQt.Add("qtmaind",1) ;
		// qt5
		m_TablibQt.Add("qt5gui",1) ;
		m_TablibQt.Add("qt5core",1) ;
		m_TablibQt.Add("qt5widgets",1) ;
		m_TablibQt.Add("qt5opengl",1) ;
		m_TablibQt.Add("qt5printsupport",1);
		m_TablibQt.Add("qt5serialport",1) ;
		m_TablibQt.Add("qt5xml",1) ;
			// <= qt5.9
		m_TablibQt.Add("qt5guid",1) ;
		m_TablibQt.Add("qt5cored",1) ;
		m_TablibQt.Add("qt5widgetsd",1);
		m_TablibQt.Add("qt5opengld",1) ;
		m_TablibQt.Add("qt5printsupportd",1);
		m_TablibQt.Add("qt5serialportd",1) ;
		m_TablibQt.Add("qt5xmld",1) ;
	}
	else
	if (m_Mac)
	{
		Mes += " : 'Mac";
		#undef Eol
		#define Eol Cr
	}
	else
	if (m_Linux)
	{
		Mes += " : 'Linux";
		#undef Eol
		#define Eol Lf
	}

#if defined(_LP64) || defined(_WIN64)
	Mes+="-64'" ;
#else
	Mes += "-32'";
#endif

	return Mes;
}
///-----------------------------------------------------------------------------
///  Get version SDK
///
///  Called by :
/// 	1. AddOnForQt::OnAttach():1,
///
wxString Pre::GetVersionSDK()
{
	// from 'cbplugin.h'
	uint16_t major 	= PLUGIN_SDK_VERSION_MAJOR
				,minor 	= PLUGIN_SDK_VERSION_MINOR
				,release= PLUGIN_SDK_VERSION_RELEASE;

    return  strInt(major) + "." + strInt(minor) + "." + strInt(release);
}

///-----------------------------------------------------------------------------
/// test clean
///
///  Called by :none
///

bool Pre::isClean()
{
	if (m_clean)
	{
		m_Filecreator.Clear();
		m_Registered.Clear();
		m_Createdfile.Clear();
		m_Filestocreate.Clear();
		m_Filewascreated.Clear();
	}

	return m_clean;
}

///-----------------------------------------------------------------------------
/// Set abort
///
/// Called by :
///		1. AddOnForQt::OnAbortAdding(CodeBlocksEvent& event):1,
///
void Pre::setAbort(bool _abort)
{
	m_abort = _abort;
}

///-----------------------------------------------------------------------------
/// Set Set 'm_pProject'
///
/// Called by :
///		1. AddOnForQt::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
///		2. AddOnForQt::OnProjectFileRemoved(CodeBlocksEvent& _event):1,
///
/// Calls to :
///
void Pre::setProject(cbProject * _pProject)
{
	bool good = _pProject != nullptr ;
	if (good)
	{
		m_pProject = _pProject;
		m_nameProject = m_pProject->GetTitle();

		m_nameActiveTarget = m_pProject->GetActiveBuildTarget();
		ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget);
		if (pBuildTarget)   setBuildTarget(pBuildTarget);
		/// active target is virtual !!
		else
		//...
			;
	}
	else
		printError(" Error in 'Pre::setProject(_pProject == null)'" );
}

///-----------------------------------------------------------------------------
/// Set 'm_pBuildTarget'
///
/// Called by :
///		1. Pre::setProject(cbProject * _pProject):1,
///
void Pre::setBuildTarget(ProjectBuildTarget * _pBuildTarget)
{
	if (_pBuildTarget)
	{
		m_pBuildTarget = _pBuildTarget;
	/// NO for a virtaual target
		m_dirObjects =  m_pBuildTarget->GetObjectOutput();
        m_pMam->ReplaceMacros(m_dirObjects);
        if (!m_dirObjects.Last() != Slash )
				m_dirObjects += Slash;
	}
	else
		printError(" Error in 'Pre::setBuildTarget(_pbuildtarget == null)'" );
}

///-----------------------------------------------------------------------------
///	Give the full complement filename  : target + "\" + filename
///
///  Called by :
///		1. Pre::complementDirectory():1,

wxString Pre::fullFilename(const wxString & _file)
{
	wxString filename = _file;
	wxString nameactivetarget = m_pProject->GetActiveBuildTarget() ;
	filename = nameactivetarget + wxString(Slash) + filename;

	return filename;
}

///-----------------------------------------------------------------------------
///	Give the complement directory
///
///  Called by :
///		1. Build::buildOneFile(cbProject * _pProject, const wxString& _fcreator):1,
///		2. Build::addAllFiles():1,

wxString Pre::complementDirectory() const
{
	wxString nameactivetarget = m_pProject->GetActiveBuildTarget() ;
	wxString dircomplement = m_dirPreBuild + nameactivetarget + wxString(Slash) ;

	return dircomplement;
}

///-----------------------------------------------------------------------------
///  Verify complement file
///		ex : 'moc_xxx.cpp', 'ui_xxx.h', 'qrc_xxx.cpp',
///  	file format  = '"m_dirPreBuild"\..._xxxx.ext',
///  	... = ["moc", "ui", "qrc"]
///
///  Called by :
///		1. AddOnForQt::OnProjectFileRemoved(CodeBlocksEvent& _event)
///
bool Pre::isComplementFile(const wxString & _file)
{
//Mes = "isComplementFile ? -> " + _file ;
//printWarn(Mes);
//print(allStrTable("m_Registered", m_Registered));

// target name
	m_nameActiveTarget = m_pProject->GetActiveBuildTarget() ;
// if it is a virtual target, replace it with its first real target
	virtualToFirstRealTarget(m_nameActiveTarget );

	wxString filename = m_dirPreBuild + fullFilename(_file) ;
//Mes = " Pre::isComplementFile : filename = " + quote(filename);
//printWarn(Mes);
	int16_t  index = m_Registered.Index (filename);
	bool ok = index != wxNOT_FOUND;

	return ok;
}

///-----------------------------------------------------------------------------
///  Search creator file  ex : 'xxx.h'
///		format 'm_Filecreator' ->  'dircreator\xxx.h'
///
///  Called by :
///		1. AddOnForQt::OnProjectFileRemoved(CodeBlocksEvent& _event)
///
bool Pre::isCreatorFile(const wxString & _file)
{
//Mes = "isCreatorFile -> " + file ;
//printWarn(Mes);
//print(allStrTable("m_Filecreator", m_Filecreator));
	wxString dircreator = m_Filecreator.Item(0).BeforeLast(Slash)  ;
	wxString filename =  dircreator + Slash + _file;

	int16_t  index = m_Filecreator.Index (filename);
	bool ok = index != wxNOT_FOUND;
//Mes = " -> file = " + file;
//Mes += ", filename = " + filename;
//Mes += ", index = " + strInt(index);
//print(Mes);

	return ok;
}

///-----------------------------------------------------------------------------
/// Gives the name of the file to create on
///		file = xxx.h(pp) 	->	moc_xxx.cpp
///		file = xxx.ui	->	ui_xxx.h
///		file = xxx.rc	->  rc_xxx.cpp
///		file = xxx.cpp	->  xxx.moc
///
/// Called by  :
///		1. Build::buildOneFile(cbProject * _pProject, const wxString& _fcreator):1,
///		2. Build::unregisterComplementFile(const wxString & file, bool _creator, bool _first):1,
///		3. Build::addAllFiles():1,
///
wxString Pre::nameCreated(const wxString& _file)
{
	wxString name  = _file.BeforeLast('.') ;
	wxString fout ;
	if (name.Find(Slash) > 0)
	{
	// short name
		name = name.AfterLast(Slash) ;
	// before path
		fout += _file.BeforeLast(Slash) + wxString(Slash) ;
	}
	wxString ext  = _file.AfterLast('.')  ;
//1- file *.ui  (forms)
	if ( ext.Matches(m_UI) )
		fout += m_UI + "_" + name + DOT_EXT_H  ;
	else
//2- file *.qrc  (resource)
	if (ext.Matches(m_Qrc) )
		fout += m_Qrc + "_" + name + DOT_EXT_CPP  ;
	else
//3- file *.h or *hpp with 'Q_OBJECT' and/or 'Q_GADGET'
	if (ext.Matches(EXT_H) || ext.Matches(EXT_HPP)  )
		fout +=  m_Moc + "_" + name + DOT_EXT_CPP ;
	else
//4- file *.cpp with 'Q_OBJECT' and/or 'Q_GADGET'
	if (ext.Matches(EXT_CPP) )
		fout +=  name + DOT_EXT_MOC ;

	fout = fout.AfterLast(Slash) ;

	return fout  ;
}

///-----------------------------------------------------------------------------
///  Set index page to log
///
///  Called by : none
///
void Pre::SetPageIndex(int _logindex)
{
	m_LogPageIndex = _logindex;
}
///-----------------------------------------------------------------------------
/// Gives if a target is a command target
///
///	Called by :
///
///		1. Pre::isCommandTarget(const wxString& _nametarget):1,
///		2. Pre::compileableProjectTargets():1,
///		3. Pre::compileableVirtualTargets(const wxString& _virtualtarget):1,
///
bool Pre::isCommandTarget(const ProjectBuildTarget * _pBuildTarget)
{
	// virtual target ?
	if (!_pBuildTarget)		return false;

	return _pBuildTarget->GetTargetType() == ::ttCommandsOnly;
}
///-----------------------------------------------------------------------------
/// Gives if a target is a command target
///
///	Called by :
///
///  	1. Pre::detectQtTarget(const wxString& _nametarget, bool _report):1,
///		2. Pre::fullFileCreator(const wxString&  _fcreator, wxString _creatortarget):1,
///		3. AddOnForQt::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
///
bool Pre::isCommandTarget(const wxString& _nametarget)
{
	return isCommandTarget(m_pProject->GetBuildTarget(_nametarget) );
}

///-----------------------------------------------------------------------------
/// Search virtual Qt target
///
/// Called by  :
///		1. Pre::detectQtTarget(const wxString& _nametarget, bool _report):1
///
bool Pre::isVirtualQtTarget(const wxString& _namevirtualtarget, bool _warning)
{
	bool ok = m_pProject->HasVirtualBuildTarget(_namevirtualtarget);
	if (ok)
	{
		//ProjectBuildTarget * pBuildTarget ;
		wxArrayString realtargets = compileableVirtualTargets(_namevirtualtarget);
		// search the first Qt targets
		for (wxString target : realtargets)
		{
			ok = hasLibQt(m_pProject->GetBuildTarget(target));
			if (ok) break;
		}
		if (ok && _warning)
		{
			Mes = quote(_namevirtualtarget) + _("is a virtual 'Qt' target") + " !!" ;
			print(Mes);
		}
	}

	return ok  ;
}

///-----------------------------------------------------------------------------
/// Search all not compileable target for project
///	or Search all not compileable target for virtual project
///
/// Called by  :
///		1. Pre::detectQtProject(cbProject * _pProject, bool _report):1,
///
wxArrayString Pre::compileableProjectTargets()
{
	wxArrayString compilTargets;
	ProjectBuildTarget * pBuildTarget ;
	uint16_t ntarget = m_pProject->GetBuildTargetsCount();
	while (ntarget)
	{
		ntarget--;
//Mes = "indice ntarget : " + strInt(ntarget); printWarn(Mes);
		pBuildTarget = m_pProject->GetBuildTarget(ntarget);
		// virtual target ?
		if(!pBuildTarget) continue;
		// real command target ?
		if (isCommandTarget(pBuildTarget) ) continue;
		// a compileable target
		compilTargets.Add(pBuildTarget->GetTitle());
//Mes = "valid target : " + strInt(ntarget); printWarn(Mes);
	}

	return compilTargets;
}

///-----------------------------------------------------------------------------
/// Search all not compileable target for project
///	or Search all not compileable target for virtual project
///
/// Called by  :
///		1. Pre::isVirtualQtTarget(const wxString& _namevirtualtarget, bool _warning):1
///		2. Pre::virtualToFirstRealTarget(wxString& _virtualtarget, bool _warning):1
///		3. Pre::detectQtTarget(const wxString& _nametarget, bool _report):1
///		4. Pre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):1
///
wxArrayString Pre::compileableVirtualTargets(const wxString& _virtualtarget)
{
	wxArrayString compilTargets, realTargets;
	ProjectBuildTarget * pBuildTarget ;
	// search real targets : if is not a virtual target 'compilTargets' is empty
	realTargets = m_pProject->GetExpandedVirtualBuildTargetGroup(_virtualtarget);
	// the table may contain non-compileable targets
	for (wxString target : realTargets)
	{
		// search 'ttCommandOnly' type
		pBuildTarget = m_pProject->GetBuildTarget(target);
		if (!pBuildTarget) 	continue;
		// real command target ?
		if (isCommandTarget(pBuildTarget) ) continue ;

		// good target
		compilTargets.Add(pBuildTarget->GetTitle());
	}

	return compilTargets;
}

///-----------------------------------------------------------------------------
/// Detection of a 'Qt' Project : it contains at least one target Qt
///
/// Called by  :
///		1. AddOnForQt::OnActivateProject(CodeBlocksEvent& _event):1,
///		2. AddOnForQt::OnNewProject(CodeBlocksEvent& _event):1,
///		3. AddOnForQt::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
///		4. AddOnForQt::OnComplements(CodeBlocksEvent& _event):1,
///
/// Calls to :
///		1. Pre::hasLibQt(CompileTargetBase * _pContainer):2,
///		2. Pre::compileableProjectTargets():1,
///
bool Pre::detectQtProject(cbProject * _pProject, bool _report)
{
printD("=> Begin 'Pre::detectQtProject(...)");

	if (! _pProject)	return false;

// libraries Qt in project and targets
	m_pProject = _pProject;
	bool isQtProject =  hasLibQt(m_pProject), isQtTarget = false;
//Mes = "isQtProject = " + strBool(isQtProject); printWarn(Mes);
	// search in compileable targets
	if (!isQtProject)
	{
		//wxArrayString compiletable = compileableProjectTargets();
//Mes = allStrTable("compiltable", compiletable); printWarn(Mes);
		for( wxString target: compileableProjectTargets())
		{
			isQtTarget = hasLibQt(m_pProject->GetBuildTarget(target));
//print("isQtTarget => " + strBool(isQtTarget) );
			if (isQtTarget)
				break;
		}
	}

	if (isQtProject || isQtTarget)
	{
		if (_report)
		{
			wxString nametarget = m_pProject->GetTitle() ;
			wxString title = _("The project") +  quote(m_nameProject) + _("uses Qt libraries !") ;
			wxString txt =  quote( m_namePlugin ) + _("will generate the complements files") + "..." ;
			// also to Code::Blocks log
			InfoWindow::Display(title, txt, 5000);
		}
		// usemake ?
		if( m_pProject->IsMakefileCustom())
		{
			Mes = "... " ;
			Mes += _("but please, DISABLE using of custom makefile");
			Mes += Lf + quote(m_namePlugin) + _("not use makefile.");
			print(Mes);
			Mes += Lf + _("CAN NOT CONTINUE") ;
			Mes += " !" ;
			wxString title = _("Used makefile");
			title += " !!";
			cbMessageBox(Mes, title, wxICON_WARNING ) ;

			Mes = m_namePlugin + " -> " + _("end") + " ...";
			printWarn(Mes);
			isQtProject = false;
		}
	}


printD("	<= End 'Pre::detectQtProject(...)");

	return (isQtProject || isQtTarget);
}

///-----------------------------------------------------------------------------
/// If virtual target give the real fist target
///
/// Called by  :
///		1. Build::findGoodfiles():1,
///		2. Pre::isComplementFile(const wxString & _file):1,
///
bool Pre::virtualToFirstRealTarget(wxString& _virtualtarget, bool _warning)
{
	// verify if it'a a virtual target
	if (!m_pProject->HasVirtualBuildTarget(_virtualtarget)) return false;
	// the first real target not command target
	_virtualtarget = compileableVirtualTargets(_virtualtarget).Item(0);
	if (_warning) {
			Mes = quote( _virtualtarget );
			Mes += _("it is a virtual target");
			Mes += ", ";
			Mes += _("that is replaced by his first real target") ;
			Mes += quote( _virtualtarget);
			printWarn(Mes);
	}
//Mes = " _virtualtarget =  " + quote(_virtualtarget);
//print(Mes);

	return !_virtualtarget.IsEmpty();
}

///-----------------------------------------------------------------------------
/// Detection of a 'Qt' target
///
/// Called by  :
///		1. Pre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report):1,
///		2. AddOnForQt::OnActivateTarget(CodeBlocksEvent& _event):1,
///		3. AddOnForQt::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
///		4. AddOnForQt::OnNewProject(CodeBlocksEvent& _event):1,
///
/// Calls to :
///		1. Pre::hasLibQt(CompileTargetBase * _pContainer):1,
///
bool Pre::detectQtTarget(const wxString& _nametarget, bool _report)
{
	bool ok = false ;
	if(!m_pProject)  return ok;
	if (isCommandTarget(_nametarget)) 	return ok;

//Mes = "qtpre::detectQtTarget -> " + quote( _nametarget ) ; printWarn(Mes);
	// is one virtual target
	bool virtQt = isVirtualQtTarget(_nametarget);
	wxArrayString compilTargets;	//
	if (virtQt)
	{
		compilTargets =  compileableVirtualTargets(_nametarget);
		if (_report)
		{
			Mes = Tab + quote("::" + _nametarget);
			Mes += _("is a virtual 'Qt' target that drives") ;
			Mes += " ..."; printWarn(Mes);
		}
	}
	else
		compilTargets.Add(_nametarget) ;

/// all compileable targets
	bool goodQt = false;
	for (wxString target : compilTargets)
	{
//Mes = "compare : " + quote( target) +  ", " + quote( _nametarget);
//print(Mes);
		ok = hasLibQt(m_pProject->GetBuildTarget(target));
//Mes = strBool(ok) ; printError(Mes);
		if (_report)
		{
			if (virtQt) 	Mes = Tab ;
			else			Mes = wxEmptyString;
		//Mes += "qtpre::detectQtTarget ->";
			Mes += Tab + quote( "::" + target);
		// advices
			Mes += Tab + Tab + _("is") + Space;
			if(!ok)		Mes += _("NOT") + Space;
			Mes += _("a Qt target.");
			print(Mes);
		}
		goodQt = goodQt || ok ;
	}
//Mes = strBool(goodQt) ; printError(Mes);

	return goodQt;
}

///-----------------------------------------------------------------------------
/// Return true if good  'CompileTargetBase* container'
///
/// Called by  :
///		1. Pre::detectQtProject(cbProject * _pProject, bool _report = false):1,
///		2. Pre::detectQtTarget(const wxString& _nametarget, bool _report):1,
///
bool Pre::hasLibQt(CompileTargetBase * _pContainer)
{
//Mes = "into 'Pre::hasLibQt(...)'" ; printWarn(Mes);
	bool ok = false;
	if (!_pContainer) 	return ok;

	wxArrayString tablibs = _pContainer->GetLinkLibs() ;
	uint16_t nlib = tablibs.GetCount() ;
//Mes = "nlib = " + strInt(nlib); printWarn(Mes);
	if (nlib > 0 )
	{
		wxString namelib ;
		uint16_t u=0;
		int16_t index= -1, pos ;
		while (u < nlib && !ok )
		{
			// lower, no extension
			namelib = tablibs.Item(u++).Lower().BeforeFirst('.') ;
//Mes = strInt(u) + " -> " + quote( namelib ); printWarn(Mes);
			// no prefix "lib"
			pos = namelib.Find("lib") ;
			if (pos == 0)
				namelib.Remove(0, 3) ;
			// begin == "qt"
			pos = namelib.Find("qt") ;
//Mes = "pos 'qt' = " + strInt(pos); printWarn(Mes);
			if (pos != 0) 		continue ;
			// find
/// ATTENTION : the table should then contain all Qt libraries !!
			index = m_TablibQt.Index(namelib);
//Mes = "index = " + strInt(index); printWarn(Mes);
			ok = index != -1 ;
			// first finded
			if (ok)
				break;
		}
	}
//Mes = "haslibQt(...) = " + strBool(ok); printWarn(Mes);

	return ok;
}

///-----------------------------------------------------------------------------
/// Detects already existing complement files in the project and
///	populates complement tables with existing files
///
/// Called by :
///		1. AddOnForQt::OnActivateProject(CodeBlocksEvent& _event):1,
///		2. AddOnForQt::OnRenameProjectOrTarget(CodeBlocksEvent& _event):2,
///		3. AddOnForQt::OnComplements(CodeBlocksEvent& _event):1,
///
/// Calls to :
///		1. Pre::refreshTables():1,

bool Pre::detectComplementsOnDisk(cbProject * _pProject, const wxString & _nametarget,  bool _report)
{
printD("=> Begin 'Pre::detectComplementsOnDisk(..., " + _nametarget + ", " + strBool(_report) + ")'" );
/// DEBUG
//* *********************************************************
//	beginDuration("Pre::detectComplementsOnDisk(...)");
//* *********************************************************
	m_dirProject = _pProject->GetBasePath();
	wxFileName::SetCwd (m_dirProject);

	bool ok = wxDirExists(m_dirPreBuild);
//Mes = "m_dirPreBuild = " + quote( m_dirPreBuild ) + ",  ok = ") +  strBool(ok);
//print(Mes);
	// it's an old project already compiled once
	if (ok)
	{
		wxArrayString filesdisk; wxArrayString compilTargets;
		wxString nametarget; bool strangers = false;
	// deregister all old complement
		m_Filewascreated.Clear();
	// register all complement files on the disk in 'filesdisk'
		size_t n = wxDir::GetAllFiles(m_dirPreBuild, &filesdisk);
		if (!n) return false;

	// read directory names = target name of project
		for (wxString filepath : filesdisk)
		{
//Mes = quote(filepath); print(Mes);
			// extract target name
			nametarget = filepath.AfterFirst(Slash).BeforeFirst(Slash);
//Mes = quote(nametarget); print(Mes);
			// is a target of project ?
			if (_pProject->GetBuildTarget(nametarget) )
			{
				m_Filewascreated.Add(filepath) ;
			}

			else	strangers = true;
		}
		ok = m_Filewascreated.GetCount() > 0;
//Mes = " n = " + strInt(n) ; print(Mes);
		if (ok)
		{
		// initializes all other tables
			refreshTables();
			// for debug
			//refreshTables(WITH_DEBUG);
		// we have to do it for compileable targets Qt
			bool virt = m_pProject->HasVirtualBuildTarget(_nametarget);
			if (virt)
				compilTargets = compileableVirtualTargets(_nametarget);
			else
				compilTargets.Add(_nametarget);

			bool isQtTarget;
			wxString diradding; wxArrayString  filewascreated;
			for (wxString target : compilTargets)
			{
				isQtTarget = detectQtTarget(target, NO_REPORT) ;
	//	Mes = "target = " + quote(target) + ", isQtTarget:" + strBool(isQtTarget);
	//	print(Mes);
				if (!isQtTarget) 	continue;

				diradding = m_dirPreBuild + target ;
	//	Mes = "diradding  = " + quote( diradding  );
	//	printWarn(Mes);
				ok = wxDirExists(diradding);
				if (ok)
				{
					n = wxDir::GetAllFiles(diradding, &filewascreated );
					if (_report)
					{
						if (virt)	Mes = Tab;
						else 		Mes = wxEmptyString;
						Mes +=  Tab + quoteNS("::" + target);
						// tabulation to SizeLe (=16)
						size_t le = Mes.Len();
						if (le <= SizeLe)	Mes.Append(' ', SizeLe-le);
						// advices
						Mes += Tab +  _("is") + Space;
						if(!ok)		Mes += _("NOT") + Space;
						Mes += _("a Qt target") ;
						Mes += ", ";
						Mes +=  _("with") + Space;
						if (n)
						{
							Mes += _("has") + Space + strInt(n) + Space ;
							Mes += _("complement file(s) already created on disk in") ;
							Mes += quote( diradding );
						}
						else
						{
							Mes += _("no complement file to disk.") ;
						}
						print(Mes);
					}
				}
				else
				if (_report)
				{
					Mes =  _("The directory") + quote(diradding);
					Mes += _(" no exists")  +  Dot ;
					Mes += Space + _("It's mandatory to 'Rebuild' the target") ;
					Mes += " !!"; printWarn(Mes) ;
				}
			} // end for
		}
		else{
			Mes =  Tab+ "=> " + _("The directory") + quote(m_dirPreBuild);
			Mes += _("is empty")  + Dot +  Space ;
			Mes += _("You need to 'Rebuild' the project")  +  " !!";
			printWarn(Mes) ;
		}
		if (strangers)
		{
			Mes = "	==>";
			Mes += quote( _pProject->GetTitle()) + _("has stranger complement files in");
			Mes += quote(m_dirPreBuild);
			Mes += "==> "  + _("you can delete it from the disk");
			printError(Mes);
		}
	}
	else
	// it's a new project or people has deleted 'm_dirPreBuild' !
	if (_report)
	{
		//Mes = "Pre::detectComplementsOnDisk -> ");
		Mes =  Tab+ "=> " + _("The directory of complements does not exist") ;
		Mes += " ! ,";
		Mes += _("you need to 'Rebuild' the project") ;
		Mes += " !!";
		printWarn(Mes);
	}

/// DEBUG
//* *******************************************************
//	endDuration("Pre::detectComplementsOnDisk(...)");
//* *******************************************************
	Mes.Clear();

printD("	<= End 'Pre::detectComplementsOnDisk(...) => " + strBool(ok) );

	return ok;
}

///-----------------------------------------------------------------------------
///	Refresh all the tables
///
///	Called by :
///		1. Pre::detectComplementsOnDisk(cbProject * _pProject
///								, const wxString & çnametarget,  bool çreport):1,
///
///	Calls to :
///		1. Pre::copyArray (const wxArrayString& _strrarray, int _nlcopy = 0):2,
///		2. Pre::wasCreatedToCreator(const wxArrayString& _strarray):1,
///
bool Pre::refreshTables(bool _debug)
{
/// DEBUG
//* ***********************************************
//	beginDuration("Pre::refreshTables(...)");
//* ***********************************************
	bool ok = true;

	m_Registered.Clear();
	m_Registered = copyArray(m_Filewascreated ) ;
///
	m_Createdfile.Clear();
	m_Createdfile = copyArray(m_Filewascreated ) ;
///
	m_Filecreator.Clear();
	m_Filecreator = wasCreatedToCreator() ;
///
	m_Filestocreate.Clear();
	for (wxString item :  m_Filewascreated)
	{
		m_Filestocreate.Add(m_Devoid);
	}
	if (_debug)
	{
		// DEBUG
		Mes = allStrTable("m_Filewascreated", m_Filewascreated);
		print(Mes);

		Mes = allStrTable("m_Registered", m_Registered);
		print(Mes);

		Mes = allStrTable("m_Createdfile", m_Createdfile);
		print(Mes);

		Mes = allStrTable("m_Filecreator", m_Filecreator);
		print(Mes);

		Mes = allStrTable("m_Filestocreate", m_Filestocreate);
		print(Mes);
	}

/// DEBUG
//* *********************************************
//	endDuration("Pre::refreshTables(...)");
//* *********************************************
	Mes.Clear();

	return ok;
}
///-----------------------------------------------------------------------------
/// check that the filename not contain illegal characters for windows !
///
/// called by :
///		Pregen::BuildModuleMenu(const ModuleType type, wxMenu* menu, const FileTreeData* data):1,
/// calls :
///
int Pre::filenameOk(wxString & _namefile)
{
printD("=> Begin 'Pre::filenameOk(" + _namefile + ")'" );
	// all blanks -> "____"
	int nrep =  _namefile.Replace(" ", "____" , true ) ;
	/// characters list
	wxString illegal = "[:*?\"<>|]";
	size_t ncar = illegal.Len();
	wxString car;
/// all illegal characters
	for (int u = 0; u < ncar; u++)
	{
		car = illegal.Mid(u, 1);
		if (_namefile.Find(car) != 1)
		{ /// replace all illegal character  by " "
			_namefile.Replace(car, " ", true);
		}
	}
	/// illegal characters ?
	ncar = _namefile.Replace(" ", "" , true );
	/// change
	if (nrep)
	{
		_namefile.Replace("____", " " , true );
	}

printD("	<= End 'Pre::filenameOk(...)' => " + strInt(ncar) + " illegal characters" );

	return ncar ;
}

///-----------------------------------------------------------------------------
/// Copy a 'wxArrayString' to an another
///
/// Called by  :
///		-# Build::filesTocreate(bool _allrebuild):1,
///		-# Build::createFiles():4,
///		-# Pre::refreshTables(bool debug):2,
///
wxArrayString Pre::copyArray (const wxArrayString& _strarray, int _nlcopy)
{
	wxArrayString tmp ;
	int nl = _strarray.GetCount()  ;
	if (!nl)
		return  tmp ;
    // adjust number of lines to copy
	if (_nlcopy > 0 && _nlcopy <= nl)
		nl = _nlcopy;
	// a line
	wxString line;
	for (int u = 0; u < nl; u++)
	{
	// read strarray line
		line = _strarray.Item(u) ;
	// write line to tmp
		tmp.Add(line, 1) ;
	}

	return tmp ;
}

///-----------------------------------------------------------------------------
/// Give a string from a 'wxArrayString' for debug
///
/// Called by  :  for debug
///
wxString Pre::allStrTable(const wxString& _title, const wxArrayString& _strarray)
{
//Mes = "Pre::allStrTable ...";
//printError(Mes);
	wxString mes = "--------- debug --------------";
	mes += Lf + quote( m_dirProject ) ;
	mes += Lf +"=>" + quote( _title ) ;
	uint16_t nl = _strarray.GetCount();
	mes += " : " + strInt(nl) + Space + _("files") ;
	for (uint16_t u = 0; u < nl; u++)
	{
	// read strarray line from  '1' ... 'nl'
		mes += Lf + strInt(u+1) + "- " + _strarray.Item(u) ;
	}
	mes +=  Lf + "--------- end  debug ------------" ;

	return mes;
}

///-----------------------------------------------------------------------------
/// For create 'm_FilesCreator' from 'm_Filewascreated'
/// reverse a 'wxArrayString' to an another
///
/// Called by  :
///		1. Pre::refreshTables():1,
///
///	Calls to :
///		1. Pre::toFileCreator(const wxString& _fcreated):n,
///
wxArrayString Pre::wasCreatedToCreator()
{
// read file list to 'm_Filewascreated' with (moc_....cpp, ui_....h, qrc_....cpp, __Devoid__)
// write file list to 'tmp' with  (*.h, *.cpp, *.qrc, *.ui)
	wxArrayString tmp;
	wxString fullnameCreator;
	for (wxString fcreated: m_Filewascreated)
	{
		fullnameCreator = toFileCreator(fcreated)  ;
//Mes = "	fcreated = " + quote(fcreated) + " => " +  quote(fullnameCreator);
//printWarn(Mes);
	//add to array
		tmp.Add(fullnameCreator);
	}

	return tmp;
}
///-----------------------------------------------------------------------------
/// For create  'creator files' from  'created files'
/// reverse a (string' to an another
///
/// Called by  :
///		1. Pre::wasCreatedToCreator():1,
///
///	Calls to :
///		1. Pre::fullFileCreator(const wxString& _fcreator):1,
///
wxString Pre::toFileCreator(const wxString &_fcreated)
{
	wxString target, fcreated, fcreator , prepend, ext, name, fullname;
//Mes = " ::toFileCreator :: _fcreated = " + quote(_fcreated ) ; printWarn(Mes);
	target = _fcreated.AfterFirst(Slash).BeforeFirst(Slash);
//Mes = " target = " + quote(target) ; printWarn(Mes);
	fcreated = _fcreated.AfterLast(Slash);
//Mes = " fcreated = " + quote(fcreated ) ; printWarn(Mes);
	if (fcreated.Contains("_") )
	{
		prepend = fcreated.BeforeFirst('_') ;
		name = fcreated.AfterFirst('_') ;
	}
	/// 'xxx.moc'
	else
	{
		name = fcreated;
	}
//Mes = " 		prepend = " + quote(prepend) ; printWarn(Mes);
//Mes = " 		name = " + quote(name) ; printWarn(Mes);
	name = name.BeforeFirst('.');
	ext = fcreated.AfterLast('.');
//1- file 'ui_uuu.h'  (forms)  -> 'uuu.ui'
	if ( prepend.Matches(m_UI) && ext.Matches(EXT_H))
		fcreator =  name + DOT_EXT_UI ;
	else
//2- file 'qrc_rrr.cpp'  (resource) -> 'rrr.qrc'
	if (prepend.Matches(m_Qrc) )
		fcreator = name + DOT_EXT_QRC  ;
	else
//4- file 'moc_mmm.cpp'  -> 'mmm.h'
	if (prepend.Matches(m_Moc) )
		fcreator =  name + DOT_EXT_H ;
	else
//5- file 'xxx.moc'  -> 'xxx.cpp'
	if (prepend.IsEmpty() && ext.Matches(m_Moc) )
		fcreator =  name + DOT_EXT_CPP ;
//6- full name
	fullname = fullFileCreator(fcreator, target);
//Mes = " fcreator = " + quote( fcreator ) ;
//Mes += " ->  fullname = " + quote( fullname ) ;
//printWarn(Mes);

	return fullname;
}

///-----------------------------------------------------------------------------
/// Gives a relative to common top level path of 'fcreator';
///		example 'parserfile.h' => 'src/parser/parserfile.h'
///
/// Called by  :
///		1. Pre::toFileCreator(const wxString& _fcreated):1,
///
wxString Pre::fullFileCreator(const wxString&  _fcreator, wxString _creatortarget)
{
	wxString fullname = wxEmptyString ;
	if (!m_pProject)
		return fullname;
//Mes = Tab + " ==> search _fcreator = " + quote( _fcreator ) ;
//Mes += " of target :" + quote(_creatortarget);
//print(Mes);
	ProjectFile * prjfile ;
	bool good = false;
	// all files of the project
	for (uint16_t  nf = 0; nf < m_pProject->GetFilesCount() ; nf++)
	{
		// get a file of the project
		prjfile = m_pProject->GetFile(nf);
        if (!prjfile) 	continue;
        // search in all the real targets of the file
		if (prjfile->GetBuildTargets().Index(_creatortarget) == wxNOT_FOUND)	continue;
		// is a command target : it's possible ?
		if (isCommandTarget(_creatortarget) )	continue;

		// read the relative file name
		fullname = prjfile->relativeToCommonTopLevelPath ;
		// we found the file ?
		good = fullname.AfterLast(Slash).Matches(_fcreator);
		if (good)
			break;
	}

	if (!good)
	{
		Mes = "**" + quote(_creatortarget) + ":" + quote( _fcreator ) ;
		Mes += Space + _(" which is a creator file is missing in the CB project") + " !";
		printError(Mes);
		fullname = wxEmptyString ;
		/// ask if we should delete it!
		/// TODO

	}
//Mes = _(" => fullname = ") + quote( fullname);print(Mes);

	return fullname;
}

///-----------------------------------------------------------------------------
