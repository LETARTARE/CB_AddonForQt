/***************************************************************
 * Name:      creater.cpp
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2022-04-22
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#include "creater.h"

#include <manager.h>
#include <cbproject.h>
#include <compiletargetbase.h>
#include <projectmanager.h>
#include <macrosmanager.h>
#include <editormanager.h>
#include <cbeditor.h>
#include <wx/datetime.h>
#include <wx/dir.h>
#include <wx/filename.h>
#include <projectfile.h>
// not change  !
#include "defines.h"
///-----------------------------------------------------------------------------
/// Constuctor

/// Called by :
///		1. AddOnForQt::OnAttach():1,
///
Creater::Creater(const wxString & _namePlugin, int _logIndex)
	: Pre(_namePlugin, _logIndex)
{
/// !! not use  '+ Slash'   !!  because Slash = wxT(...)
	m_dirPreBuild = "adding" + wxString(Slash) ;
}
///-----------------------------------------------------------------------------
/// Destructor

/// Called by :
///		1. AddOnForQt::OnRelease():1,
///
Creater::~Creater()
{
}

///-----------------------------------------------------------------------------
///  Creater create start banner
///
/// Called by :
///		1. Creater::buildAllFiles(cbProject * _pProject, bool _workspace, bool _allbuild):1,
/// Calls to :
///		1. Pre::date():1,
///
void Creater::beginMesBuildCreate()
{
// begin
	if(!m_pProject)
		return;

// base directory
	m_dirProject = m_pProject->GetBasePath() ;
// directory changed
	wxFileName::SetCwd (m_dirProject);
	bool ok = createDir (m_dirPreBuild) ;
	if (!ok)
		return ;

	m_nameProject = m_pProject->GetTitle();
	m_nameActiveTarget = m_pProject->GetActiveBuildTarget();
// for test
//m_pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget);

// display log
	Mes = "-------------- " ;
	Mes += "PreBuild :" ;
	Mes += quote( m_nameActiveTarget );
	Mes += _("in");
	Mes += quote( m_nameProject );
	Mes += "-------------- " ;
	_printWarn(Mes);
// date
	Mes = _("started on");
	Mes += " : " + date() ;
	_print(Mes);
// for duration
	m_start = clock();
	Mes.Clear();
}

///-----------------------------------------------------------------------------
///  Creater create end banner
///
/// Called by :
///		1. Creater::buildAllFiles(cbProject * _pProject, bool _workspace, bool _allbuild):1,
/// Calls to :
///		1. Pre::date():1,
///		2. Pre::duration():1,
///
void Creater::endMesBuildCreate()
{
// date and duration
	Mes = _("ended on") ;
	Mes += " : " + date() ;
	_print(Mes) ;
	Mes =  _("duration") ;
	Mes += " = " + duration() ;
	_printWarn(Mes);
	Mes.Clear();
}
///-----------------------------------------------------------------------------
/// Generating the complements files...
///
/// Called by :
///		1. AddOnForQt::doComplements(const cbFutureBuild&  _domake):1
///
/// Calls to :
///		1. Creater::beginMesBuildCreate():1,
///		2. Creater::findGoodfiles():1,
///		3. Creater::addAllFiles():1,
///		4. Creater::filesTocreate(bool _allrebuild) :1,
///		5. Creater::createFiles():1,
///		6. Creater::validCreated():1,
///		7. Creater::endMesBuildCreate():1,
///
bool Creater::buildAllFiles(cbProject * _pProject, cb_unused bool _workspace, bool _allbuild)
{
	if (!_pProject) return false;

/// DEBUG
//* ****************************************
//	beginDuration("buildAllFiles(...)");
//* ****************************************
// begin banner
//	beginMesBuildCreate();
	m_pProject = _pProject;
	bool ok = false;
	m_clean = false;
	m_abort = false;
///***************************************
///1- find good target with eligible files
///***************************************
	// analyzing all project files
	Mes = _("Search creator file(s)") ;
	Mes += " ..." ;
	_printWarn(Mes);
	int nelegible =  findGoodfiles() ;
	if (nelegible > 0)
	{
///******************************************
///2-  registered all files in 'm_Registered'
///******************************************
		int nfiles = addAllFiles() ;
		Mes = Tab + "-> " + strInt(nfiles) + Space ;
		Mes += _("creator(s) file(s) registered in the plugin") ;
		_print(Mes) ;
		if (nfiles > 0)
		{
///****************************************
///3-  files to create to 'm_Filestocreate'
///****************************************
			wxUint16 ntocreate = filesTocreate(_allbuild) ;
			if (ntocreate == 0 )
				ok = !_allbuild ;
			else
				ok = true;
			if (ok)
			{
///********************************************************************
///4- create files
/// 	from 'FileCreator', 'm_Registered'
/// 	1- adds files created in 'm_Createdfile'
/// 	2- create additional files as needed  by 'create(qexe, fileori)'
///*********************************************************************
				Mes = _("Creating complement file(s)") + " ..." ;
				_printWarn(Mes);

				bool good = createFiles() ;
				if (good)
				{
                    Mes = "... "  + _("End creating complement file(s)") ;
                    _printWarn(Mes);

///**************************
///5- register files created
///**************************
					good = validCreated() ;
					if (!good)
					{
						Mes = Tab + _("No file to create.") ;
						_printWarn(Mes);

					}
					ok = true;
				} // end (good)
				else
				if (! m_abort)
				{
					Mes = Tab + _("Complement file creation error") + " !!" ;
					_printError(Mes);
				}
				ok = good;
			}	//end (ntocreate > 0)
			else
			{
				Mes = Tab + _("File to create error") + " !!" ;
				_printError(Mes);
			}
		} // end (nfiles > 0)
		else
		{
			Mes = Tab + _("File registration error") + " !!" ;
			_printError(Mes);
		}
	} // end (nelegible > 0)
	else
	{
		Mes = Tab + _("No elegible file (with 'Q_OBJECT' or 'Q_GADGET')") + " !!" ;
		_printWarn(Mes);
		ok = true;
	}

/// debug
//_print(allStrTable("m_Registered", m_Registered));
/// <=

    m_clean = m_Registered.IsEmpty();

/// DEBUG
//* **************************************
//	endDuration("buildAllFiles(...)");
//* **************************************
	Mes.Clear();

	return ok;
}
///-----------------------------------------------------------------------------
///  Creater create one file start banner
///
/// Called by :
///		1. Creater::buildOneFile(cbProject * _pProject, const wxString& _fcreator) :1,
/// Calls to :
///		1. Pre::date():1,
///
void Creater::beginMesFileCreate()
{
// banner
	_printLn;
	Mes = "-------------- " ;
	Mes += "PreCompileFile :" ;
	Mes += quote( m_pProject->GetActiveBuildTarget() );
	Mes += _("in");
	Mes += quote( m_pProject->GetTitle() );
	Mes += " :" + quote( m_filename );
	Mes += "-------------- " ;
	_printWarn(Mes);
// date
	Mes = _("started on : ") + date() ;
	_print(Mes);
// for duration
	m_start = clock();
	Mes.Clear();
}
///-----------------------------------------------------------------------------
///  Creater create one file end banner
///
/// Called by :
///		1. Creater::buildOneFile(cbProject * _pProject, const wxString& _fcreator) :1,
/// Calls to :
///		1. Pre::date():1,
///		2. Pre::duration():1,
///
void Creater::endMesFileCreate()
{
// date and duration
	Mes = _("ended on : ") + date() ;
	_print (Mes) ;
	Mes =  _("duration = ") + duration() ;
	_printWarn(Mes);
	Mes.Clear();
}
///-----------------------------------------------------------------------------
/// Generating only ONE complement file...
///
///  Called by :
///		1. AddOnForQt::OnComplements(CodeBlocksEvent& _event):1,
///
///  Calls to :
///		1.	Creater::beginMesFileCreate():1,
///		2.	Creater::isElegible(const wxString& _file):1,
///		3.	Creater::inProjectFileCB(const wxString& _file):1,
///		4.	Creater::compareDate(const wxString&  _fileref
///									, const wxString&  _filetarget):1
///		5.  Creater::createFileComplement(const wxString& _qexe
///							, const wxString& _fcreator, const wxString& _fout):1,
///		6.	Creater::endMesFileCreate():1,
///

bool Creater::buildOneFile(cbProject * _pProject, const wxString& _fcreator)
{
	if(!_pProject)		return false;
_printD("=> Begin Creater::buildOneFile(..., " + quote(_fcreator) + ")'" );

    m_pProject = _pProject;
    m_nameActiveTarget = m_pProject->GetActiveBuildTarget();
	if (m_nameActiveTarget.IsEmpty())
        return false;

	ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget)  ;
	if (!pBuildTarget)
        return false ;

	m_filename = _fcreator;
// directory changed
	m_dirProject = m_pProject->GetBasePath();
	wxFileName::SetCwd (m_dirProject);
	bool ok = createDir (m_dirPreBuild) ;
//_print("ok =" + strBool(ok) );
	if (!ok)	return ok;

// file it is correct ? already tested  in 'AddOnForQt::BuildModuleMenu(...)'
    bool elegible = isElegible(_fcreator);
//_print("elegible =" + strBool(elegible) );
    if (!elegible)		return elegible ;

// begin pre-Compile
	beginMesFileCreate() ;
//_print(" target name :" + quote(pBuildTarget->GetTitle()) );
// init Qt tools
	ok = findTargetQtexe(pBuildTarget) ;
//_print("ok =" + strBool(ok) );
	if (!ok)	return ok ;

	m_nameActiveTarget =  m_pProject->GetActiveBuildTarget() ;
//_print("m_nameActiveTarget =" + quote(m_nameActiveTarget) );
///-----------------------------------------------------------------------------
// complete path
     wxString fout = complementDirectory() + nameCreated(_fcreator) ;
//Mes = Tab + quote( _fcreator + quote( " -> " ) + fout ) ; _print(Mes);
// already registered
    bool inproject = inProjectFileCB(fout) ;
    if (!inproject)
    {
    // has included ?
        wxString extin = _fcreator.AfterLast('.') ;
        wxString extout = fout.AfterLast('.')  ;
        bool nocompile = false ;
        if ( extin.Matches(EXT_H) && extout.Matches(EXT_CPP) )
            nocompile =  hasIncluded(_fcreator) ;
    // add file : AddFile(Nametarget, file, compile, link, weight)
        ProjectFile* prjfile = m_pProject->AddFile(m_nameActiveTarget, fout, !nocompile, !nocompile, 50);
        if (!prjfile)
        {
        // display message
            Mes  = "===> " ;
            Mes += _("can not add this file ");
            Mes += quote( fout ) + _(" to target ") + m_nameActiveTarget ;
            _printError (Mes) ;
            cbMessageBox(Mes, "AddFile(...)", wxICON_ERROR) ;
        }
    // display
        if (nocompile)
        {
            Mes = Tab + _("Add ") + quote( fout ) ;
            Mes += Lf + Tab + Tab + "*** " + _("This file is included, ") ;
            Mes += _("attributes 'compile' and 'link' will be set to 'false'") ;
            _print(Mes);
        }
	// svn 9501 : CB 13.12  and >
	// verify if complements exists already
         m_pMprj->GetUI().RebuildTree() ;

    // create complement ...
        m_Identical = false;
    }
    else
    {
//_print("not registered in project...");
    // Check the date
        m_Identical = false ;
        //- verify datation on disk
        if (::wxFileExists(fout))
            m_Identical = compareDate(_fcreator, fout) ;
    }
    if (m_Identical)
    {
        Mes = Tab + _("Nothing to do (complement file exist on disk)") ;
        _print(Mes) ;
    }
// ! identical date -> create
    else
///-----------------------------------------------------------------------------
    {
        Mes = Tab + _(" One complement file is created in the project ...") ;
        _printWarn(Mes);
    // create file complement with 'moc'
        wxString strerror = createFileComplement(m_Mexe, _fcreator, fout);
// _print("buildOneFile::strerror = " + quote(strerror)) ;
        if (!strerror.IsEmpty())
        {
        // error message
            wxString title = _("Creating ") + quote( fout ) ;
            title += _(" failed") ;
            title += " ...";
		//1- error create directory  :
			// = _("Unable to create directory ")
		//2- error buildtarget no exist :
			// = quote( m_nameActiveTarget ) + _(" does not exist !!") ;
		//3- error create complement
            Mes =  "=> " ;
            Mes += strerror.BeforeLast(Lf.GetChar(0)) ;
            _printError (Mes) ;
            cbMessageBox(Mes, title) ;
        }
    }
/*  TO REVIEW ...
	// targets no virtual list
	wxArrayString rltargets;
// virtual target ?  without warning
	bool virt = m_pProject->HasVirtualBuildTarget(m_nameActiveTarget);
	if (virt)
		rltargets = compileableVirtualTargets(m_nameActiveTarget);
	else
		rltargets.Add(m_nameActiveTarget);

	wxUint16 ntargets = rltargets.GetCount();
	ProjectBuildTarget* pBuildTarget;
	wxString fout;
    for (wxUint16 nt = 0 ; nt < ntargets ; nt++ )
    {
		m_nameActiveTarget = rltargets.Item(nt);
		// all targets are reals
		pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget);
		if (isCommandTarget(pBuildTarget) ) 	continue;

		.... see code before
	}
*/

// end preCompile
	endMesFileCreate() ;

	Mes.Clear();

_printD(" <= End Creater::buildOneFile(...) => "  + strBool(elegible) );

    return elegible;
}
///-----------------------------------------------------------------------------
/// Cleaning only ONE complement file...
///
///  Called by :
///		1. AddOnForQt::OnComplements(CodeBlocksEvent& _event):1,
///
///  Calls to :
///		1.	Creater::beginMesFileCreate():1,
///		2.	Creater::isElegible(const wxString& _file):1,
///		3.	Creater::inProjectFileCB(const wxString& _file):1,
///		4.	Creater::compareDate(const wxString&  _fileref
///									, const wxString&  _filetarget):1
///		5.  Creater::createFileComplement(const wxString& _qexe
///							, const wxString& _fcreator, const wxString& _fout):1,
///		6.	Creater::endMesFileCreate():1,
///
bool Creater::cleanOneFile(cbProject * _pProject, const wxString& _fcreator)
{
	if(!_pProject)		return false;

_printD("=> Begin Creater::buildOneFile(..., " + quote(_fcreator) + ")'" );

	bool ok = false;
	m_pProject = _pProject;
	ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget)  ;
	if (!pBuildTarget)	return false ;

	m_filename = _fcreator;
	/// ......... TO FINISH

_printD("	<= End Creater::cleanOneFile(...) => "  + strBool(ok) );
	return ok;
}

///-----------------------------------------------------------------------------
/// Find path Qt container  ->  'CompileOptionsBase * container'
///
/// Called by  :
///		1. Creater::findTargetQtexe(CompileTargetBase * _buildtarget):1,
///
wxString Creater::pathlibQt(CompileTargetBase * _pContainer)
{
_printD("=> Begin 'Creater::pathlibQt(...)");
	wxString path;
	if (!_pContainer) 	return path;
//_print("Name container : " + quote(_pContainer->GetTitle()) );

	wxArrayString tablibdirs = _pContainer->GetLibDirs() ;
	int npath = tablibdirs.GetCount() ;
//Mes = "path lib number = " + strInt(npath); _printError(Mes);
	if (!npath)
		return path;

	wxString  path_nomacro ;
	if (npath > 0 )
	{
		bool ok = false ;
		int u = 0  ;
		while (u < npath && !ok )
		{
			path = tablibdirs.Item(u++);
			path.Lower();
		// used local variable ?
			ok = path.Find("$qt") != -1 ;
			if (ok)
			{
//Mes = Lf + _("$qt => ") + quote( path ); _printWarn(Mes);
				m_pMam->ReplaceEnvVars(path) ;
//Mes = Lf + _("Local variable path Qt => '") + path; Mes += "'"; _print(Mes);
				path_nomacro =  path ;
			// remove "\lib"
				path_nomacro = path_nomacro.BeforeLast(Slash) ;
				path_nomacro += wxString(Slash)  ;
			}
			else
			{
			// use global variable ?
				ok = path.Find("#qt") != -1 ;
				if (ok)
				{
//	debug qt
//Mes = Lf + _("#qt => ") + quote( path ); _printWarn(Mes);
					m_pMam->ReplaceMacros(path) ;
//Mes = Lf + _("Global variable path Qt => '") + path;Mes += "'"; _print(Mes);
					path_nomacro =  path ;
				// remove "\lib"
					path_nomacro = path_nomacro.BeforeLast(Slash) ;
					path_nomacro += wxString(Slash)  ;
				}
			// no variable ! , absolute path ?
				else
				{
//Mes = Lf + _("Path Qt => '") + path ; Mes += "'";	_printWarn(Mes);
					path_nomacro += path;
				}
			}
		}
	}
	Mes.Clear();
_printD("	<= End 'Creater::pathlibQt(...) => " + quote(path_nomacro) );

	return path_nomacro  ;
}

///-----------------------------------------------------------------------------
/// Find path Qt exe : 'ProjectBuildTarget* _pBuildTarget'
///
/// Called by  :
///		1.	Creater::createFiles():1
///		2.	Creater::buildOneFile(cbProject * _pProject, const wwxFileExistsxString& _fcreator):1
///

bool Creater::findTargetQtexe(CompileTargetBase * _pBuildTarget)
{
	if (! _pBuildTarget)  return false ;

_printD("=> Begin 'Creater::findTargetQtexe(...)");

	wxString qtpath = pathlibQt(_pBuildTarget) ;
// Mes = Lf + _("QT path for the target = ") + quote(qtpath); _print(Mes);
	if(qtpath.IsEmpty() || qtpath == "\\" )
	{
		Mes = _("No library path QT 'in the target") + Lf + _("or nothing library !")  ;
		Mes += Lf +  _("Check that you have a correct path to the Qt path libraries !") ;
		Mes += Lf + _("Cannot continue.") ;
		_printError(Mes);
	//	cbMessageBox(Mes, "", wxICON_ERROR) ;
		return false ;
	}

	wxString qtexe = qtpath + "bin" + wxFILE_SEP_PATH  ;
//	Mes = "qtexe = " + quote(qtexe) ;  _printWarn(Mes);
	if (m_Win)
	{
		m_Mexe = qtexe + "moc.exe" ;
		m_Uexe = qtexe + "uic.exe" ;
		m_Rexe = qtexe + "rcc.exe" ;
		// compile *_fr.ts -> *_fr.qm
		m_Lexe = qtexe + "lrelease.exe" ;
	}
	else
	if (m_Linux)
	{	// check !
		m_Mexe = qtexe + "moc" ;
		m_Uexe = qtexe + "uic" ;
		m_Rexe = qtexe + "rcc" ;
		m_Lexe = qtexe + "lrelease" ;
	}
	else
	if (m_Mac)
	{ 	// ???
		m_Mexe = qtexe + "moc";
		m_Uexe = qtexe + "uic" ;
		m_Rexe = qtexe + "rcc" ;
		m_Lexe = qtexe + "lrelease" ;
	}
/// debug
//	Mes = "m_Mexe = " + quote(m_Mexe) ;  _printWarn(Mes);
//	Mes = "m_Uexe = " + quote(m_Uexe) ;  _printWarn(Mes);
//	Mes = "m_Rexe = " + quote(m_Rexe) ;  _printWarn(Mes);
//	Mes = "m_Lexe = " + quote(m_Lexe) ;  _printWarn(Mes);
/// <=
    bool Findqtexe = true, bdquote = false;
	bool FindMexe  = wxFileExists(m_Mexe) ;
	bool FindUexe  = wxFileExists(m_Uexe) ;
	bool FindRexe  = wxFileExists(m_Rexe);
	bool FindLexe  = wxFileExists(m_Lexe) ;
	if (!FindMexe)
    {
		Mes = _("Could not query the executable Qt") +  quote(m_Mexe)  ;
		Findqtexe = false;
		bdquote = m_Mexe.find(Dquote);// != -1 ;
    }
    else
    if (!FindUexe)
    {
        Mes = _("Could not query the executable Qt") +  quote(m_Uexe)  ;
        bdquote = m_Mexe.find(Dquote);
        Findqtexe = false;
    }
    else
    if (!FindRexe)
    {
        Mes = _("Could not query the executable Qt") +  quote(m_Rexe)  ;
        bdquote = m_Rexe.find(Dquote);
        Findqtexe = false;
    }
    else
    if (!FindLexe)
    {
        Mes = _("Could not query the executable Qt") +  quote(m_Lexe)  ;
        bdquote = m_Lexe.find(Dquote);
        Findqtexe = false;
    }
// error on path ?
    if (!Findqtexe )
    {
		Mes += Lf + _("Cannot continue") ;
		Mes += ", " ;
		Mes += _("Verify your installation Qt");
		if (bdquote)
		{
            Mes += " (" ;
            Mes += _("!! you have a quote in path !!");
            Mes += "" ;
		}
		Mes += "." ;
		_printError(Mes);

		wxString title = _("Search executable Qt") ;
		title += " ..." ;
		cbMessageBox(Mes, title, wxICON_ERROR) ;
	}
	m_IncPathQt = pathIncludeMoc(m_pProject) +  pathIncludeMoc(_pBuildTarget);
	m_DefinesQt = definesMoc(m_pProject) + definesMoc(_pBuildTarget);

	Mes.Clear();

_printD("	<=> End 'Creater::findTargetQtexe(...) => " + strBool(Findqtexe) );

	return Findqtexe ;
}
///-----------------------------------------------------------------------------
///	Read content file
///
/// Called by  :
/// 	1. Creater:: q_object(filename, qt_macro):1,
///		2. Creater::hasIncluded(const wxString& fcreator):1,
///
wxString Creater::ReadFileContents(const wxString& _filename)
{
	wxFileName fname(m_pMam->ReplaceMacros(_filename));
	NormalizePath(fname, wxEmptyString);
	wxFile f(fname.GetFullPath());

	return cbReadFileContents(f);
}

///-----------------------------------------------------------------------------
/// Looking for eligible files to the active target (around project files),
/// 	meet to table 'm_Filecreator' : return eligible file number
///
/// Called by  :
///		1. Creater::buildAllFiles(cbProject * prj, bool workspace, bool allbuild):1
///
/// Calls to :
///		1. Creater::virtualToFirstRealTarget(m_nameactivetarget):1,
///		2. Creater::isGoodTargetQt(m_nameActiveTarget):1,
///		3. Creater::isElegible(file):1,
///
wxUint16 Creater::findGoodfiles()
{
_printD("=> Begin 'Creater::findGoodfiles()'");
//	Mes = _("Search creator file(s) ...") ; _printWarn(Mes);
	/// DEBUG
//* ****************************************
//	beginDuration("findGoodFiles(...)");
//* ****************************************
	m_Filecreator.Clear();
// active target ( virtual or real )
	m_nameActiveTarget = m_pProject->GetActiveBuildTarget() ;
	if (m_nameActiveTarget.IsEmpty() )
		return 0;

	m_nameProject = m_pProject->GetTitle();
// => peek the first real target of the virtual
	virtualToFirstRealTarget(m_nameActiveTarget);
// is good target  ?
	ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget);
	// virtual target ?
	if (!pBuildTarget)	return 0 ;

	if (! isGoodTargetQt(pBuildTarget))		return 0 ;

// **********************
// Around project files
// **********************
	ProjectFile * prjfile ;
	wxArrayString tabtargets ;
	wxString file, nametarget;
	int nt, nfprj = m_pProject->GetFilesCount()  ;
//Mes = "nfprj = " + strInt(nfprj); _print(Mes);
	bool good ;
	/// DEBUG
//* ************************************************
//	beginDuration("findGoodFiles:: all files)");
//* ************************************************
// all files project
	for (int nf =0 ; nf < nfprj; nf++ )
	{
	// abort ?
		if (m_abort)
		{
			good = false;
			break;
		}
		prjfile = m_pProject->GetFile(nf);
		if (!prjfile) 	continue  ;
	//	file name
		file = prjfile->relativeFilename ;
//Mes = "file = " + file; _print(Mes);
		if (file.IsEmpty() )	continue  ;
		// no complement file
		if (file.StartsWith(m_dirPreBuild) )	continue  ;
//Mes = "finded file = " + file; _print(Mes);
	// all targets of file
		tabtargets = prjfile->GetBuildTargets() ;
		nt = tabtargets.GetCount()  ;
//Mes = "nt = " + strInt(nt); _print(Mes);
		if (nt > 0)
		{
		// all file targets
			for (int nft=0; nft < nt; nft++)
			{
				if (m_abort)
				{
					good = false;
					break;
				}
				nametarget = tabtargets.Item(nft) ;
//Mes = "nametarget = " + nametarget; _print(Mes);
				good = nametarget.Matches(m_nameActiveTarget) ;
				if (good )
					break ;
			}
		// target == active target
			if (good )
			{
//Mes = "nametarget = " + nametarget;	_printError(Mes);
//Mes = Tab + "file finded " + strInt(nf) + " = " + file;  _printWarn(Mes);
			// not eligible ?
				if (! isElegible(file))		continue  ;
//Mes = Tab + "file elegible " + strInt(nf) + " = " + file; _printWarn(Mes);
			// add the file and it target
				m_Filecreator.Add(file, 1) ;
//Mes = "elegible file = " + file; _print(Mes);
			}
		}
	// execute pending events
		m_pM->ProcessPendingEvents();
	}
	/// DEBUG
//* **********************************************
//	endDuration("findGoodFiles:: all files)");
//* **********************************************
	Mes.Clear();

	if (m_abort) return 0;

_printD("	<= End 'Creater::findGoodfiles()'");

//  number file eligible
	return m_Filecreator.GetCount()  ;
}

///-----------------------------------------------------------------------------
/// Reference of libQt by target
///		"" or  "5r" or "5d" or "0" (for 'qtmain')
///   	qt5 -> '5', release -> 'r' and debug -> 'd'
///		'qtmain' -> '0'
///
/// Called by  :
///		-# Creater::isGoodTargetQt(const ProjectBuildTarget * _pBuildtarget):1,
///
wxString Creater::refTargetQt(const ProjectBuildTarget * _pBuildTarget)
{
	wxString refqt ;
	if (! _pBuildTarget)
		return refqt  ;

//Mes = "Creater::refTargetQt(...)"; _printWarn(Mes);
	wxArrayString tablibs = _pBuildTarget->GetLinkLibs();
	int nlib = tablibs.GetCount() ;
//Mes = "nlib = " + strInt(nlib); _printWarn(Mes);
    if (!nlib)
    {
        tablibs = m_pProject->GetLinkLibs();
        nlib = tablibs.GetCount() ;
        if (!nlib)
        {
            Mes = _("This target has no linked library") ;
            Mes += " !!";
            _printError(Mes);
            return refqt ;
        }
    }
// search lib
	bool ok = false ;
	wxString str =  "" ;
	wxString namelib ;
	int u=0, index= -1, pos ;
	while (u < nlib && !ok )
	{
	// lower, no extension
		namelib = tablibs.Item(u++).Lower().BeforeFirst('.') ;
//Mes = strInt(u) + "- namelib = " + quote( namelib )); _printWarn(Mes);
	// no prefix "lib"
		pos = namelib.Find("lib");
		if (pos == 0)
			namelib.Remove(0, 3) ;
	// begin == "qt"
		pos = namelib.Find("qt") ;
		if (pos != 0)	continue ;
//Mes = strInt(pos) ; _printWarn(Mes);
	// compare
		index = m_TablibQt.Index(namelib);
//Mes = strInt(index) ; _printWarn(Mes);
		ok = index != -1 ;
	// first finded
		if (ok)
		{
		// == 'qtmain' or 'qtmaind'
			if ( namelib.StartsWith("qtmain" ) )
			{
				refqt = "0";
			}
			else
		// qt5xxxx or qt5xxxd
			if (namelib.GetChar(2) == '5')
			{
				refqt = "5" ;
			// the last
				str = namelib.Right(1) ;
				if ( str.Matches("d" ) )
					refqt += "d" ;
				else
					refqt += "r" ;
			}
			else
				refqt = "";

			break;
		}
	}
//Mes = " return refqt = " + quote( refqt )); _printWarn(Mes);
	Mes.Clear();

	return refqt ;
}
///-----------------------------------------------------------------------------
/// Give the good type of target for Qt
///
/// Called by  :
///		1. AddOnForQt::OnComplements(CodeBlocksEvent& event):1,
///		2. Creater::findGoodfiles():1,
///
/// Calls to:
///		1. Creater::refTargetQt(ProjectBuildTarget * _pBuildtarget):1,
///
bool Creater::isGoodTargetQt(const ProjectBuildTarget * _pBuildTarget)
{
	if (!_pBuildTarget)
		return false ;

	bool ok = ! isCommandTarget(_pBuildTarget);
	wxString str = refTargetQt(_pBuildTarget)  ;
//Mes = "str = " + quote( str ); _printWarn(Mes);
	bool isqt = ! str.IsEmpty()  ;
	if (!isqt)
	{
	// TODO ...
		Mes = Tab + _("This target have nothing library Qt") + " !" ;
		Mes += Lf + Tab + _("PreBuild cannot continue.") ;
		_printWarn(Mes);
	}
//Mes = "isGoodTargetQt  = " + strBool( ok && isqt); _printWarn(Mes);
	Mes.Clear();

	return ok && isqt ;
}

/// /////////////////////////// Search into files //////////////////////////////

///-----------------------------------------------------------------------------
/// Search elegible files
/// 'file' was created ?  ->  moc_*.cxx, ui_*.h, qrc_*.cpp
///
/// Called by  :
///		1.
///		2. Creater::buildOneFile(cbProject * prj, const wxString& fcreator):1,
///
/// Calls to :
/// 	1. Creater::q_object(const wxString& filename, const wxString& qt_macro):1;
///
bool Creater::isElegible(const wxString& _file)
{
_printD("=> Begin 'Creater::isElegible(" + _file + "'" );
	wxString ext = _file.AfterLast('.')  ;
//_print("ext = " + quote(ext) );
	bool ok = ext.Matches(m_UI) || ext.Matches(m_Qrc) ;
	if (ok)
		return ok;

	ok = false ;
	if (ext.Matches(EXT_H) || ext.Matches(EXT_HPP) || ext.Matches(EXT_CPP) )
		ok = q_object(_file, "Q_OBJECT" ) > 0  ;

_printD("	<= End 'Creater::isElegible(...) => " + strBool(ok) );

	return ok ;
}

///-----------------------------------------------------------------------------
/// Return occurrences of "Q_OBJECT" + occurrences of "Q_GADGET"
/// ==> TO DO : with 'RegExp' ...
///
/// Called by  :
///		1. Creater::isElegible(const wxString& file):1
///
int Creater::q_object(const wxString& _filename, const wxString& _qt_macro)
{
_printD("=> Begin 'Creater::q_object(" + _filename + ", " + _qt_macro + "'" );
/// DEBUG
//* **************************************************************************
//	beginDuration("q_object(" + filename + ", " + qt_macro + "");
//* **************************************************************************

///1- the comments
	wxString CPP = "//" ;
	wxString BCC = "/*", ECC = "*/" ;
	// ends of line
	wxString SA, SD;
	SA.Append(SepA); SD.Append(SepD) ;

///2- verify exist  : 'filename' is relative file name
	wxString namefile = m_dirProject + _filename ;
	if (! ::wxFileExists(namefile))
	{
		Mes = quote( namefile ) + _("NOT FOUND") + " !!!" + Lf ;
		_printError(Mes) ;
		return -1  ;
	}
///3- read source 'namefile'
	wxString source = ReadFileContents(namefile) ;
	if (source.IsEmpty())
	{
		Mes = quote( namefile ) + ": " + _("file is empty") +  " !!!" + Lf ;
		_printError(Mes) ;
		return -1 ;
	}
///----------------------------------------------------------------------
/// 1st test before the detailed analysis
	int number = 0  ;
	number = qt_find(source, "Q_OBJECT");
	number += qt_find(source, "Q_GADGET");
///-----------------------------------------------------------------------
	if (number)
	{
		int rest = source.length() ;
	// copy
		wxString temp = source.Mid(0, rest ), after ;
	// pos
		int posbcc, posecc, poscpp;
		bool ok ;
///4- delete C comments
		do
		{
			posecc = -1;
			rest = temp.length() ;
		// first DCC next FCC
			posbcc = temp.Find(BCC) ;
			if (posbcc != -1)
			{
				rest -= posbcc ;
				after = temp.Mid(posbcc, rest) ;
				posecc = after.Find(ECC) ;
				// 'posecc' relative to 'posbcc' !!
			}
		// comments exists ?
			ok = posbcc != -1 && posecc != -1  ;
			if (ok)
			{
		// delete full comment
				temp.Remove(posbcc, posecc + ECC.length()) ;
			}
		}
		while (ok)  ;
///5- delete C++ comments
		do
		{
			posecc = -1;
			rest = temp.length()  ;
		// start of a comment C
			poscpp = temp.Find(CPP) ;
			if (poscpp != -1)
			{
			// verify the caracter before : 'http://www.xxx' !!
				wxChar xcar = temp.GetChar(poscpp-1) ;
				if (xcar != ':' )
				{
					rest -= poscpp  ;
				// string comment over begin
					after = temp.Mid(poscpp, rest) ;
				// end comment
					posecc = after.Find(SA) ;
				}
				else
					poscpp = -1  ;
			}
			ok = poscpp != -1 && posecc != -1 ;
			if (ok)
			{
			// delete full comment
				temp.Remove(poscpp, posecc) ;
			}
		}
		while (ok) ;

///6- delete all text == "...."
		if (temp.Contains(Dquote) )
		{
			int posb, pose ;
			do
			{
				pose = -1;
				rest = temp.length() ;
			// first Dquote,
				posb = temp.Find(Dquote);
				if (posb != -1)
				{
					rest -= posb;
					after = temp.Mid(posb + 1, rest) ;
				// next Dquote
					pose = after.Find(Dquote);
				}
				// text exists ?
				ok = posb != -1 && pose != -1 ;
				if (ok)
				{
				// delete full text = "..."
					temp.Remove(posb, pose + 2) ;
				}

			} while (ok);
		}

///7- find "Q_OBJECT" and/or "Q_GADJECT"
		number = 0  ;
		wxString findtext = "Q_OBJECT" ;
		if ( _qt_macro.Matches(findtext) )
		{
			number = qt_find(temp, findtext)  ;
			findtext = "Q_GADGET" ;
			number += qt_find(temp, findtext)  ;
		}
		else
		{
			Mes = _("The file") + Space + _filename + Space;
			Mes += _("is not a good creator !!") ;
			_printError(Mes);
		}

	}
	/// DEBUG
//* *********************************
//	endDuration("q_object(...)");
//* *********************************
	Mes.Clear();

_printD("	<= End 'Creater::q_object(...) => " + strInt(number) );

	return number ;
}

///-----------------------------------------------------------------------------
/// Find valid "Q_OBJECT" or "Q_GADGET"  ( ex : QKeySequence )
///
/// Looks for occurrences of "Q_OBJECT" or "Q_GADGET" valid in the file.
///	Remarks:
///		Qt documentation indicates another METATOBJETS keyword -> "Q_GADGET"
///		like "Q_OBJECT" but when the class inherits from a subclass of QObject.
///	Algorithm:
///		1 - eliminates comments type C   '/*... ...*/'
///		2 - eliminates comments type Cpp  '// .... eol'
///		3 - one seeks "Q_OBJECT" then 'xQ_OBJECTy' to verify that 'x' in
///			[0xA, 0x9, 0x20] and 'y' in [0xd (or 0xA), 0x9, 0x20]
///		4 - one seeks "Q_GADGET" in the same way
///
///-----------------------------------------------------------------------------
/// Search 'qt_text' in 'tmp'
///
/// Called by  :
///		1. Creater::q_object(const wxString& filename, const wxString& qt_macro):4
///
int  Creater::qt_find (wxString _tmp, const wxString& _qt_text)
{
	uint8_t tab = 0x9, espace = 0x20  ;
	int len_text = _qt_text.length() ;
	int posq, number = 0;
	bool ok, good, goodb, gooda ;
	do
	{
		posq = _tmp.Find(_qt_text) ;
		ok = posq != -1 ;
		if (ok)
		{
		// pred and next caracter
			wxUChar xcar = _tmp.GetChar(posq-1) ;
			//uint8_t xcar = _tmp.GetChar(posq-1) ;
			wxUChar carx = _tmp.GetChar(posq + len_text)  ;
			//uint8_t carx = _tmp.GetChar(posq + len_text)  ;
		// only autorized caracters
			// before
			goodb = (xcar == espace || xcar == tab || xcar == SepA) ;
			// after
			gooda = (carx == espace || carx == tab || carx == SepD || carx == SepA);
			gooda = gooda || carx == '(' || carx == '_' ;
			good = goodb && gooda  ;
			if (good)
				number++  ;
		// delete analyzed
			_tmp.Remove(0, posq + len_text)  ;
		}
	}
	while (ok) ;

	return number ;
}

/// //////////////////////// End search into files /////////////////////////////

/// /////////////////////// Unregistered files /////////////////////////////////

///-----------------------------------------------------------------------------
///	To unregister a project file (creator and complements) in 'Pregen'
///		format 'm_Filecreator' ->  'dircreator\xxx.h'  ( src\xxx.h)
/// 	return 'true' when right
///
///	Called by :
///		1. AddOnForQt::OnProjectFileRemoved(CodeBlocksEvent& _event):1,
///
///	Calls to :
///		1. Creater::unregisterComplementFile(const wxString & _file):1,
///
bool Creater::unregisterCreatorFile(const wxString & _file)
{
// copy file, absolute project path
	wxString filename = _file, pathproject = m_pProject->GetCommonTopLevelPath();
// relative file path
	filename.Replace(pathproject, "" ) ;
//Mes = " ==> creator filename to find =" + quote( filename ); _print(Mes);
	int16_t index = m_Filecreator.Index (filename);
	bool ok = index != wxNOT_FOUND;
	if (!ok)
		return ok;
// replace creator by 'm_Devoid'
	m_Filecreator.RemoveAt(index); m_Filecreator.Insert(m_Devoid, index);
// _print(allStrTable("m_Filecreator", m_Filecreator));
// find complement in m_Registered
    filename = m_Registered.Item(index);
// remove complement if it's exists
	ok = filename.Matches(m_Devoid);
	if (!ok)
	{
		ok = unregisterComplementFile(filename);
	}

	Mes.Clear();

	return ok;
}

///-----------------------------------------------------------------------------
///	To unregister project file(s) (complements) in 'Pregen'
///		format 'm_Registered' ->  'dircomplement\\moc_xxx.cpp'  (m_dirPreBuild\\debug\\moc_xxx.cpp)
/// 	return 'true' when right
///
///	Called by :
///		1. AddOnForQt::OnProjectFileRemoved(CodeBlocksEvent& _event):1,
///		2. Creater::unregisterCreatorFile(const wxString & _file):n,
///
///	Calls to :
///		1. Pre::nameCreated(const wxString& _file):1,
/// 	2. Creater::removeComplementToDisk(const wxString & _filename, bool _withobject = false):1,
///

bool Creater::unregisterComplementFile(wxString & _file)
{
_printD("=> Begin 'Creater::unregisterComplementFile(" + _file + ")");

// copy file, absolute project path
	wxString filename = _file, pathproject = m_pProject->GetCommonTopLevelPath();
// relative file path
	filename.Replace(pathproject, "" ) ;
//Mes = " Creater::unregisterComplementFile() ==> filename to find = " + quote( filename ); _printWarn(Mes);
// we look for the number of 'complement' occurrences in 'm_Registered'
	bool ok = false;
	wxUint16 nf = m_Registered.GetCount();
//Mes =  " from " + quote( strInt(nf) )  + " file(s) ..." ; _print(Mes);
	wxString item;
	while (nf--)
	{
		item = m_Registered.Item(nf);
//Mes =  Tab + strInt(nf) +  " ==> " + quote(item); _print(Mes);
		if (!item.Contains(filename))	continue ;
	// replace table complement by 'm_Devoid'
		m_Registered.RemoveAt(nf); 		m_Registered.Insert(m_Devoid, nf);
		m_Filestocreate.RemoveAt(nf); 	m_Filestocreate.Insert(m_Devoid, nf);
		m_Createdfile.RemoveAt(nf);		m_Createdfile.Insert(m_Devoid, nf);
		m_Filewascreated.RemoveAt(nf);  m_Filewascreated.Insert(m_Devoid, nf);
	// remove from disk
//Mes =  Tab + Tab + strInt(nf) +  " ==> " + quote( item );
//Mes += " is here..."; _print(Mes);
		ok = removeComplementToDisk(filename);
		if (!ok)
		{
			Mes = " ==> ";
			Mes += _("error filename") ;
			Mes += + " =" + quote( filename );
		/// TO DO an other method ...
			ok = true;
		}
	}
	// no file  ?
	m_clean = m_Registered.IsEmpty();
// return short name
	_file = filename ;

	Mes.Clear();
_printD("	<= End Creater::unregisterComplementFile(...) => " + strBool(ok) );

	return ok;
}

///-----------------------------------------------------------------------------
///	To unregister all complements file to 'CB' after rename one target
/// return 'true' when right
///
/// Called by :
///		1. Creater::updateNewTargetName(ProjectBuildTarget * _pBuildTarget,
///							   				const wxString & _oldTargetName):1,
///
/// Call to :
///		1. Creater::removeComplementToDisk(const wxString & _filename,
///											  bool _withobject):1,
///
bool Creater::unregisterAllComplementsToCB(const wxString & _oldTargetName,
											  cbProject * _pProject)
{
Mes = "=> Begin 'Creater::unregisterAllComplementsToCB(" ;
Mes += _oldTargetName + ", ...)"; _printD(Mes);

	bool ok = false;
	wxString filename ;
	ProjectFile * prjfile ;
	int index;
// begin remove files
	_pProject->BeginRemoveFiles();
	bool withObject;
// all complement are registerd to 'm_Registered'
	for (wxString fregistered : m_Registered)
	{
//Mes = " fregistered = " + quote( fregistered  ) ; _print(Mes);
		if (fregistered.Matches(m_Devoid))	continue;

		prjfile = _pProject->GetFileByFilename(fregistered);
		if (prjfile)
		{
			filename = prjfile->relativeFilename ;
			if (filename.IsEmpty())		continue  ;
		// memorize compiled complement before remove from project...
			withObject	= prjfile->compile || prjfile->link;
		// unregister file from CB
			ok = _pProject->RemoveFile(prjfile);
			if (!ok)
			{
				Mes = " ==> RemoveFile() : " ;
				Mes += _("error with file") ;
				Mes += " =" + quote( filename );
				_printError(Mes);
				break;
			}
			else
			{
			// update  'm_Registered'
				index = m_Registered.Index(fregistered);
			// replace 'fregistered' by 'm_Devoid'
				m_Registered.RemoveAt(index); m_Registered.Insert(m_Devoid, index);

			// also remove file from disk
//Mes = " file = " + quote( file ) ; _print(Mes);
				ok = removeComplementToDisk(filename, withObject);
				if (!ok)
				{
					Mes = " ==> removeComplementToDisk() : ";
					Mes += _("error with file") ;
					Mes += " =" + quote( filename );
					_printError(Mes);
					break;
				}
			}
		}
	}

// end remove files
	_pProject->EndRemoveFiles();
// mandatory ! tree refresh
	 m_pMprj->GetUI().RebuildTree() ;

	Mes = Tab + "<== " + _("End complement file(s) was removed from C::B and disk");
	_printWarn(Mes);

	Mes.Clear();

_printD("	<= End 	Creater::unregisterAllComplementsToCB(...) => " + strBool(ok) );

	return ok;
}

///-----------------------------------------------------------------------------
/// To remove one complement files in project :
///		(1. 'm_Registered.item(pos)' is clear)
///		is it necessary ?  => yes in version >= 2.4.0
///		(2. disk : (m_dirPreBuild\\nametarget\\m_dirPreBuild) is removed)
///		return 'true' when right
///
/// Called by :
///		1. Creater::unregisterComplementFile(const wxString & _file, bool _creator, bool _first):1,
///		2. Creater::unregisterAllComplementsToCB(const wxString & _oldTargetName, cbProject * _project, bool _first):1,

bool Creater::removeComplementToDisk(const wxString & _filename, bool _withobject)
{
Mes = "=> Begin 'Creater::removeComplementToDisk(" ;
Mes += _filename + "," + strBool(_withobject) + ")";_printD(Mes);

// complement exists
	bool ok = ::wxFileExists(_filename);
// Mes = "ok = " + strBool(ok); _print(Mes);
	if (ok)
	{
	// remove complement file
		ok = ::wxRemoveFile(_filename);
		if (ok)
		{
			Mes = Tab +  "-" + quote( _filename );
		// if 'ui_xxx.h' => no compiled file '*.o'
			wxString beginfile = _filename.AfterLast('\\').BeforeFirst('_');
			if (beginfile.Matches("ui") )
			{
				_printWarn(Mes);
				return ok;
			}
		// extract the used target
			wxString  pathadding, targethere;
		// find target here
			int32_t pos = _filename.Find(m_dirPreBuild);
			if (pos >=0)
			{
				targethere  = _filename.Mid(pos).BeforeLast('\\').AfterLast('\\');
			}

/// it is also necessary to delete the compiled file from the old complement
/// ATTENTION : the active target may be different of target used !!

			wxString pathObjects, ofile = _filename.Before('.')  + ".o";
//Mes = " m_dirObjects  =" + quote( m_dirObjects ) ; _printWarn(Mes);
			pathObjects = m_dirObjects + ofile;
			m_pMam->ReplaceMacros(pathObjects);
			if (!targethere.IsEmpty())
				pathObjects.Replace(m_nameActiveTarget, targethere);
//Mes = " To delete => pathObjects  =" + quote( pathObjects ) ; _print(Mes);
			// is there a compiled complement file ?
				ok = ::wxFileExists(pathObjects);
				if (ok) {
					ok = ::wxRemoveFile(pathObjects);
					if (ok)
					{
						Mes += "," + quote( pathObjects );
						_print(Mes);
					}
				// error to removing :
					else
					{
						Mes = Tab + Tab + "-> " + _("not removed") + quote( pathObjects );
						_printError(Mes);
					}
				}
			// no compiled complement
				else
				{
				// the compiled complement file is missing
					if (_withobject)
					{
						Mes = Tab + Tab + "-> " + _("not exists") + quote( pathObjects );
						_printError(Mes);

					}
				// no complement has been compiled
					else
					{
					/// TO REVIEW to an other method ...
						_print(Mes);
					}
				}
		/// <=
		}
		else
		{
			Mes = Tab + _("Can not delete") + quote( _filename ) + " !!";
			_printError(Mes) ;
		}
	}
	else
	{
		Mes = Tab + _("File") + quote( _filename ) + _("not found") + " !!";
		_printError(Mes) ;
	}
	Mes.Clear();

_printD("	<= End 'Creater::removeComplementToDisk(...) =>" + strBool(ok) );

	return ok;
}
///-----------------------------------------------------------------------------
/// To remove one complement '*.o' file in project :
///		(1. disk : is removed)
///		return 'true' when right
///
/// Called by :
///		1. Creater::removeComplementToDisk(const wxString & _filename):1,

bool Creater::removeComplementObjToDisk(const wxString & _filename)
{
	bool ok = ::wxFileExists(_filename);
	/// TO FINISH ...


	return ok;
}

///-----------------------------------------------------------------------------
///	To remove all complement's directory after rename one target
/// return 'true' when right
///
/// Called by :
///		1. Creater::unregisterAllComplementsToCB(const wxString & _oldTargetName,
///													cbProject * _pProject, bool _first):1,
///
/// Call to :
///		1. bool Creater::recursRmDir(wxString _rmDir):2,
///
bool Creater::removeComplementDirToDisk(const wxString & _oldTargetName)
{
	bool ok ;
// delete old complements directory : '"m_dirPreBuild"\_oldTargetName\*.*'
	wxString file = m_dirPreBuild + _oldTargetName;
	Mes = Tab + _("Complement directory was removed from disk") + " ...";
	_printWarn(Mes);

	ok = recursRmDir(file);
	Mes = Tab + "- " + quoteNS( file + Slash + "*.*" ) ;
	if(ok)
	{
		Mes += Space + _("is deleted");
		_print(Mes);
	}
	else
	{
		Mes += " ==> " ;
		Mes +=  _(" cannot be deleted !!!");
		_printError(Mes);
	}
// delete old complements objects directory : 'obj\_oldTargetName\"m_dirPreBuild"\*.*'
	file = m_dirObjects + m_dirPreBuild ;
//Mes = "==> to delete objects" + Space + quote( file ) ; _printWarn(Mes);
	Mes = Tab + _("Object complement directory was removed from disk");
//	Mes +=  Space + quote( file );
	_printWarn(Mes);
	ok = recursRmDir(file);
	Mes = Tab + "-" + quote( file + "*.*" ) ;
	if (ok)
	{
		Mes += Space + _("is deleted");
		_print(Mes);
	}
	if (!ok)
	{
		Mes += " ==>" ;
		Mes += _(" cannot be deleted");
		Mes += " !!!";
		_printError(Mes);
	}
	Mes.Clear();

	return ok;
}

///-----------------------------------------------------------------------------
/// Remove old executable from old target
///
/// Called by :
///		1. Creater::updateNewTargetName(ProjectBuildTarget * _pBuildTarget,
///									 		const wxString & _oldTargetName):1,
///

bool Creater::removeOldExecutable(ProjectBuildTarget* _pBuildTarget, const wxString & _oldTargetName)
{
	bool ok = _pBuildTarget != nullptr;
	if (ok)
	{
		wxString oldexename = _pBuildTarget->GetOutputFilename();
		oldexename.Replace("$(TARGET_NAME)", _oldTargetName);
//Mes = " old outputname = " + quote( oldexename ); _printWarn(Mes);
		ok = ::wxFileExists(oldexename);
		if (ok)
		{
		// remove executable file
			ok = ::wxRemoveFile(oldexename);
			if (ok)
			Mes = Tab + _("Old executable") + quote( oldexename ) ;
			if(ok)
			{
				Mes += _("is deleted");
				_print(Mes);
			}
			else
			{
				Mes += " ==> " ;
				Mes += _("cannot be deleted") ;
				Mes += " !!!";
				_printError(Mes);
			}
		}
	}
	Mes.Clear();

	return ok;
}

///-----------------------------------------------------------------------------
/// Remove old include path qt for CB
///
/// Called by :
///		1. Creater::updateNewTargetName(ProjectBuildTarget * _pBuildTarget,
///									 		const wxString & _oldTargetName):1,
///

bool Creater::removeOldPathtoCB(ProjectBuildTarget * _pContainer, const wxString & _oldTargetName)
{
//Mes = " *** removeOldPathtoCB(...) from" + quote( _oldTargetName ) ; _printD(Mes);
	bool ok = false;
	// read target include path
	wxArrayString tabincludedirs = _pContainer->GetIncludeDirs(), tabpath;
//_print(allStrTable( " include dir", tabincludedirs));
	if (tabincludedirs.GetCount())
	{
		for (wxString& line : tabincludedirs)
		{
			m_pMam->ReplaceMacros(line) ;
//Mes = "** line =" + quote(line); _print(Mes);
		// it's an old path
			if (line.Contains(_oldTargetName))	continue;
		// memorize the line
			tabpath.Add(line, 1);
		}
//_print(allStrTable( " include dir", tabpath));
	// rewrite the paths without the old ones
		_pContainer->SetIncludeDirs(tabpath);
	}

	return ok;
}
///-----------------------------------------------------------------------------
/// Recursively deletes all directories and files
///
/// Called by :
///		1. bool Creater::recursRmDir(wxString _rmDir):1,
///		2. bool Creater::removeComplementDirToDisk(const wxString & _oldTargetName):2,
///
bool Creater::recursRmDir(wxString _rmDir)
{
// For safety, you don't empty a complete disk ('..' or '.')
    if (_rmDir.length() <= 3)
		return false;

// We make sure that the directory to be deleted exists
    if(!wxDir::Exists(_rmDir))
		return false;

// We make sure that the name of the folder to be deleted ends with a separator
	// if (_rmDir.Last() != wxFILE_SEP_PATH )
	/// patch find in "https://github.com/vslavik/poedit"
	if (_rmDir.empty() || (_rmDir.Last() != wxFILE_SEP_PATH) )
		_rmDir += wxFILE_SEP_PATH;
//Mes = quote( _rmDir ); _printWarn(Mes);

// Creating the 'wxDir*' object
    wxDir* pDir = new wxDir(_rmDir);
    if (!pDir)
		return false;

/// Retrieving the first item in the folder to be deleted
    wxString item;
    bool ok = pDir->GetFirst(&item);
    if (ok)
    {
        do
        {
		// If the item found is a directory
            if (wxDirExists(_rmDir + item))
			/// !!! recursive call !!!
                recursRmDir(_rmDir + item);
            else
		// Otherwise, we delete the file
			if(!wxRemoveFile(_rmDir + item))
			{
			//Error during deletion (access rights ?)
				Mes =_( "Could not remove item") + quote(_rmDir + item);
				_printError(Mes);
			}
        }
        while (pDir->GetNext(&item));
    }
/// From now on, the folder is empty, so we can delete it
    // But first you have to destroy the wxDir variable that always points to it
    delete pDir;
    // You can now delete the folder passed as a parameter
    ok = wxFileName::Rmdir(_rmDir);
    if (!ok)
    {
	//Error during deletion (access rights ?)
		Mes = _("Could not remove directory") + quote(_rmDir);
		_printError(Mes);
    }
    Mes.Clear();

    return ok;
}

///-----------------------------------------------------------------------------
/// Total updates of the new target from old target
/// TO FINISH ...
///
/// Called by :
///		1. AddOnForQt::OnRenameProjectOrTarget(CodeBlocksEvent& _event):1,
///
/// Calls to :
///		1. Creater::unregisterAllComplementsToCB(const wxString & _oldTargetName,
///											  		cbProject * _project, bool _first):1,
///		2. Creater::removeComplementDirToDisk(const wxString & _oldTargetName):1,
///		3. Creater::removeOldPathtoCB(ProjectBuildTarget * _pContainer, const wxString & _oldTargetName):1,
///		4. Creater::removeOldExecutable(ProjectBuildTarget* _pBuildTarget, const wxString & _oldTargetName):1,

bool Creater::updateNewTargetName(ProjectBuildTarget * _pBuildTarget,
									 const wxString & _oldTargetName)
{
	bool ok = true;

//1- unregister complements in 'm_dirPreBuild\\oldNameTarget\\*.*'
//2- remove complements from disk
	//ok = unregisterAllComplementsToCB(_oldTargetName, m_pProject, FIRST);
	ok = unregisterAllComplementsToCB(_oldTargetName, m_pProject);

//3- remove old directory to disk
	ok = removeComplementDirToDisk(_oldTargetName);

//4- remove old paths to new build target
	ok = removeOldPathtoCB(_pBuildTarget, _oldTargetName);

//5- remove 'BuildCB\\oldNameTarget\\app.exe'
	ok = removeOldExecutable(_pBuildTarget, _oldTargetName);

	return ok;
}


/// //////////////////////// End Unregisterd files /////////////////////////////

///-----------------------------------------------------------------------------
/// To register ALL new files in project : 'm_Registered' is meet
///	return 'true' when right
///
/// Called by  :
///		1. Creater::buildAllFiles(cbProject * prj, bool workspace, bool allbuild):1,
///
/// Calls to :
///		1. Creater::addOneFile(const wxString& fcreator, const wxString& fout):1,
///
wxUint16 Creater::addAllFiles()
{
_printD(="=> Begin Creater::addAllFiles()");

	m_Registered.Clear();

	bool valid = false;
	wxString fout;
// read file list to 'm_Filecreator'  (*.h, *.cpp, *.qrc, *.ui)
    for (wxString fcreator : m_Filecreator)
	{
	// complement file
		fout = complementDirectory() + nameCreated(fcreator) ;
	// record one complement file
		valid = addOneFile(fcreator, fout) ;
	// ok ?
		if (valid)
		{	// registered in table
			m_Registered.Add(fout, 1) ;
		}
	}
// tree refresh
	// >= svn 9501 : CB 13.12
	if (m_pMprj)
		m_pMprj->GetUI().RebuildTree() ;
//  end registering
	wxUint16 nf = m_Registered.GetCount() ;
//	Mes = Tab + "-> " + strInt(nf) ;
//	Mes += Space + _("creator(s) file(s) registered in the plugin") ; _print(Mes) ;

_printD("	<== End Creater::addAllFiles()");
	return  nf;
}
///-----------------------------------------------------------------------------
/// To register One file in project and 'm_Registered' is actualized
///	return 'true' when right
///
/// Called by  :
///		1. Creater::addAllFiles():1,
///
/// Calls to :
///		1. Pre::nameCreated(const wxString& file):1,
///		2. Creater::hasIncluded(const wxString& fcreator):1,
///
bool Creater::addOneFile(const wxString& _fcreator, const wxString& _fout)
{
// local variables
	bool valid = false, nocompile = false, weight = 50;
// has included ?
	wxString extin = _fcreator.AfterLast('.') , extout = _fout.AfterLast('.')  ;
	if ( (extin.Matches(EXT_H) && extout.Matches(EXT_CPP)) ||
		 (extin.Matches(EXT_CPP) && extout.Matches(EXT_MOC)) )
		nocompile =  hasIncluded(_fcreator) ;
	else
// ui_*.h file
	if (extin.Matches(EXT_UI) && extout.Matches(EXT_H) )  {
		nocompile = true;
		weight = 40; // before all files
	}
//Mes = Tab + quote(_fcreator) + "->" +  quote( _fout ) ;
//Mes += " : include =" +  quote( strBool(nocompile) ) ; _printWarn(Mes);

// find projectfile in project ?
	ProjectFile * prjfile = m_pProject->GetFileByFilename(_fout);
	valid = prjfile != nullptr;
	if (valid)
	{
	// and add target to this projectfile
		prjfile->AddBuildTarget(m_nameActiveTarget);
		valid = prjfile != nullptr;
		if (valid)
		{
		// included ?
			prjfile->compile = !nocompile;
			prjfile->link = !nocompile;
			prjfile->weight = weight;
		}
	}
	else {
	// then create a projectfile with the file to target
		prjfile = m_pProject->AddFile(m_nameActiveTarget, _fout, !nocompile, !nocompile, weight);
		valid = prjfile != nullptr;
		if (!valid)
		{
		// display message
			Mes  = "===> " ;
			Mes += _("can not add this file");
			Mes += quote( _fout ) + _("to target") + Space + m_nameActiveTarget ;
			_printError (Mes) ;
			cbMessageBox(Mes, "AddFile(...)", wxICON_ERROR) ;
		}
	}
	Mes.Clear();

	return  valid;
}

///-----------------------------------------------------------------------------
///	Search included file
///  *.cpp contains  (# include "moc_*.cpp" or (# include "*.moc" at the file end
///
/// Called by  :
///		1. Creater::buildOneFile(cbProject * prj, const wxString& fcreator):1
///		2. Creater::addOneFile(const wxString& fcreator, const wxString& fout):1,
///
/// Calls to :
///		1. Creater::verifyIncluded(wxString& source, wxString& txt)
///
bool Creater::hasIncluded(const wxString& _fcreator)
{
_printD("==> Begin 'Creater::hasIncluded(" + _fcreator + ")'" );
// if '_fcreator' == 'xxxx.h' -> search in 'xxxx.cpp'
// if '_fcreator' == 'xxxx.cpp'  -> search in 'xxxx.cpp'
	wxString namefile = _fcreator.BeforeLast('.') + DOT_EXT_CPP;
	if (! ::wxFileExists(namefile) )
		 return false  ;

// load file  'xxxx.cpp'
	wxString source = ReadFileContents(namefile) ;
	if (source.IsEmpty())
		return false ;

// ext
	wxString ext = _fcreator.AfterLast('.');
//Mes = Tab + Tab + "ext = " + quote( ext ) ; _print(Mes);
	namefile = namefile.AfterLast(Slash).BeforeLast('.')  ;
	wxString txt = "";
	bool include = false;
//1- ext == EXT_H =>  search >"moc_namefile.cpp"<
	if (ext.Matches(EXT_H))
	{
		txt = "moc_" + namefile + DOT_EXT_CPP;
	}
	else
//2- ext == EXT_CPP => search >"namefile.moc"<
	if (ext.Matches(EXT_CPP) )
	{
		txt = namefile + DOT_EXT_MOC ;
	}
//Mes = Tab + Tab + "txt = " + Dquote + txt + Dquote ; _print(Mes);
// find from end
	include = verifyIncluded(source, txt) ;

_printD("	<== End 'Creater::hasIncluded(...)' => " + strBool(include));
	return  include ;
}

/// Called by  :
///		1. hasIncluded(const wxString& fcreator):2
///
bool Creater::verifyIncluded(wxString& _source, wxString& _txt)
{
	wxString temp;
	int32_t pos = _source.Find(_txt);
	bool include =  pos != -1 ;
	if (include)
	{
	// delete '_txt' from pos
		_source.Remove(pos);
	// search from the end, the last character of endline
		pos = _source.Find('\n', true);
		if (pos >=0)
   		{
   		// give from pos ( the last line )
			temp = _source.Mid(pos+1);
		// search "//" or "/*"
			pos = temp.Find("//");
			if (pos >= 0 )
			{
				include = false;
			}
			else
			{
				pos = temp.Find("/*");
				include = pos == -1;
			}
		//	Mes = "line = " + quote( temp ); 	_print(Mes);
		}
		else
		{
			// ??
		}
	}

	return include;
}

///-----------------------------------------------------------------------------
/// Indicates whether a file is registered in the project
///
/// Called by  :
///		1. Creater::buildOneFile(cbProject * prj, const wxString& fcreator):1,
///		2. Creater::filesTocreate(bool allrebuild):1,
///
bool Creater::inProjectFileCB(const wxString& _file)
{
//Mes = Tab + quote( file ) ;
    // relative filename !!!
	ProjectFile * prjfile = m_pProject->GetFileByFilename (_file, IS_RELATIVE, IS_UNIX_FILENAME) ;
//Mes += " -> " + here; _printWarn(Mes);
	return prjfile != nullptr;
}

///-----------------------------------------------------------------------------
///  Create directory for "qtbrebuilt\"
///
///  Called by :
///		1. Creater::beginMesBuildCreate():1,
///		2. Creater::buildOneFile(cbProject * prj, const wxString& fcreator):1,
///		1. Creater::createComplement(const wxString& qexe, const wxUint16 index):1,
///
bool Creater::createDir (const wxString&  _dircomplement)
{
//Mes = "Creater::createDir(") + quote( dircomplement ) + "" ;
//_printWarn(Mes);
	bool ok = true  ;
	if (! wxDirExists(_dircomplement))
	{
	// Linux -> access right = 0x0777
		ok = wxFileName::Mkdir(_dircomplement,  wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
		if (!ok)
		{
			Mes = _("Can't create directory")  ;
			Mes += quote( _dircomplement ) + "!!" ;
			_printError(Mes);
			cbMessageBox(Mes, "createDir()", wxICON_ERROR) ;
		}
	}
	Mes.Clear();

	return ok ;
}
///-----------------------------------------------------------------------------
///  Remove directory for '"m_dirPreBuild"\name-target\'
///
///  Called by : none
///
bool Creater::removeDir(const wxString& _dirgen )
{
	bool ok = ::wxDirExists(_dirgen) ;
	if (ok)
	{
//Mes = quote( dirgen ) + "!!" ; _printWarn(Mes);
		ok = ::wxRmdir(_dirgen)  ;
		if (!ok)
		{
			Mes = _("Can't remove directory") ;
			Mes += quote( _dirgen) + "!!" ;
			_printError(Mes);
			cbMessageBox(Mes, "", wxICON_ERROR) ;
		}
	}
	Mes.Clear();

	return ok ;
}
///-----------------------------------------------------------------------------
/// Must complete the table 'm_Filestocreate'
///
/// Called by
///		1. Creater::buildAllFiles(cbProject * prj, bool workspace, bool allbuild)
///
///	Calls to
///		1. Pre::copyArray(const wxArrayString& strarray):1,
///		2. Creater::inProjectFileCB(const wxString& file):1,
///		3. Creater::compareDate(const wxString&  fileref, const wxString&  filetarget):1,
///
wxUint16 Creater::filesTocreate(bool _allrebuild)
{
	m_Filestocreate.Clear();
// save all open files
	bool ok = m_pM->GetEditorManager()->SaveAll();
	if (!ok )
	{
		Mes = _("Couldn't save all files ") ;
		Mes += " !!"  ;
		_printError (Mes);
		cbMessageBox(Mes, "", wxICON_ERROR)  ;
		return ok  ;
	}

    wxUint16  ntocreate = 0;
// all rebuild : copy table
	if (_allrebuild)
	{
		m_Filestocreate = copyArray(m_Registered) ;
		ntocreate = m_Filestocreate.GetCount();
	}
// build or rebuild not identical
	else
	{
	// local variables
		wxString inputfile, nameout ;
		bool inproject ;
	// analyze all eligible files/targets
		wxUint16 nfiles = m_Filecreator.GetCount() ;
		for (wxUint16 i=0; i < nfiles ; i++ )
		{
			if (m_abort)
			{
				ntocreate = 0;
				break;
			}
		// relative name inputfile
			inputfile = m_Filecreator.Item(i) ;
		// file registered to create
			nameout = m_Registered.Item(i) ;
		// already registered
			inproject =  inProjectFileCB(nameout) ;
			m_Identical = false ;
		//- verify datation on disk
			if (inproject)
			{
			// verify if exists file
				if (::wxFileExists(nameout))
				{
				// test his modification date
					m_Identical = compareDate(inputfile, nameout) ;
				}
			}
		// identical date -> no create
			if (m_Identical)
			// add 'm_Devoid' to 'Filetocreate' and continue
				m_Filestocreate.Add(m_Devoid, 1) ;
			else
			{	// to create
				m_Filestocreate.Add(nameout, 1)  ;
				ntocreate++;
			}
		} // end for
	}
	Mes.Clear();

	return ntocreate;
}
///-----------------------------------------------------------------------------
/// Compare file date
///
/// Called by  :
///		1. Creater::buildOneFile(cbProject * prj, const wxString& fcreator):1,
///		1. Creater::filesTocreate(bool allrebuild):1,
///
bool Creater::compareDate(const wxString& _fileref, const wxString&  _filetarget)
{
	if ( !(::wxFileExists(_fileref) && ::wxFileExists(_filetarget) ) )
		return false;

	wxFileName refname(_fileref);
	wxDateTime refdate = refname.GetModificationTime();

	wxFileName target(_filetarget);
	wxDateTime targetdate = target.GetModificationTime();

	bool ok = refdate.IsEqualTo(targetdate);

	return ok ;
}
///-----------------------------------------------------------------------------
/// Set date to target
///
/// Called by  :
///		1. Creater::createFileComplement(const wxString& qexe, const wxString& fcreator
///											, const wxString& fout):1,
///
bool Creater::modifyDate(const wxString&  _fileref, const wxString& _filetarget)
{
	if ( !(::wxFileExists(_fileref) && ::wxFileExists(_filetarget) ) )
		return false;

	wxFileName refname(_fileref);
	wxDateTime refdate = refname.GetModificationTime();

	wxFileName target(_filetarget);
	//wxDateTime targetdate = target.GetModificationTime();

	bool ok  = target.SetTimes(0, &refdate, 0);

	return ok ;
}
///-----------------------------------------------------------------------------
/// Create files before build project :
///		- additions built files before generation
/// Algorithm:
///		(-# 'm_Filecreator' is meet by 'findGoodfiles()')
///		(-# 'm_Registered is filled by 'addAllFiles()')
///		(-# 'm_Filestocreate' filled by 'filesTocreate(bool allrebuild)')
///		(-# meet 'm_Createdfile')
///
/// return true if all right
///
/// Called by  :
///		1. Creater::buildAllFiles(cbProject * prj, bool workspace, bool allbuild):1,
///
///	Calls to :
///		1. Creater::isNoFile(const wxArrayString& arraystr):1,
///		2. Creater::findTargetQtexe(buildtarget->GetParentProject()):1,
///		3. Creater::createComplement(const wxString& qexe, const wxUint16 index):3,
///
bool Creater::createFiles()
{
// search target qt path for moc, ...
	ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget)  ;
	if (!pBuildTarget)	return false ;

	bool  ok = findTargetQtexe(pBuildTarget) ;
	if (!ok)	return ok ;

// -> debug
//Mes = allStrTable("m_Filestocreate", m_Filestocreate); _print(Mes);

// nothing to do ?
	// 'nfilesTocreate' count good files ( no 'm_Devoid' )
	uint32_t nftocreate = nfilesToCreate(m_Filestocreate)
			, nfcreated = m_Createdfile.GetCount()
			, nftodisk = m_Filewascreated.GetCount();

//Mes = "nftocreate = "  + strInt(nftocreate) ;
//Mes += ", nfcreated = "  + strInt(nfcreated) ;
//Mes += ", nftodisk = "  + strInt(nftodisk) ; _printWarn(Mes);

// no 'abort' during last build
	if (nftocreate == nfcreated )
	{
		m_Createdfile.Clear();
		nfcreated = 0;
	//	m_nfilesCreated = 0;
	}
    // nothing to do ?
	bool emptiness = isNoFile(m_Filestocreate) ;
	if (emptiness)
	{
	// already created
		Mes = Tab + _("Nothing to do") + "( " +  strInt(nftodisk);
		Mes += Space + _("correct complement file(s) exist on disk")  + " )" ;
		_print(Mes) ;

		return emptiness  ;
	}

	Mes = Tab + "=> " + _("There are");
	Mes += Space + strInt(nftocreate) + Space ;
	Mes += _("complement file(s) to rebuild") ;
	Mes += "...";
	_printWarn(Mes);

// used by 'createComplement()'
//	if (m_clean)
//		m_nfilesCreated = nfcreated;

	m_nfilesCreated = 0;
// local variables
	wxString nameout, ext, strerror ;
	bool created = false, abandonment = false ;
	wxUint16 nfiles = m_Filestocreate.GetCount() ;
	int32_t ncreated = 0 ;
// analyze all eligible files/target
	for (wxUint16 i = 0; i < nfiles ; i++ )
	{
		if (m_abort)
		{
		// if 'i==0' -> i-1 < 0 !!
			created = false;
			ncreated = i-1;
			break;
		}
		nameout = m_Filestocreate.Item(i) ;
		if (nameout.Matches(m_Devoid) )		continue  ;

	// extension creator
		ext = m_Filecreator.Item(i).AfterLast('.') ;
	// create file
	//1- file *.ui : launch 'uic.exe'
		if ( ext.Matches(m_UI) )
			strerror = createComplement(m_Uexe, i) ;
		else
	//2- file *.qrc : launch 'rcc.exe'
		if (ext.Matches(m_Qrc) )
			strerror = createComplement(m_Rexe, i)  ;
		else
	//3- file with 'Q_OBJECT' and/or 'Q_GADGET' : launch 'moc.exe'
		if (ext.Matches(EXT_H) || ext.Matches(EXT_HPP) || ext.Matches(EXT_CPP) )
			strerror = createComplement(m_Mexe, i)  ;
	// more one
		created = strerror.IsEmpty();
		if (created)
		{
			m_Createdfile.Add(nameout, 1)  ;
		}
		else
		{
		// 'validCreated()' detect 'm_Devoid'  !!
			m_Createdfile.Add(m_Devoid, 1)  ;
		// error message
			wxString title = _("Creating") + quote( m_Registered.Item(i)) ;
			title += _("failed") ;
			title += " ...";
		// error create complement
			Mes =  Tab +"=> " ;
			Mes += strerror.BeforeLast(Lf.GetChar(0)) ;
			_printError (Mes) ;
        // message box
            Mes = strerror.BeforeLast(Lf.GetChar(0)) ;
			Mes += Lf + _("Do you want to stop pre-construction only ?");
			int16_t choice = cbMessageBox(Mes, title, wxICON_QUESTION | wxYES_NO) ;
			abandonment = choice == wxID_YES ;
			if (abandonment)
			{
                Mes = Tab  + _("Abandonment of the pre-construction") + " ...";
				_printError (Mes) ;
				break;
			}
			else
			{
				Mes = Tab  + _("Continue building") + " ...";;
				_printWarn (Mes) ;
				created = true;
			}
		}
	}
// Abandonment of the construction of supplements
	if (abandonment)
	{
	// the last files is 'm_devoid'
    // ...
	}
// Abortion of the construction of supplements
	if (m_abort)
	{
	// 'ncreated' files were created normally
		Mes = Tab + "=> " + _("You have created normally") + Space + strInt(ncreated) + "/" ;
		Mes += strInt(nfiles) + Space + _("file(s)");
		_print(Mes);
    // adversing
		Mes = _("The 'abort' button was pressed during the creation of the complement files(s)") + " !!!";
	//	_printWarn(Mes) ;
		cbMessageBox(Mes, "Create File(...)", wxICON_WARNING ) ;
	// adjust 'm_Createdfile'
		wxArrayString  temp ;
		temp = copyArray(m_Createdfile, ncreated);
			m_Createdfile.Clear();
			m_Createdfile = copyArray(temp);
	// adjust 'm_Registered'
		temp = copyArray(m_Registered, ncreated);
			m_Registered.Clear();
			m_Registered= copyArray(temp);
	// remove old file on disk
    // ...
/// Debug
//wxUint16 nf = m_Createdfile.GetCount() ;
//Mes =  Tab + "- " + strInt(nf) + Space  ;
//Mes += _("complement(s) file(s) are created in the target") ; _printWarn(Mes)  ;
/// <=
	}
	Mes.Clear();

	return created ;
}
///-----------------------------------------------------------------------------
/// Search all 'm_Devoid' to an wxArrayString
///
/// Called by  :
///		1. Creater::createFiles():1,
///
bool Creater::isNoFile (const wxArrayString& _arraystr)
{
	bool devoid = false ;
	for (const auto item : _arraystr)
	{
		devoid = item.Matches(m_Devoid) ;
		if (!devoid)
			break  ;
	}

	return devoid ;
}
///-----------------------------------------------------------------------------
/// Search all files to create into an wxArrayString
///
/// Called by  :
///		1. Creater::createFiles():1,
///
wxUint16 Creater::nfilesToCreate(const wxArrayString& _arraystr)
{
	wxUint16 nftocreate = 0;
	for (const auto item : _arraystr)
	{
		if(! item.Matches(m_Devoid) )
			nftocreate++;
	}

	return nftocreate;
}
///-----------------------------------------------------------------------------
///	Execute commands 'moc', 'uic', 'rcc' : return _("" if file created
///	else return an error string
///
/// Called by  :
///		1. Creater::buildOneFile(cbProject * prj, const wxString& fcreator):1,
///		2. Creater::createComplement(const wxString& qexe, const wxUint16 index):1,
///
///	Calls to :
///		1. Creater::executeAndGetOutputAndError():1,
///		2. Creater::modifyDate(const wxString&  fcreator, const wxString& fout):1,
///
wxString Creater::createFileComplement(const wxString& _qexe,
										  const wxString& _fcreator,
										  const wxString& _fout)
{
// build command
	wxString command = _qexe  ;

//Mes = "Creater::createFileComplement() => command = *>" + command  + "<*";
//_printWarn(Mes);

// add file name whithout extension
	if (_qexe.Matches(m_Rexe))
	{
		wxString name = _fcreator.BeforeLast('.')  ;
		command +=  " -name " + name  ;
	}
	if (_qexe.Matches(m_Mexe))
	{
		command +=  m_DefinesQt + m_IncPathQt ;
	}
// add input file
	command += Space + _fcreator ;
// add output file
	command += " -o " + _fout;
//Mes = Tab + "=> command = *>" + command  + "<*"; _printWarn(Mes);

// execute command line : use short file name
	wxString strerror = executeAndGetOutputAndError(command, PREPEND_ERROR)  ;
	bool created =  strerror.IsEmpty() ;
//_print("createFileComplement::strerror = " + quote(strerror) + " => created : " + strBool(created));

// file-project              : params = name, is relative, is Unixfilename
//_print("fout = " + quote(_fout));

	ProjectFile * prjfile =  m_pProject->GetFileByFilename(_fout, IS_RELATIVE, IS_UNIX_FILENAME) ;
	if (!prjfile)
	{
		Mes = _fout + _(" is no correct !!");
		return  Mes;
	}
// request abort ?
	if (m_abort)
	{
		created = false;
	}
// create error
	if (! created )
	{
	// unregister the last file 'fout' in the project
		m_pProject->RemoveFile(prjfile) ;
	//  >= svn 9501 : CB 13.12
		 m_pMprj->GetUI().RebuildTree() ;
	}
	else
	{
	// modify date of created file 'fout'
		m_Identical = modifyDate(_fcreator ,_fout) ;
    // more one file
		++m_nfilesCreated ;
	// display
		bool nocompile = ! prjfile->compile
			,nolink = ! prjfile->link;

		Mes = Tab + strInt(m_nfilesCreated) + "-" + quote(_fcreator)  ;
		Mes +=  "--->" + quote(_fout) ;
		if (nocompile && nolink)
		{
			Mes += Tab + "(" + _("no compiled, no linked") + ")";
			_printWarn(Mes);
		}
		else
			_print(Mes);
	}
	Mes.Clear();

//_print("createFileComplement::strerror = " + quote(strerror));
// return  error  : good if strerror is empty

	return strerror ;
}
///-----------------------------------------------------------------------------
///	Execute commands 'moc', 'uic', 'rcc' : return _("" if file created
///	else return an error string
///
/// Called by  :
///		1. Creater::createFiles():3,
///	Calls to :
///		1. Creater::createFileComplement(const wxString& qexe, cnst wxString& fcreator
///											, const wxString& fout):1,
///
wxString Creater::createComplement(const wxString& _qexe, const wxUint16 _index)
{
_printD("=> Begin 'Creater::createComplement(...)'");

//1- name relative input file  "src\filecreator.xxx"
	wxString inputfile = m_Filecreator.Item(_index) ;
	// create directory for m_nameActiveTarget
	wxString dircomplement = m_Registered.Item(_index).BeforeLast(Slash)  ;
	dircomplement +=  wxFILE_SEP_PATH ;

	if (!createDir(dircomplement))
	{
		Mes = _("Unable to create directory") + quote( dircomplement) ;
		return Mes ;
	}
//2- add search path for compiler
	ProjectBuildTarget * pBuildTarget = m_pProject->GetBuildTarget(m_nameActiveTarget) ;
	if (!pBuildTarget)
	{
		Mes = quote(m_nameActiveTarget) + _("does not exist") + " !!" ;
		return Mes ;
	}

	// add include directory
//	bool incdir = pBuildTarget->GetIncludeInTargetAll(); // ??
	pBuildTarget->AddIncludeDir(dircomplement) ;
//3- full path name  output file
	wxString outputfile = m_Registered.Item(_index) ;
//4- create on file complement
	wxString strerror = createFileComplement(_qexe, inputfile, outputfile) ;


_printD("	<= End 'Creater::createComplement(...)'");
//_print("createComplement::strerror = " + quote(strerror));
//5- return  error  : good if 'strerror' is empty

	return strerror ;
}
///-----------------------------------------------------------------------------
/// Give include path qt for 'm_Moc'
///
/// Called by :
///		1. Creater::findTargetQtexe(CompileTargetBase * buildtarget):1,
///
wxString Creater::pathIncludeMoc(const CompileTargetBase * _pContainer)
{
	wxArrayString tabincludedirs = _pContainer->GetIncludeDirs(), tabpath;
	wxString incpath;
	for (wxString& line : tabincludedirs)
	{
		m_pMam->ReplaceMacros(line) ;
		line.Prepend("-I");
		tabpath.Add(line, 1);
	}
// build 'incpath'
	if (tabpath.GetCount())
		incpath = Space + GetStringFromArray(tabpath, Space, SEPARATOR_AT_END) + Space;

	return incpath  ;
}

///-----------------------------------------------------------------------------
/// Give 'defines' qt for 'm_Moc'
///
/// Called by :
///		1. Creater::findTargetQtexe (CompileTargetBase * buildtarget):1,
///
wxString Creater::definesMoc(CompileTargetBase * _pContainer)
{
	wxArrayString tabdefines = _pContainer->GetCompilerOptions(), tabdef ;
	wxString def ;
	for (const wxString line : tabdefines)
	{
		if (line.Find("-D") != -1 )
			tabdef.Add(line, 1) ;
	}
// build 'def'
	if (tabdef.GetCount())
		def = " " + GetStringFromArray(tabdef, " ", true) + " " ;

	return def ;
}
///-----------------------------------------------------------------------------
/// Verify if at least one valid file is saved in 'm_Createdfile'
///
/// Called by  :
/// 	1. Creater::buildAllFiles(cbProject * prj, bool workspace, bool allbuild):1,
///
bool Creater::validCreated()
{
	bool tocreate = false, ok = true;
	wxUint16 ncase = m_Createdfile.GetCount()  ;
	while (ncase > 0 && !tocreate)
	{
		tocreate = ! m_Createdfile.Item(--ncase).Matches(m_Devoid)  ;
//Mes =  "validCreated() -> " + strInt(ncase) + " = " +  strBool(tocreate) ;
//_printWarn(Mes);
	}
	if (tocreate)
	{
	// svn >= svn 9501 : CB 13.12
		 m_pMprj->GetUI().RebuildTree()  ;
	// save project
		ok =  m_pMprj->SaveProject(m_pProject)  ;
		if(!ok)
		{
			Mes = _("Save project is not possible !!");
			_printError(Mes);
		}
	}
	Mes.Clear();

	return ok ;
}
///-----------------------------------------------------------------------------
/// Execute another program (always synchronous).
///
/// Called by :
///		1. Creater::createFileComplement(const wxString& qexe, const wxString& fcreator
///											, const wxString& fout):1,
///
wxString Creater::executeAndGetOutputAndError(const wxString& _command, bool _prepend_error)
{
	wxArrayString output;
	wxArrayString error;
	wxExecute(_command, output, error, wxEXEC_NODISABLE);

	wxString str_out;

	if ( _prepend_error && !error.IsEmpty())
		str_out += GetStringFromArray(error, Lf);

	if (!output.IsEmpty())
		str_out += GetStringFromArray(output, Lf);

	if (!_prepend_error && !error.IsEmpty())
		str_out += GetStringFromArray(error,  Lf);

	return  str_out;
}

///-----------------------------------------------------------------------------
