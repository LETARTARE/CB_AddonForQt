/***************************************************************
 * Name:      creater.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2022-04-22
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#ifndef _CREATER_H
#define _CREATER_H
//------------------------------------------------------------------------------
#include "pre.h"
//------------------------------------------------------------------------------
/**	\class Creater
 *	@brief The class is used to pre-creater additional Qt files
 *
 */
class Creater  : public Pre
{
	public :

		/** \brief Constructor
         *  @param _namePlugin : The plugin name
         *  @param _logIndex : The active log
         */
		Creater(const wxString & _namePlugin, int _logIndex);

		/** \brief Destructor
         */
		virtual ~Creater();

		/** \brief  Give if target is a Qt target
		 *	 @param _pBuildTarget : a project target
		 *   @return true if it's
		 */
		bool isGoodTargetQt(const ProjectBuildTarget * _pBuildTarget);
		/** \brief Give if file  is elegible
		 *	 @param _file : file name
		 *   @return true if it's
		 */
		bool isElegible(const wxString& _file);
		/** \brief Built all files necessary complements
		 *  @param _pProject The active project.
		 *  @param _workspace is true -> no report, no popup messages.
		 *  @param _allbuild is true -> rebuild complement files.isGoodTargetQt
		 *  @return	true if building is correct
		 */
		bool buildAllFiles(cbProject * _pProject, bool _workspace, bool _allbuild);
		/** \brief Built one file elegible necessary complement
		 *  @param _pProject The active project.
		 *  @param _fcreator  creator file
		 *  @return	true if building is correct
		 */
		bool buildOneFile(cbProject * _pProject, const wxString& _fcreator);
		/** \brief Clean one file elegible necessary complement
		 *  @param _pProject The active project.
		 *  @param _fcreator  creator file
		 *  @return	true if cleannin is correct
		 */
		bool cleanOneFile(cbProject * _pProject, const wxString& _fcreator);

		/** \brief Unregister a project file creator to 'Pregen'
		 *  @param _file : file name  (creator)
		 *  @return	true if correct
		 */
		bool unregisterCreatorFile(const wxString & _file);
		/** \brief Unregister a project file complement to 'Pregen'
		 *  @param _file : file name  (complement)
		 *  @return	true if correct
		 */
		bool unregisterComplementFile(wxString & _file);

		/**	\brief Remove one complement file to disk directory "m_dirPreBuild"
		 *  @param  _filename : complement file name
		 *  @param  _withobject :  a compiled file exists
		 *  @return	true if correct
         */
		bool removeComplementToDisk(const wxString & _filename, bool _withobject = false);

		/**	\brief Remove one complement objet file to disk directory m_ditObject'
		 *  @param  _filename : complement file name
		 *  @return	true if correct
         */
		bool removeComplementObjToDisk(const wxString & _filename);

		/** \brief Unregister an old target to 'CB'
		 *  @param _oldTargetName : old target name
		 *  @param _pProject ; the parent project
		 *  @return	true if correct
		 */
		bool unregisterAllComplementsToCB(const wxString & _oldtTargetName, cbProject * _pProject);
		/**	\brief Remove complement directory file to disk directory "m_dirPreBuild"
		 *  @param  _oldTargetName : target name
		 *  @return	true if correct
         */
		bool removeComplementDirToDisk(const wxString & _oldTargetName);

		/** \brief Remove old path include table for executable
		 *  @param _pContainer:  builtarget
		 *  @param _oldTargetName : old target name
		 *  @return true if correct
		 */
		bool removeOldPathtoCB(ProjectBuildTarget* _pContainer, const wxString & _oldTargetName);
		/** \brief Remove old executable
		 *  @param _pBuildTarget:  builtarget
		 *  @param _oldTargetName : old target name
		 *  @return true if correct
		 */
		bool removeOldExecutable(ProjectBuildTarget* _pBuildTarget, const wxString & _oldTargetName);

		/** \brief Recursively deletes non-empty directories
		 *	 @param _rmDir : dirrectory name
		 *   @return true if it's
		 */
		bool recursRmDir(wxString _rmDir) ;

		/** \brief Total updates of the new target
		 *			- Unregister old complement files with CB
		 *			- Unregister old complement files with the plugin (m_Registered, ...)
		 *			- Remove old path include for new target

		 *			- Removes old complements from the disk in 'm_dirPreBuild\\oldNameTarget\\*.*'
		 *			- Deletes the old directories from the disk 'm_dirPreBuild\\oldNameTarget'
		 *			- Removes objects (*.o) from the disk that are no longer used
		 *			- Removes executables from the disk that are no longer used
		 *  @param _pBuildTarget : new target build
		 *  @param _oldTargetName : old target name
		 *  @return true if correct
		 */
		bool updateNewTargetName(ProjectBuildTarget * _pBuildTarget, const wxString & _oldTargetName);


	private:

		/** \brief Startup banner pre-building
         */
		void beginMesBuildCreate();

		/** \brief Banner End pre-building
         */
		void endMesBuildCreate();

		/** \brief Startup banner pre-compiling file
         */
		void beginMesFileCreate();

		/** \brief Banner End pre-compiling file
         */
		void endMesFileCreate();

		/**	\brief Search the eligible files (which requires additional files)
		 *  @return  the number of eligible files
         */
		wxUint16 findGoodfiles();

		/** \brief Record one file elegible necessary complement
		 *  @param _fcreator  creator file
		 *  @param _fout  complement file
		 *  @return true if building is correct
		 */
		bool addOneFile(const wxString& _fcreator, const wxString& _fout) ;

		/**	\brief Records in an internal table to build additional files
		 *  @return	the number of recorded files
         */
		wxUint16 addAllFiles();

		/**	\brief Searches for files to be created
		 *  @param _allrebuild  : true -> rebuild everything, false -> reconstruct
		 *						only files whose creators (eligible) have changed
		 *  @return	the number of files to be created
         */
		wxUint16 filesTocreate(bool _allrebuild) ;

        /** \brief Creating complements files
         *  @return	true if creating is correct
         */
		bool createFiles() ;

		/**  \brief Checking complements files
         *  @return	true if checking is correct
         */
		bool validCreated() ;

		/**  \brief Reference string of libQt by target: "" or "4r" or "4d" or "5r" or "5d"
		 *	 @param _pBuildTarget : 'ProjectBuildTarget*'
		 *   @return reference string
		 */
		wxString refTargetQt(const ProjectBuildTarget * _pBuildTarget) ;

		/**  \brief Find container path
		 *	 @param _pContainer : 'cbProject*' or 'ProjectBuildTarget*'
		 * 	 	both inherited of 'CompileTargetBase'
		 *   @return path
		 */
		wxString pathlibQt(CompileTargetBase * _pContainer) ;

		/**  \brief Is whether the QT executables exist
		 *	 @param _pBuildTarget : 'CompileTargetBase*' of used target
		 *   @return true if they exist all
		 */
		bool findTargetQtexe(CompileTargetBase * _pBuildTarget) ;

		/**  \brief Read contents file
		 *	 @param _filename : file name
		 *   @return contents file
		*/
		wxString ReadFileContents(const wxString& _filename) ;

		/** \brief Search a Qt macro inside file
		 *	 @param _filename : file name
		 *	 @param _qt_macro : the macro (Q_OBJECT, Q_GADGET ...)
		 *   @return  number macros
		 */
		int q_object(const wxString& _filename, const wxString& _qt_macro) ;

		/** \brief Search a text inside a string
		 *	 @param _tmp : string
		 *	 @param _qt_text : the text
		 *	 @return  occurency number
		 */
		int  qt_find (wxString _tmp, const wxString& _qt_text) ;

		/** \brief Indicates whether the file is already registered in the CB project
		 *  @param _file : file name
		 *  @return if it's
		 */
		bool inProjectFileCB(const wxString& _file) ;

		/** \brief Indicates whether the file is already included in another
		 *	 @param _fcreator : file name
		 *   @return true if it's
		 */
		bool hasIncluded(const wxString& _fcreator) ;
		bool verifyIncluded(wxString& _source, wxString& _txt) ;

		/** \brief Create a directory
		 *	@param _dircomplement : dir name
		 *  @return true if it's correct
		 */
		bool createDir (const wxString& _dircomplement) ;

		/** \brief Remove a directory
		 *	@param _dirgen : directory name
		 *  @return true if it's correct
		 */
		bool removeDir (const wxString&  _dirgen ) ;

		/** \brief Compare both date files
		 *	@param  _fileref : file name reference
		 * 	@param  _filetarget : file name target
		 *  @return true if identical dates
		 */
		bool compareDate(const wxString& _fileref, const wxString& _filetarget) ;

		/** \brief  Modify date file target
		 *	@param  _fileref : file name reference
		 * 	@param  _filetarget : file name target
		 *  @return true if changing it's correct
		 */
		bool modifyDate(const wxString& _fileref, const wxString& _filetarget)  ;

		/** \brief Indicates whether a table is empty
		 *  @param _arraystr : a table
		 *  @return if it's
		 */
		bool isNoFile(const wxArrayString& _arraystr) ;

		/** \brief Search the number files to create
		 *  @param _arraystr : a table
		 *  @return the number
		 */
		wxUint16 nfilesToCreate(const wxArrayString& _arraystr) ;

        /** \brief Execute commands 'moc', 'uic', 'rcc'
         *	 @param _qexe  : executable name
         *	 @param _index : position creator file inside 'm_Filecreator'
         *   @return  "" if file created else error string
         */
		wxString createComplement(const wxString& _qexe, const wxUint16 _index) ;

		/** \brief Execute commands 'moc', 'uic', 'rcc'
         *	@param _qexe : executable name
         *	@param _fcreator : file name creator
         *	@param _fout : file name to create
         *  @return _"" if file created else error string
         */
		wxString createFileComplement(const wxString& _qexe, const wxString& _fcreator,
									  const wxString& _fout) ;

		/** \brief Retrieve path include table for executable
		 *  @param _pContainer : receptacle
		 *  @return string path include
		 */
		wxString pathIncludeMoc(const CompileTargetBase * _pContainer);

		/** \brief Retrieve all defines table for executables
		 *  @param _pContainer : receptacle
		 *  @return string all defines
		 */
		wxString definesMoc(CompileTargetBase * _pContainer) ;

        /**  \brief Launch a command line
         *	 @param _command : command line
         *	 @param _prepend_error : prepend error to return
         *   @return  empty string if it's correct else error message
         */
		wxString executeAndGetOutputAndError(const wxString& _command, bool _prepend_error = true) ;

	private:

		/** \brief Indicate if identical dates
		 */
		bool m_Identical = false;

		wxString
		/** \brief Contains all defines Qt for executables
		 */
			m_DefinesQt,
		/** Contains all path include Qt for executables
		 */
			m_IncPathQt,
		/** \brief Contains file name
		 */
			m_filename
		;
};

#endif // _CREATER_H
