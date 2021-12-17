/***************************************************************
 * Name:      addonforqt.h
 * Purpose:   Code::Blocks plugin '
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2021-12-15
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/
#ifndef _ADDONFORQT_H_
#define _ADDONFORQT_H_
//-------------------------------------------------
// For compilers that support precompilation, includes <wx/wx.h>
#include <wx/wxprec.h>
// For Linux
#include <wx/xrc/xmlres.h>  // XRCID
//-------------------------------------------------
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif
// for class cbPlugin
#include <cbplugin.h>

#include <wx/event.h>
/** \brief Defines the futur current state of the compiler.
 *  \brief to use with 'AddOnForQt::OnMenuComplements(wxCommandEvent& _event)'
 */
enum cbFutureBuild
{
    fbNone = 10,
    fbBuild,
    fbClean,
    fbRebuild,
    fbWorkspaceBuild,
    fbWorkspaceClean,
    fbWorkspaceReBuild
};


//-------------------------------------------------
class Pre;
class Build;
class TextCtrlLogger;
//-------------------------------------------------
/** \class  AddOnForQt
 * @brief Qt plugin main class
*/
class AddOnForQt : public cbPlugin
{
  public:
    /** Constructor */
    AddOnForQt();

    /** \brief Destructor
      */
    virtual ~AddOnForQt() {}

    /** \brief Invoke configuration dialog.
      */
    virtual int Configure() {return 0;}

    /** Return the plugin's configuration priority.
      * This is a number (default is 50) that is used to sort plugins
      * in configuration dialogs. Lower numbers mean the plugin's
      * configuration is put higher in the list.
      */
    virtual int GetConfigurationPriority() const { return 50; }

    /** Return the configuration group for this plugin. Default is cgUnknown.
      * Notice that you can logically OR more than one configuration groups,
      * so you could set it, for example, as "cgCompiler | cgContribPlugin".
      */
    virtual int GetConfigurationGroup() const { return cgContribPlugin; }

    /** Return plugin's configuration panel.
      * @param parent The parent window.
      * @return A pointer to the plugin's cbConfigurationPanel. It is deleted by the caller.
      */
    virtual cbConfigurationPanel* GetConfigurationPanel(wxWindow* parent) { return 0 ;}

    /** Return plugin's configuration panel for projects.
      * The panel returned from this function will be added in the project's
      * configuration dialog.
      * @param parent The parent window.
      * @param project The project that is being edited.
      * @return A pointer to the plugin's cbConfigurationPanel. It is deleted by the caller.
      */
    virtual cbConfigurationPanel* GetProjectConfigurationPanel(wxWindow* parent, cbProject* project){ return 0 ;}

    /** \brief This method is called by Code::Blocks and is used by the plugin
      * to add any menu items it needs on Code::Blocks's menu bar.\n
      * It is a pure virtual method that needs to be implemented by all
      * plugins. If the plugin does not need to add items on the menu,
      * just do nothing ;)
      * @param menuBar the wxMenuBar to create items in
      */
    virtual void BuildMenu(wxMenuBar* _menuBar);

    /** \brief This method is called by Code::Blocks core modules (EditorManager,
      * ProjectManager etc) and is used by the plugin to add any menu
      * items it needs in the module's popup menu. For example, when
      * the user right-clicks on a project file in the project tree,
      * ProjectManager prepares a popup menu to display with context
      * sensitive options for that file. Before it displays this popup
      * menu, it asks all attached plugins (by asking PluginManager to call
      * this method), if they need to add any entries
      * in that menu. This method is called.\n
      * If the plugin does not need to add items in the menu,
      * just do nothing ;)
      * @param type the module that's preparing a popup menu
      * @param menu pointer to the popup menu
      * @param data pointer to FileTreeData object (to access/modify the file tree)
      */
    virtual void BuildModuleMenu(const ModuleType _type, wxMenu* _menu, const FileTreeData* _data = 0);

    /** \brief This method is called by 'Code::Blocks' and is used by the plugin
      * to add any toolbar items it needs on Code::Blocks's toolbar.\n
      * It is a pure virtual method that needs to be implemented by all
      * plugins. If the plugin does not need to add items on the toolbar,
      * just do nothing ;)
      * @param toolBar the wxToolBar to create items on
      * @return The plugin should return true if it needed the toolbar, false if not
      */
    virtual bool BuildToolBar(wxToolBar* _toolBar);

     /** @brief Execute the plugin.
      *
      * This is the only function needed by a 'cbToolPlugin'
      * This will be called when the user selects the plugin from the "Plugins"
      * menu.
      */
    virtual int Execute() { return 0 ;}

  protected:
    /** \brief Any descendent plugin should override this virtual method and
      * perform any necessary initialization. This method is called by
      * Code::Blocks (PluginManager actually) when the plugin has been
      * loaded and should attach in Code::Blocks. When Code::Blocks
      * starts up, it finds and <em>loads</em> all plugins but <em>does
      * not</em> activate (attaches) them. It then activates all plugins
      * that the user has selected to be activated on start-up.\n
      * This means that a plugin might be loaded but <b>not</b> activated...\n
      * Think of this method as the actual constructor...
      */
    virtual void OnAttach();

    /** \brief Any descendent plugin should override this virtual method and
      * perform any necessary de-initialization. This method is called by
      * Code::Blocks (PluginManager actually) when the plugin has been
      * loaded, attached and should de-attach from Code::Blocks.\n
      * Think of this method as the actual destructor...
      * @param appShutDown If true, the application is shutting down. In this
      *         case *don't* use Manager::Get()->Get...() functions or the
      *         behaviour is undefined...
      */
    virtual void OnRelease(bool appShutDown);

  /// --------------------------------------------------------------------------

    /// il faudrait hériter de 'cbCompilerPlugin()' qui ne possède que des
    /// virtuelles pures dans 'cbplugin.h"
     int KillProcess();

     /** \brief Enable alls menus bar for Qt
      *  \param _valid : true => enable all menus
      */
    void enableAllMenubarQt(const bool _valid);
    void enabledAllMenuBuildCB(const bool _valid);

    /** \brief This method called by menu clicked
     *  \param _event :
     */
    void OnMenuComplements(wxCommandEvent& _event) ;
    /** \brief This method called by 'OnMenuComplements(wxCommandEvent& _event)'
     *  \param _domake : order to make
     *  \return true if ok
     */
    bool doComplements(const cbFutureBuild&  _domake);

/// menu entries constante identificator
  //
    static const long idMenuQt ;

    static const long idFirstState;
    static const long idActionStop;
  // projects
    static const long idBuildAddons;
    static const long idRebuildAddons ;

    static const long idBuildAddonsAlone;
    static const long idRebuildAddonsAlone ;
    static const long idCleanAddonsAlone ;
  // file
    static const long idBuildOneAddin;
    static const long idCleanOneAddin;
  // workspace
    static const long idBuildWSAddons;
    static const long idCleanWSAddons ;
    static const long idRebuildWSAddons ;

    static const int idKillProcess;

    DECLARE_EVENT_TABLE();

public :
    /** \brief Defines the current state of the 'qtAddons'
   *  \brief to use with state graph of 'qtAddons'
   */
  enum qtState
  {
      fbWait = 10,             /// idle
      fbStop,                       /// stop state
      fbWaitBuild,              /// wait state  for  build
      fbActionBuild,           /// build complements
      fbWaitRebuild,           /// wait state  for  rebuild
      fbActionRebuild,        /// rebuild complements
      fbWaitClean,             ///  wait sate for clean
      fbActionClean,          ///  clean complement
      fbWaitBuildWS,            /// wait state  for workspace build
      fbActionBuildWS,         ///  workspace build
      fbWaitCleanWS,        /// wait state  for workspace clean
      fbActionCleanWS,     /// workspace clean    fbWaitRebuildWS,       /// wait state  for workspace rebuild
      fbWaitRebuildWS,
      fbActionRebuildWS   ///  workspace rebuild
  };

    qtState m_State;

protected:

  /** -------------------- personal methods --------------------------------- */
      /** \brief Enable the menu items of '&Collect'
      * \param _state :
     */
    void enableMenus(qtState _state);

    /** \brief This method called by end start application
      * @param _event Contains the event which call this method
      */
  //  void OnAppDoneStartup(CodeBlocksEvent& _event);

    /** \brief This method called by plugin is manually loaded
      * @param _event Contains the event which call this method
      */
    void OnPluginLoaded(CodeBlocksEvent& _event);

    /** \brief This method called by loading complete
      * @param _event Contains the event which call this method
      */
    void OnPluginLoadingComplete(CodeBlocksEvent& _event);

    /** \brief This method called by begin shutdown application
      * @param _event Contains the event which call this method
      */
    void OnAppBeginShutDown(CodeBlocksEvent& _event);

    /** \brief This method called by
      * 'cbEVT_PROJECT_OPEN' for notifie open project
      * @param _event Contains the event which call this method
      */
  //  void OnOpenProject(CodeBlocksEvent& _event);

    /** \brief This method called by
      * 'cbEVT_PROJECT_CLOSE' for notifie close project
      * @param _event Contains the event which call this method
      */
 //   void OnCloseProject(CodeBlocksEvent& _event);

    /** \brief This method called by
      * 'cbEVT_PROJECT_SAVE' for notifie save project
      * @param _event Contains the event which call this method
      */
    void OnSaveProject(CodeBlocksEvent& _event);

   /** \brief This method called by
    * 'cbEVT_EDITOR_SAVE' for notifie save file
    * @param _event Contains the event which call this method
    */
    void OnSaveFileEditor(CodeBlocksEvent& _event);

    /** \brief This method called by project activate allows detect project using the
      * Qt libraries
      * @param _event Contains the event which call this method
      */
    void OnActivateProject(CodeBlocksEvent& _event);

    /** \brief This method called by target activate allows detect target using the
      * Qt libraries
      * @param _event Contains the event which call this method
      */
    void OnActivateTarget(CodeBlocksEvent& _event);

    /** This method called by a new project allows detect active project and
	    * active target
      * @param _event Contains the event which call this method
      */
    void OnNewProject(CodeBlocksEvent& _event);

    /** \brief This method called by
      * 'cbEVT_PROJECT_RENAMED' for notifie rename project or
      * 'cbEVT_BUILDTARGET_RENAMED' for notifie rename target
      * @param _event Contains the event which call this method
      */
    void OnRenameProjectOrTarget(CodeBlocksEvent& _event);

    /** \brief This method called by 'cbEVT_PROJECT_FILE_REMOVED' for
      * allows remove complement file
      * @param _event Contains the event which call this method
      */
    void OnBeginFileRemoved(CodeBlocksEvent& _event);
    void OnProjectFileRemoved(CodeBlocksEvent& _event);
    void OnEndFileRemoved(CodeBlocksEvent& _event);


    /** \brief This method called by 'cbEVT_WORKSPACE_CLOSING_BEGIN' for
      * no analyse by AddOnForQt
      * @param _event Contains the event which call this method
      */
    void OnWorkspaceClosed(CodeBlocksEvent& _event);
     /** \brief This method called by 'cbEVT_WORKSPACE_LOADING_COMPLETE' for
      * allows analyse by AddOnForQt
      * @param _event Contains the event which call this method
      */
    void OnWorkspaceComplete(CodeBlocksEvent& _event);

    /** \brief This method called by 'cbEVT_COMPILER_FINISHED' for
      *   1. abort pre-build all the additional files ...
      *   2. abort pre-compile one additional file ...
      * @param _event Contains the event which call this method
      */
    void OnAbortAdding(CodeBlocksEvent& _event);
    /** \brief This method called by
      *   1. abort pre-build all the additional files ...
      *   2. abort pre-compile one additional file ...
      * @param _event Contains the event which call this method
      */
    void OnUpdateUI(wxUpdateUIEvent& event);
    /** \brief
     *  \return true if 'adding complement'
     */
    bool IsRunning() const ;
    /** \brief
     *  \return true 'Abort' is enabled
     */
    bool getAbort();
    /** \brief
     *  \param _enabled : true for enebled
     */
    void setAbort(bool _enabled);

  private:
    /** \brief read the bar menu
     */
    wxMenuBar* m_menuBar = nullptr;
    /** \brief items menu 'Build'
     */
    wxMenu * m_buildMenu = nullptr;
    bool m_buildFinded = false;
    /** \brief read the tools bar
     */
    wxToolBar* m_toolBar = nullptr;

    /** \brief actual project
     */
    cbProject* m_pProject = nullptr;
    wxString m_activeNameProject = wxEmptyString;
    wxString m_activeNameTarget = wxEmptyString;
    /** \brief pre-Builder for QT
     */
    Build * m_pBuild = nullptr;
    /** \brief project and target types
      */
    bool m_isQtProject = false
         ,m_isQtActiveTarget = false
         ,m_isNewProject = false
         ;
    /**  removing complement files
     */
    bool m_removingIsFirst = false , m_iscomplement = false ;
    /**  \brief a pseudo event
     */
    bool m_pseudoEvent = false;
    /**  \brief app init done, all plugins loading
      */
    bool m_initDone = false;
    /**  \brief no parsing while workspace changed
      */
    bool m_noParsing = true;
    /** \brief state build running
      */
    bool m_isRunning = false ;

    /** \brief Global used to record different messages for the construction report
     */
    wxString Mes = wxEmptyString
				    ,SaveMes = wxEmptyString;

     /** \brief Save the file name
     */
    wxString m_fileCreator;

  private:
    /**  \brief Append log message to 'PreBuild log'
     *  @param  _Text : message to display
     *  @param  _lv : level -> Logger::caption, info, warning, success, error,
     *               critical, failure, pagetitle, spacer, asterisk
     */
    void AppendToLog(const wxString& _Text, Logger::level _lv = Logger::info);

    /** \brief Show a log
     *  @param _indexLog : page index
     */
    void SwitchToLog(int _indexLog);
    void SwitchToMyLog();

    /** \brief Emit an abort for 'CompilerGCC' class
      * @param _idAbort : Abort identificator
     */
    void compilingStop(int _idAbort);

    /** \brief Give a string for a 'cbFutureBuild'
      * @param _domake : a 'cbFurureBuild'
     */
    wxString domakeToStr(const cbFutureBuild&  _domake);

  private:

    /** \brief Log tab in the message pane
     */
    TextCtrlLogger* m_AddonLog = nullptr;

    /** \brief Index of our log tab (can this change during run time ??)
     */
    size_t m_LogPageIndex = 0,
    /**  Last log index
     */
        m_LastIndex = 0
        ;

    /**  \brief last log
     */
    Logger * m_Lastlog = nullptr;

    /**  \brief Manager log
     */
    LogManager* m_LogMan = nullptr;

    /** \brief valid message to 'Prebuild log'
     */
    bool m_WithMessage = false;
};

#endif // _ADDONFORQT_H_
//-------------------------------------------------
