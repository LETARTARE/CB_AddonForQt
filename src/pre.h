/*************************************************************
 * Name:      pre.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2020-10-05
 * Copyright: LETARTARE
 * License:   GPL
 *************************************************************
 */

#ifndef _PRE_H_
#define _PRE_H_
//------------------------------------------------------------------------------
#include <wx/arrstr.h>
#include <time.h>
//------------------------------------------------------------------------------
class cbProject;
class CompileTargetBase;
class ProjectBuildTarget;
class MacrosManager;
class Manager;
class ProjectFile;
class TextCtrlLogger;
//------------------------------------------------------------------------------

/**	\class Pre
 *	@brief The class that is used to detect Qt libraries
 *
 */
class Pre
{
	public:

		/** \brief Constructor
         * @param _nameplugin  The plugin name.
         * @param _logindex The active log page.
         */
		Pre(const wxString & _nameplugin, int _logindex);
		/** Destructor
		  */
		virtual ~Pre();

		/** \brief Locate and call a menu from string (e.g. "/Valgrind/Run Valgrind::MemCheck")
		 *  it's a copy of 'CodeBlocks::sc_globals.cpp::CallMenu(const wxString& menuPath)'		 *
         *  \param _menuPath
		 *  \return menu identificateur or -1 il failed
         */
        int CallMenu(const wxString& _menuPath);

		/** \brief Plugin name
		 * @return name
		 */
		wxString namePlugin();

		/** \brief Set 'm_pProject'
		 * @param _pProject :
		 */
		void setProject(cbProject * _pProject);
		/** \brief Set 'm_pBuildTarget'
		 * @param _pBuildTarget :
		 */
		void setBuildTarget(ProjectBuildTarget * _pBuildTarget);

		/** \brief Give a full complement filename
		 * @param _file : short name
		 * @return a full filename
		 */
		wxString fullFilename(const wxString & _file);
		/** \brief Test if complement filename
		 * @param _file : short name
		 * @return true : is a complement file
		 */
        bool isComplementFile(const wxString & _file);
        /** \brief Test if creator filename
		 * @param _file : short name
		 * @return true if is a creator file
		 */
        bool isCreatorFile(const wxString & _file);
        /** \brief Generate the name of the complement file
		 *	 @param _file : file name creator
		 *   @return file name complement
		 */
		wxString nameCreated(const wxString& _file);

        /** \brief Give complement directory
		 * @return directory name
		*/
		wxString complementDirectory() const;

        /**
         * @return true if tables are empties
         */
        bool isClean();

		/** \brief Give if target is command only
		 *	 @param _nametarget : target name
		 *   @return true if it's command target
		 */
		bool isCommandTarget(const wxString& _nametarget) ;
		bool isCommandTarget(const ProjectBuildTarget * _pBuildTarget) ;
        /** \brief Give if target is virtual Qt
		 *	 @param _namevirtualtarget : target name
		 *	 @param _warning : indicate a message
		 *   @return true if it's virtual
		 */
		bool isVirtualQtTarget(const wxString& _namevirtualtarget, bool _warning=false) ;

		/**  \brief Give compileable targets list for project or virtual target
		 *   @return a table : of compileable target name
		 */
		wxArrayString compileableProjectTargets();
		/**  \brief Give compileable targets list for project or virtual target
		 *	 @param _virtualtarget : a virtual target name
		 *   @return a table : of compileable target name
		 */
		wxArrayString compileableVirtualTargets(const wxString& _virtualtarget);
		/** \brief If it is a virtual target, replace it with its first real target
		 *	 @param _virtualtarget : target name
		 *	 @param _warning : indicate a message
		 *   @return true if it's virtual
		 */
		bool virtualToFirstRealTarget(wxString& _virtualtarget, bool _warning=false) ;

		/** \brief Detects if the project has at least one target using Qt libraries
		 * refuses QT projects using a 'makefile'
         * @param _pProject : the active project.
         * @param _report : display report if true.
         * @return true : if used
         */
		bool detectQtProject(cbProject * _pProject, bool _report = false);
		/** \brief Detects if the current target uses Qt libraries,
         * @param _nametarget : the active target.
         * @param _report : display report if true.
         * @return true : if used
         */
		bool detectQtTarget(const wxString& _nametarget, bool _report = false);

		/** \brief Detects if the project has complements on disk
         * @param _pProject : the active project.
         * @param _nametarget : the active target.
         * @param _report : display report if true.
         * @return true : if complement on disk
         */
		bool detectComplementsOnDisk(cbProject * _pProject,
										const wxString & _nametarget=wxEmptyString,
										bool _report = true);

		/** \brief refresh all tables from 'm_Filewascreated'
		 * @param _debug : display report if true.
		 * @return true if ok
		 */
		bool refreshTables(bool _debug = false);

		/** \brief Get SDK version from 'cbplugin.h'
		 * @return version du SDK ex: "1.19.0" for Code::Blocks 13.12
		 */
		wxString GetVersionSDK();

		/**  \brief Set page index to log
		 * @param _logindex : page index
		 */
		void SetPageIndex(int _logindex);

		/** \brief setAbort complement file creating
		  * @param _abort
		 */
		void setAbort(bool _abort) ;

		/** \brief Startup duration
		 * @param _namefunction : used function name
         */
		void beginDuration(const wxString & _namefunction = wxString());

		/** \brief End duration
		 * @param _namefunction : used function name
         */
		void endDuration(const wxString & _namefunction = wxString());

		/** \brief check that the filename not contain illegal characters for windows !
		 *  \param _namefile :
		 * \return numbers characters
         */
		int filenameOk(wxString & _namefile);

	protected :

		/** \brief Give an array from another
         *	@param _strarray : array name
         *	@param _nlcopy : number of lines to copy, if 0 -> all
         *	@return a copy, not an adress
         */
		wxArrayString copyArray (const wxArrayString& _strarray,  int _nlcopy = 0)  ;

		/** \brief give a string of table 'title' for debug
		 *	@param _title : titlr name
		 *	@param _strarray : array name
		 *  @return one string of array
		 */
		wxString allStrTable(const wxString& _title, const wxArrayString& _strarray);

		/** \brief Construct 'm_FilesCreator' from 'm_Filewascreated'
         *	@return a reverse,
         */
		wxArrayString wasCreatedToCreator() ;
		/** \brief Give an string from another
         *	@param _fcreated : file created name
         *	@return a reverse,
         */
		wxString toFileCreator(const wxString & _fcreated) ;
		/** \brief Give a full relative name
         *	@param _fcreator : file name
         *	@param _creatortarget 	: creator target
         *	@return relative name
         */
		wxString fullFileCreator(const wxString&  _fcreator, wxString _creatortarget=wxEmptyString) ;

		/**  \brief Give a duration
		 *   @return duration mS
		 */
		wxString duration();

	public:
		/**  \brief Give a date
		 *   @return date
		 */
		wxString date();

		/** \brief name of plugin
		 */
		wxString m_namePlugin = wxEmptyString;
		/** \brief platforms Windows
		 */
		bool m_Win = false,
		/** \brief platforms Linux
		 */
			 m_Linux = false,
		/** \brief platforms Mac
		 */
			 m_Mac = false
			 ;
	protected:
		/** \brief the active project
		 */
		cbProject * m_pProject = nullptr;
		/** \brief the active build target
		 */
		ProjectBuildTarget * m_pBuildTarget = nullptr;

		/**  \brief project directory absolute path
		 */
		wxString  m_dirProject = wxEmptyString,
		/**  \brief generation complements directory
		 */
				  m_dirPreBuild = wxEmptyString,
		/**  \brief generation objects directory
		 */
				  m_dirObjects = wxEmptyString,
		/**  \brief project name,
		 */
                  m_nameProject = wxEmptyString,
		/** \brief Contains active target name
		 */
                  m_nameActiveTarget = wxEmptyString;
		/**  \brief numbers files projects
		 */
		uint16_t m_nfilesCreated = 0;
		/** \brief executable name files Qt : 'moc'
		 */
		wxString m_Mexe = wxEmptyString,
		/** \brief executable name files Qt : 'uic'
		 */
				m_Uexe = wxEmptyString,
		/** \brief executable name files Qt : 'rcc'
		 */
				m_Rexe = wxEmptyString,
		/** \brief executable name files Qt : 'lrelease'
		 */
				m_Lexe = wxEmptyString;
		/** \brief files prefix for 'moc'
		 */
		wxString m_Moc = "moc",
        /** \brief files prefix for 'uic'
		 */
				 m_UI = "ui",
		/** \brief files prefix for 'rcc'
		 */
				 m_Qrc = "qrc",
		/** \brief files extension for 'lrelease'
		 */
				 m_Lm = "qm";

		/**  \brief manager
		 */
		Manager * m_pm = nullptr;
		/**  \brief macros manager
		 */
		MacrosManager  * m_pMam = nullptr;

		/** \brief table contains libray name Qt
		 */
		wxArrayString  m_TablibQt ;

		/** \brief String with "__void__"
		 */
		const wxString m_Devoid = "__void__" ;

		/// the tables
		wxArrayString
			/** \brief Creators files
			 */
			m_Filecreator,
			/** \brief Files to be created (complement) in the project
			 */
			m_Filestocreate,
			/** \brief Files created list
			 */
			m_Createdfile,
			/**	\brief Files registered in the project
			 */
			m_Registered,
			/** \brief File was already created
			 */
			m_Filewascreated
		;
		/**  \brief the table is empty
		 */
		bool m_clean = false;

		/**  \brief messages to console
		 */
		wxString Mes = wxEmptyString
				,SaveMes = wxEmptyString;

		/** \brief log page index
		 */
		int	m_LogPageIndex = 0;

		/**  \brief calculate duration  mS
		 */
		clock_t m_start;

		/**  \brief Abort complement file creating
		 */
		bool m_abort = false;

	public:

        /** \brief Search Qt libraries in project or target
         *  @param _pContainer: 'cbProject * Project' or 'ProjectBuildTarget * buildtarget'
         *		both inherited of 'CompileTargetBase'
		 *  @return true if finded
         */
		bool hasLibQt(CompileTargetBase * _pContainer) ;

		/**  \brief Adjusts variable depending on the platform
		 *  \return
		 */
		wxString platForm() ;
};

#endif // _PRE_H_
