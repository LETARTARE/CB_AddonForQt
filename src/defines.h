/***************************************************************
 * Name:      defines.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2023-01-18
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/

///-----------------------------------------------------------------------------
#ifndef _DEFINES_H_
	#define _DEFINES_H_

///-----------------------------------------------------------------------------
/** Version
 */
#define VERSION_WXT wxString("'3.6.0'")

/** Version Qt
 */
#define Qt 5
//#define Qt 6

/** \brief display begin and end messages of function
 */
//#define WITH_MES_DEBUG

/** \brief name of directory for complements
 */
#define dirAddon	"adding"

/** @brief end of line for Win/Linux/OX
 */
#define 	Cr 		wxString("\r")
#define 	Lf 		wxString("\n")
#define 	CrLf 	wxString("\r\n")
#define 	Eol 	wxString("\r\n")
#define 	Quote 	wxString("'")
#define	    Dquote  wxString("\"")
#define 	Tab	    wxString("\t")
#define 	Space	wxString(" ")
#define 	Point	wxString(".")
#define 	Dot	    wxString(".")
#define 	cDot		'.'
/** \brief text separator
 */
#define 	SepD 	char(13) 	// 0xD, \n
#define 	SepA 	char(10)	// 0xA, \r
#define 	SizeSep 2
#define	    SizeLe	16
/** @brief surrounded by 'Quote'
 */
#define 	quote(a)	(Space + Quote + wxString(a) + Quote + Space)
#define 	quoteNS(a)	(Quote + wxString(a) + Quote)
#define 	dquote(a)	(Space + Dquote + wxString(a) + Dquote + Space)
/** \brief  for print an integer and a boolean
 */
#define     strInt(__a)		(wxString()<<__a)
    #define iToStr(__a)			( wxString()<<__a )
#define     strDouble(a)	(wxString()<<a)
    #define  dToStr(__a)	(wxString()<<__a)
#define     strBool(a)		(wxString()<<(a==0 ? _("false"): _("true") ))
    #define bToStr(__b)			( wxString()<<(__b==0 ? _("false" ): _("true")) )
#define     strChar(__c)		wxString(__c)
    #define cToStr(__c)		wxString(__c)

#include <wx/filefn.h>
/** @brief directory separator
 *  use "...."  +  strSlash   : not  "...."  +  cSlash !!
 */
#define 	cSlash 		wxFILE_SEP_PATH
#define 	strSlash  	wxString(cSlash)
///-----------------------------------------------------------------------------
#include <logmanager.h>
#define lm				Manager::Get()->GetLogManager()
/** @brief messages  -> 'Code::Blocks log'
 */
#define Print			lm->Log
#define PrintLn		    lm->Log("")
#define PrintWarn		lm->LogWarning
#define PrintError		lm->LogError
/** @brief messages  -> 'Code::Blocks Debug log'
 */
#define DPrint			lm->DebugLog
#define DPrintErr		lm->DebugLogError

/** @brief messages  -> 'Addons log'
 */
#define __p               m_LogPageIndex
#define _print(__a)	    lm->Log(__a, __p)
#define _printLn		    lm->Log("", __p)
#define _printWarn(__a)	lm->LogWarning(__a, __p)
#define _printError(__a)	lm->LogError(__a, __p)
#ifdef WITH_MES_DEBUG
	#define printD(__a)		lm->Log(__a, __p)
	#define printWarnD(__a)	lm->LogWarning(__a, __p)
#else
	#define _printD(__a)
	#define _printWarnD(__a)
#endif
///-----------------------------------------------------------------------------
/** @brief news extensions
 */
#define		EXT_UI			"ui"
#define		EXT_MOC			"moc"
#define		EXT_QRC			"qrc"
#define		DOT_EXT_UI		".ui"
#define		DOT_EXT_MOC		".moc"
#define		DOT_EXT_QRC		".qrc"

#include <filefilters.h>
#define  	EXT_H 				FileFilters::H_EXT
#define  	EXT_HPP 			FileFilters::HPP_EXT
#define  	EXT_CPP 			FileFilters::CPP_EXT
#define 	DOT_EXT_H 			FileFilters::H_DOT_EXT
#define 	DOT_EXT_CPP 		FileFilters::CPP_DOT_EXT
#define     DOT_DYNAMICLIB_EXT  FileFilters::DYNAMICLIB_DOT_EXT
///-----------------------------------------------------------------------------
/** @brief booleans
 */
#define IS_COMPLEMENT		true
#define IS_NO_COMPLEMENT	false

#define IS_CREATOR			true
#define IS_NO_CREATOR		false

#define WITH_REPORT			true
#define NO_REPORT			false

#define FIX_LOG_FONT		true
#define NO_FIX_LOG_FONT		false

#define SEPARATOR_AT_END	true
#define NO_SEPARATOR_AT_END	false

#define IS_RELATIVE			true
#define NO_RELATIVE			false

#define IS_UNIX_FILENAME	true
#define NO_UNIX_FILENAME	false

#define PREPEND_ERROR		true
#define NO_REPEND_ERROR		true

#define WITH_WARNING		true
#define NO_WARNING			false

#define ALLBUILD			true
#define NO_ALLBUILD			false

#define WITH_DEBUG			true
#define NO_DEBUG			false

#define FIRST				true
#define NO_FIRST			false
///-----------------------------------------------------------------------------

#endif // _DEFINES_H_
