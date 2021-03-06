/***************************************************************
 * Name:      defines.h
 * Purpose:   Code::Blocks plugin
 * Author:    LETARTARE
 * Created:   2015-10-17
 * Modified:  2020-06-27
 * Copyright: LETARTARE
 * License:   GPL
 **************************************************************/

///-----------------------------------------------------------------------------
#ifndef _DEFINES_H_
	#define _DEFINES_H_

///-----------------------------------------------------------------------------
/** Version
 */
#define VERSION_WXT wxString("'3.3.0'")

/** \brief display begin and end messages of function
 */
//#define WITH_MES_DEBUG

/** @brief end of line for Win/Linux/OX
 */
#define 	Cr 		wxString("\r")
#define 	Lf 		wxString("\n")
#define 	CrLf 	wxString("\r\n")
#define 	Eol 	wxString("\r\n")
#define 	Quote 	wxString("'")
#define	Dquote  wxString("\"")
#define 	Tab	wxString("\t")
#define 	Space	wxString(" ")
#define 	Point	wxString(".")
#define 	Dot	wxString(".")
#define 	dot		'.'
/** \brief text separator
 */
#define 	SepD 	char(13) 	// 0xD, \n
#define 	SepA 	char(10)	// 0xA, \r
#define 	SizeSep 2
#define	SizeLe	16
/** @brief surrounded by 'Quote'
 */
#define 	quote(a)	(Space + Quote + wxString(a) + Quote + Space)
#define 	quoteNS(a)	(Quote + wxString(a) + Quote)
#define 	dquote(a)	(Space + Dquote + wxString(a) + Dquote + Space)
/** \brief  for print an integer and a boolean
 */
#define strInt(a)	(wxString()<<a)
#define strBool(a)	(wxString()<<(a==0 ? _("false"): _("true") ))
#define strChar(a)	wxString(a)

#include <wx/filefn.h>
/** @brief directory separator
 *  use "...."  +  strSlash   : not  "...."  +  Slash !!
 */
#define 	Slash 		wxFILE_SEP_PATH
#define 	strSlash  	wxString(Slash)
///-----------------------------------------------------------------------------
#include <logmanager.h>
#define lm				Manager::Get()->GetLogManager()
/** @brief messages  -> 'Code::Blocks log'
 */
#define Print			lm->Log
#define PrintLn		lm->Log("")
#define PrintWarn		lm->LogWarning
#define PrintError		lm->LogError
/** @brief messages  -> 'Code::Blocks Debug log'
 */
#define DPrint			lm->DebugLog
#define DPrintErr		lm->DebugLogError

/** @brief messages  -> 'PreBuild log'
 */
#define p               m_LogPageIndex
#define print(a)	    lm->Log(a, p)
#define printLn		    lm->Log("", p)
#define printWarn(a)	lm->LogWarning(a, p)
#define printError(a)	lm->LogError(a, p)
#ifdef WITH_MES_DEBUG
	#define printD(a)		lm->Log(a, p)
	#define printWarnD(a)	lm->LogWarning(a, p)
#else
	#define printD(a)
	#define printWarnD(a)
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
#define  	EXT_H 			FileFilters::H_EXT
#define  	EXT_HPP 		FileFilters::HPP_EXT
#define  	EXT_CPP 		FileFilters::CPP_EXT
#define 	DOT_EXT_H 		FileFilters::H_DOT_EXT
#define 	DOT_EXT_CPP 	FileFilters::CPP_DOT_EXT
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
