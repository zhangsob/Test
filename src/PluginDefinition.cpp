//this file is part of notepad++
//Copyright (C)2003 Don HO <donho@altern.org>
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "PluginDefinition.h"
#include "menuCmdID.h"

#include <vector>
#include <string>
#include "zutil.h"
#include "ZNpp.h"

//
// The plugin data that Notepad++ needs
//
FuncItem funcItem[nbFunc];

//
// The data of Notepad++ that you can use in your plugin commands
//
NppData nppData;

//
// Initialize your plugin data here
// It will be called while plugin loading   
void pluginInit(HANDLE /*hModule*/)
{
}

//
// Here you can do the clean up, save the parameters (if any) for the next session
//
void pluginCleanUp()
{
}

//
// Initialization of your plugin commands
// You should fill your plugins commands here
void commandMenuInit()
{

    //--------------------------------------------//
    //-- STEP 3. CUSTOMIZE YOUR PLUGIN COMMANDS --//
    //--------------------------------------------//
    // with function :
    // setCommand(int index,                      // zero based number to indicate the order of command
    //            TCHAR *commandName,             // the command name that you want to see in plugin menu
    //            PFUNCPLUGINCMD functionPointer, // the symbol of function (function pointer) associated with this command. The body should be defined below. See Step 4.
    //            ShortcutKey *shortcut,          // optional. Define a shortcut to trigger this command
    //            bool check0nInit                // optional. Make this menu item be checked visually
    //            );
    //setCommand(0, TEXT("Hello Notepad++"), hello, NULL, false);
    //setCommand(1, TEXT("Hello (with dialog)"), helloDlg, NULL, false);

	setCommand( 0, TEXT("Korean Charset Check"), KoreanCharsetCheck, NULL, false);
	setCommand( 1, TEXT("---"), NULL, NULL, false);
	setCommand( 2, TEXT("Ascii Check"), AsciiCheck, NULL, false);
	setCommand( 3, TEXT("EUC-KR Check"), EUCKRCheck, NULL, false);
	setCommand( 4, TEXT("CP949 Check"), CP949Check, NULL, false);
	setCommand( 5, TEXT("UTF-8 Check"), UTF8_Check, NULL, false);
	setCommand( 6, TEXT("---"), NULL, NULL, false);
	setCommand( 7, TEXT("Ascii Table"), AsciiTable, NULL, false);
	setCommand( 8, TEXT("EUC-KR Table"), EUCKRTable, NULL, false);
	setCommand( 9, TEXT("CP949 Table"), CP949Table, NULL, false);
	setCommand(10, TEXT("Unicode Table"), UnicodeTable, NULL, false);
	setCommand(11, TEXT("UTF-8 Table"), UTF8Table, NULL, false);
	setCommand(12, TEXT("---"), NULL, NULL, false);	
	setCommand(13, TEXT("About"), KCC_About, NULL, false);
}

//
// Here you can do the clean up (especially for the shortcut)
//
void commandMenuCleanUp()
{
	// Don't forget to deallocate your shortcut here
}


//
// This function help you to initialize your plugin commands
//
bool setCommand(size_t index, TCHAR *cmdName, PFUNCPLUGINCMD pFunc, ShortcutKey *sk, bool check0nInit) 
{
    if (index >= nbFunc)
        return false;

    if (!pFunc)
        return false;

    lstrcpy(funcItem[index]._itemName, cmdName);
    funcItem[index]._pFunc = pFunc;
    funcItem[index]._init2Check = check0nInit;
    funcItem[index]._pShKey = sk;

    return true;
}

//----------------------------------------------//
//-- STEP 4. DEFINE YOUR ASSOCIATED FUNCTIONS --//
//----------------------------------------------//


void KoreanCharsetCheck()
{
	ZNpp npp(nppData) ;
	int line_count = npp.GetLineCount() ;
	std::vector<unsigned char> line_texts ;
	struct _count {
		int Ascii ;
		int EucKr ;
		int Cp949 ;
		int UTF_8 ;
		int Count ;
	} bytes, lines, line_bytes ;

#ifdef _DEBUG
	std::string comments ;
#endif

	memset(&bytes, 0x00, sizeof bytes) ;
	memset(&lines, 0x00, sizeof lines) ;
	
	unsigned char checkCharset = ZNpp::Ascii_Charset | ZNpp::EUCKR_Charset | ZNpp::CP949_Charset | ZNpp::UTF8_Charset ;
	for(int line = 0; line < line_count; ++line) {
		line_texts = npp.GetLine(line) ;

#ifdef _DEBUG
		if(line == 0) {
			log("line = %d\n", line) ;
			comments = "" ;
			for (const auto& c : line_texts)
				comments += format(" %02X", c) ;
			comments += "\n" ;
		}
		std::vector<unsigned char> charsets = ZNpp::GetKoreanCharset(line_texts, checkCharset, line == 0) ;
#else
		std::vector<unsigned char> charsets = ZNpp::GetKoreanCharset(line_texts, checkCharset) ;
#endif
		
		memset(&line_bytes, 0x00, sizeof line_bytes) ;
		line_bytes.Count = (int)line_texts.size() ;
		for (const auto& c : charsets) {
			if(c & ZNpp::Ascii_Charset)	line_bytes.Ascii += 1 ;
			if(c & ZNpp::EUCKR_Charset)	line_bytes.EucKr += 1 ;
			if(c & ZNpp::CP949_Charset)	line_bytes.Cp949 += 1 ;
			if(c & ZNpp::UTF8_Charset)	line_bytes.UTF_8 += 1 ;
#ifdef _DEBUG
			if(line == 0) {
				comments += format(" %02X", c) ;
			}
#endif
		}
#ifdef _DEBUG
		if(line == 0) {
			log("%s\n", comments.c_str()) ;
		}
#endif
		if(line_bytes.Ascii == line_bytes.Count)	lines.Ascii += 1 ;
		if(line_bytes.EucKr == line_bytes.Count)	lines.EucKr += 1 ;
		if(line_bytes.Cp949 == line_bytes.Count)	lines.Cp949 += 1 ;
		if(line_bytes.UTF_8 == line_bytes.Count)	lines.UTF_8 += 1 ;
#ifdef _DEBUG
		else										log("is not UTF-8 Line : %d\n", line) ;
#endif
		lines.Count += 1 ;

		bytes.Count += line_bytes.Count ;
		bytes.Ascii += line_bytes.Ascii ;
		bytes.EucKr += line_bytes.EucKr ;
		bytes.Cp949 += line_bytes.Cp949 ;
		bytes.UTF_8 += line_bytes.UTF_8 ;
	}
	
	std::string message ;
	message += format("Ascii : %d / %d bytes, %d / %d lines\n", bytes.Ascii, bytes.Count, lines.Ascii, lines.Count) ;
	message += format("EucKr : %d / %d bytes, %d / %d lines\n", bytes.EucKr, bytes.Count, lines.EucKr, lines.Count) ;
	message += format("Cp949 : %d / %d bytes, %d / %d lines\n", bytes.Cp949, bytes.Count, lines.Cp949, lines.Count) ;
	message += format("UTF-8 : %d / %d bytes, %d / %d lines\n", bytes.UTF_8, bytes.Count, lines.UTF_8, lines.Count) ;
	message += "\n" ;
	if(bytes.Ascii == bytes.Count)		message += format("This Text is Ascii Charset") ;
	else if(bytes.EucKr == bytes.Count)	message += format("This Text is EUC-KR Charset") ;
	else if(bytes.Cp949 == bytes.Count)	message += format("This Text is CP949 Charset") ;
	else if(bytes.UTF_8 == bytes.Count)	message += format("This Text is UTF-8 (Korean) Charset") ;
	else                                message += format("This Text is Unknown Charset") ;
	::MessageBoxA(NULL, message.c_str(), TITLE, MB_OK) ;
}

void AsciiCheck()
{
	ZNpp npp(nppData) ;
	int line_count = npp.GetLineCount() ;
	std::vector<unsigned char> line_texts ;
	std::string comments ;

	int select_cursor = 2 ;
	int select_size = 2 ;

	int pos = 0 ;
	int Line_Ascii_Count = 0 ;
	int Line_NotAscii_Count = 0 ;
	for(int line = 0; line < line_count; ++line) {
		line_texts = npp.GetLine(line) ;
		std::vector<unsigned char> charsets = ZNpp::GetAscii(line_texts) ;

		comments = "" ;
		for (const auto& c : line_texts) {
			comments += format(" %02X", c) ;
		}
		comments += "\n" ;
		Line_Ascii_Count = 0 ;
		Line_NotAscii_Count = 0 ;

		int i = 0;
		std::vector<std::pair<int, int>> not_ascii_ranges ;
		for (const auto& c : charsets) {
			if((c & ZNpp::Ascii_Charset) && (c & 0x0F)) {
				Line_Ascii_Count++ ;
				if (Line_NotAscii_Count) {
					not_ascii_ranges.push_back(std::make_pair<int, int>(select_cursor*3+1, select_cursor*3+1 + Line_NotAscii_Count*3-1)) ;
				}
				Line_NotAscii_Count = 0;
			}
			else {
				if(Line_NotAscii_Count == 0)
					select_cursor = i ;
				Line_NotAscii_Count++ ;
			}
				
			comments += format(" %02X", c) ;
			i++ ;
		}
			
		if(line_texts.size() != (size_t)Line_Ascii_Count) {
			npp.Annotation(RGB(0xFF, 0x00, 00), RGB(0x00, 0xFF, 00), line, comments, not_ascii_ranges) ;
			npp.GotoLine(line) ;
			npp.SetSel(pos + select_cursor, pos + select_cursor+select_size) ;
		}

		pos += (int)line_texts.size() ;
	}
}

void EUCKRCheck()
{
	ZNpp npp(nppData) ;
	int line_count = npp.GetLineCount() ;
	std::vector<unsigned char> line_texts ;
	std::string comments ;

	int select_cursor = 2 ;
	int select_size = 2 ;

	int pos = 0 ;
	int Line_EUCKR_Count = 0 ;
	int Line_NotEUCKR_Count = 0 ;
	for(int line = 0; line < line_count; ++line) {
		line_texts = npp.GetLine(line) ;
		std::vector<unsigned char> charsets = ZNpp::GetEUCKR(line_texts) ;

		comments = "" ;
		for (const auto& c : line_texts)
			comments += format(" %02X", c) ;
		comments += "\n" ;

		Line_EUCKR_Count = 0 ;
		Line_NotEUCKR_Count = 0 ;
		int i = 0;
		std::vector<std::pair<int, int>> not_euckr_ranges ;
		for (const auto& c : charsets) {
			if((c & ZNpp::EUCKR_Charset) && (c & 0x0F)) {
				Line_EUCKR_Count++ ;
				if (Line_NotEUCKR_Count) {
					not_euckr_ranges.push_back(std::make_pair<int, int>(select_cursor*3+1, select_cursor*3+1 + Line_NotEUCKR_Count*3-1)) ;
				}
				Line_NotEUCKR_Count = 0;
			}
			else {
				if(Line_NotEUCKR_Count == 0)
					select_cursor = i ;
				Line_NotEUCKR_Count++ ;
			}
				
			comments += format(" %02X", c) ;
			i++ ;
		}
			
		if(line_texts.size() != (size_t)Line_EUCKR_Count) {
			npp.Annotation(RGB(0xFF, 0x00, 00), RGB(0x00, 0xFF, 00), line, comments, not_euckr_ranges) ;
			npp.GotoLine(line) ;
			npp.SetSel(pos + select_cursor, pos + select_cursor+select_size) ;
		}

		pos += (int)line_texts.size() ;
	}
}

void CP949Check()
{
	ZNpp npp(nppData) ;
	int line_count = npp.GetLineCount() ;
	std::vector<unsigned char> line_texts ;
	std::string comments ;

	int select_cursor = 2 ;
	int select_size = 2 ;

	int pos = 0 ;
	int Line_CP949_Count = 0 ;
	int Line_NotCP949_Count = 0 ;

	for(int line = 0; line < line_count; ++line) {
		line_texts = npp.GetLine(line) ;
		std::vector<unsigned char> charsets = ZNpp::GetCP949(line_texts) ;

		comments = "" ;
		for (const auto& c : line_texts) {
			comments += format(" %02X", c) ;
		}
		comments += "\n" ;
		Line_CP949_Count = 0 ;
		Line_NotCP949_Count = 0 ;
		int i = 0;
		std::vector<std::pair<int, int>> not_cp949_ranges ;

		for (const auto& c : charsets) {
			if((c & ZNpp::CP949_Charset) && (c & 0x0F)) {
				Line_CP949_Count++ ;
				if (Line_NotCP949_Count) {
					not_cp949_ranges.push_back(std::make_pair<int, int>(select_cursor*3+1, select_cursor*3+1 + Line_NotCP949_Count*3-1)) ;
				}
				Line_NotCP949_Count = 0;
			}
			else {
				if(Line_NotCP949_Count == 0)
					select_cursor = i ;
				Line_NotCP949_Count++ ;
			}

			comments += format(" %02X", c) ;
			i++ ;
		}
			
		if(line_texts.size() != (size_t)Line_CP949_Count) {
			npp.Annotation(RGB(0xFF, 0x00, 00), RGB(0x00, 0xFF, 00), line, comments, not_cp949_ranges) ;
			npp.GotoLine(line) ;
			npp.SetSel(pos + select_cursor, pos + select_cursor+select_size) ;
		}

		pos += (int)line_texts.size() ;
	}
}

void UTF8_Check()
{
	ZNpp npp(nppData) ;
	int line_count = npp.GetLineCount() ;
	std::vector<unsigned char> line_texts ;
	std::string comments ;

	int select_cursor = 2 ;
	int select_size = 2 ;

	int pos = 0 ;
	int Line_UTF8_Count = 0 ;
	int Line_NotUTF8_Count = 0 ;
	for(int line = 0; line < line_count; ++line) {
		line_texts = npp.GetLine(line) ;
		std::vector<unsigned char> charsets = ZNpp::GetUTF_8(line_texts) ;
		//log("line = %d\n", line) ;
		comments = "" ;
		for (const auto& c : line_texts) {
			comments += format(" %02X", c) ;
		}
		comments += "\n" ;
		Line_UTF8_Count = 0 ;
		Line_NotUTF8_Count = 0 ;
		int i = 0;
		std::vector<std::pair<int, int>> not_utf8_ranges ;
		for (const auto& c : charsets) {
			if((c & ZNpp::UTF8_Charset) && (c & 0x0F)) {
				Line_UTF8_Count++ ;
				if (Line_NotUTF8_Count) {
					not_utf8_ranges.push_back(std::make_pair<int, int>(select_cursor*3+1, select_cursor*3+1 + Line_NotUTF8_Count*3-1)) ;
				}
				Line_NotUTF8_Count = 0;
			}
			else {
				if(Line_NotUTF8_Count == 0)
					select_cursor = i ;
				Line_NotUTF8_Count++ ;
			}
				
			comments += format(" %02X", c) ;
			i++ ;
		}
		//log("%s\n", comments.c_str()) ;
		if(line_texts.size() != (size_t)Line_UTF8_Count) {
			npp.Annotation(RGB(0xFF, 0x00, 00), RGB(0x00, 0xFF, 00), line, comments, not_utf8_ranges) ;
			npp.GotoLine(line) ;
			npp.SetSel(pos + select_cursor, pos + select_cursor+select_size) ;
		}

		pos += (int)line_texts.size() ;
	}
}

void AsciiTable()
{
    // Open a new document
    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;

    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

    // Say hello now :
    // Scintilla control has no Unicode mode, so we use (char *) here
	std::map<unsigned int, std::string> table = ZNpp::GetAsciiTable() ;
	std::string line("---ASCII Table---\r\n") ;
	::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
	for (const auto& code : table) {
		if(code.first <= 0xFF) {
			line = format("%02X ==> %s\r\n", code.first, code.second.c_str()) ;
			::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
		}
		else if (code.first <= 0xFFFF) {
			line = format("%02X %02X ==> %s\r\n", (code.first >> 8) & 0x00FF, (code.first >> 0) & 0x00FF, code.second.c_str()) ;
			::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
		}
		else if (code.first <= 0xFFFFFF) {
			line = format("%02X %02X %02X ==> %s\r\n", (code.first >> 16) & 0x00FF, (code.first >> 8) & 0x00FF, (code.first >> 0) & 0x00FF, code.second.c_str()) ;
			::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
		}
	}
}

void EUCKRTable()
{
	ZNpp npp(nppData) ;
    npp.NewFile() ;	// Open a new document
    npp.ToAnsi() ;

	std::map<unsigned int, std::string> table = ZNpp::GetEUCKRTable() ;
	npp.AddText("---EUCKR Table---\r\n") ;
	for (const auto& code : table) {
		if(code.first <= 0xFF) {
			npp.AddText(format("%02X ==> %s\r\n", code.first, code.second.c_str())) ;
		}
		else if (code.first <= 0xFFFF) {
			npp.AddText(format("%02X %02X ==> %s\r\n", (code.first >> 8) & 0x00FF, (code.first >> 0) & 0x00FF, code.second.c_str())) ;
		}
		else if (code.first <= 0xFFFFFF) {
			npp.AddText(format("%02X %02X %02X ==> %s\r\n", (code.first >> 16) & 0x00FF, (code.first >> 8) & 0x00FF, (code.first >> 0) & 0x00FF, code.second.c_str())) ;
		}
	}
}

void CP949Table()
{
    // Open a new document
    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;

    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

	::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FORMAT_ANSI);

    // Say hello now :
    // Scintilla control has no Unicode mode, so we use (char *) here
	std::map<unsigned int, std::string> table = ZNpp::GetCP949Table() ;
	std::string line("---CP949 Table---\r\n") ;
	::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
	for (const auto& code : table) {
		if(code.first <= 0xFF) {
			line = format("%02X ==> %s\r\n", code.first, code.second.c_str()) ;
			::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
		}
		else if (code.first <= 0xFFFF) {
			line = format("%02X %02X ==> %s\r\n", (code.first >> 8) & 0x00FF, (code.first >> 0) & 0x00FF, code.second.c_str()) ;
			::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
		}
		else if (code.first <= 0xFFFFFF) {
			line = format("%02X %02X %02X ==> %s\r\n", (code.first >> 16) & 0x00FF, (code.first >> 8) & 0x00FF, (code.first >> 0) & 0x00FF, code.second.c_str()) ;
			::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
		}
	}
}

void UnicodeTable()
{
    // Open a new document
    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;

    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

	::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FORMAT_ANSI);

    // Say hello now :
    // Scintilla control has no Unicode mode, so we use (char *) here
	std::map<unsigned int, std::string> table = ZNpp::GetUnicodeTable() ;
	std::string line("---Unicode Table---\r\n") ;
	::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
	for (const auto& code : table) {
		if(code.first <= 0xFF) {
			line = format("%02X ==> %s\r\n", code.first, code.second.c_str()) ;
			::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
		}
		else if (code.first <= 0xFFFF) {
			line = format("%02X %02X ==> %s\r\n", (code.first >> 8) & 0x00FF, (code.first >> 0) & 0x00FF, code.second.c_str()) ;
			::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
		}
		else if (code.first <= 0xFFFFFF) {
			line = format("%02X %02X %02X ==> %s\r\n", (code.first >> 16) & 0x00FF, (code.first >> 8) & 0x00FF, (code.first >> 0) & 0x00FF, code.second.c_str()) ;
			::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
		}
	}
}

void UTF8Table()
{
    // Open a new document
    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;

    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

	::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FORMAT_ANSI);

    // Say hello now :
    // Scintilla control has no Unicode mode, so we use (char *) here
	std::map<unsigned int, std::string> table = ZNpp::GetUTF_8Table() ;
	std::string line("---UTF8 Table---\r\n") ;
	::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
	for (const auto& code : table) { 
		if(code.first <= 0xFF) {
			line = format("%02X ==> %s\r\n", code.first, code.second.c_str()) ;
			::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
		}
		else if (code.first <= 0xFFFF) {
			line = format("%02X %02X ==> %s\r\n", (code.first >> 8) & 0x00FF, (code.first >> 0) & 0x00FF, code.second.c_str()) ;
			::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
		}
		else if (code.first <= 0xFFFFFF) {
			line = format("%02X %02X %02X ==> %s\r\n", (code.first >> 16) & 0x00FF, (code.first >> 8) & 0x00FF, (code.first >> 0) & 0x00FF, code.second.c_str()) ;
			::SendMessage(curScintilla, SCI_ADDTEXT, line.length(), (LPARAM)line.data()) ;
		}
	}
}

void KCC_About()
{
	::MessageBoxA(NULL, "b0a1@iripple.co.kr", TITLE, MB_OK) ;
}

#if 0
void hello()
{
    // Open a new document
    ::SendMessage(nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);

    // Get the current scintilla
    int which = -1;
    ::SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&which);
    if (which == -1)
        return;
    HWND curScintilla = (which == 0)?nppData._scintillaMainHandle:nppData._scintillaSecondHandle;

    // Say hello now :
    // Scintilla control has no Unicode mode, so we use (char *) here
    ::SendMessage(curScintilla, SCI_SETTEXT, 0, (LPARAM)"Hello, Notepad++!");
}

void helloDlg()
{
    ::MessageBox(NULL, TEXT("Hello, Notepad++!"), TEXT("Notepad++ Plugin Template"), MB_OK);
}
#endif