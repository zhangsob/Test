#ifndef ZNPP_H
#define ZNPP_H

#include "PluginInterface.h"
#include <string>
#include <vector>
#include <map>

class ZNpp
{
public :
	enum ZCharSetBytes {
		UnknownCharset	= 0x00,

		Ascii_Charset	= 0x10,
		EUCKR_Charset	= 0x20,
		CP949_Charset	= 0x40,
		UTF8_Charset	= 0x80,

		Ascii_Binary	= 0x10,
		Ascii_1Byte		= (UTF8_Charset | CP949_Charset | EUCKR_Charset | Ascii_Charset | 0x01),
		EUCKR_2Byte		= (CP949_Charset | EUCKR_Charset | 0x02),	// 0xA1 ~ 0xAC, 0xB0 ~ 0xC8, 0xCA ~ 0xFD
		EUCKR_1Byte		= (CP949_Charset | EUCKR_Charset | 0x01),	// 0xA1 ~ 0xFE
		CP949_2Byte		= (CP949_Charset | 0x02),	// 0x81 ~ 0xC6
		CP949_1Byte		= (CP949_Charset | 0x01),	// 0x41 ~ 0x5A, 0x61 ~ 0x7A, 0x81 ~ 0xA0
		UTF8_3Byte		= (UTF8_Charset | 0x03),
		UTF8_2Byte		= (UTF8_Charset | 0x02),
		UTF8_1Byte		= (UTF8_Charset | 0x01),
	};

private :
		ZNpp();
public :
#if defined(SCINTILLA_H)
		ZNpp(NppData nppData);
		
		void						NewFile() ;
		int							GetCodePage() const ;
		void						ToAnsi() ;
		void						ToUTF8() ;
		std::string					GetFullCurrentPath() const ;
		int							GetLineCount() const ;
		std::vector<unsigned char>	GetLine(int line) const ;
		void						AddText(const std::string& text) ;
		void						GotoLine(int line) ;
		void						SetSel(int anchor, int caret) ;

		void						Annotation(int line, const std::string& annotation) ;
		void						Annotation(COLORREF fore, COLORREF back, int line, const std::string& annotation) ;
		void						Annotation(COLORREF fore, COLORREF back, int line, const std::string& annotation, const std::pair<int, int>& color_indexes) ;
		void						Annotation(COLORREF fore, COLORREF back, int line, const std::string& annotation, const std::vector<std::pair<int, int>>& color_indexes) ;
#endif

#ifdef _DEBUG
static	std::vector<unsigned char>	GetKoreanCharset(const std::vector<unsigned char>& bytes, unsigned char& checkCharset, bool isLogging) ;
#else
static	std::vector<unsigned char>	GetKoreanCharset(const std::vector<unsigned char>& bytes, unsigned char& checkCharset) ;
#endif
static	std::vector<unsigned char>	GetAscii(const std::vector<unsigned char>& bytes) ;
static	std::vector<unsigned char>	GetEUCKR(const std::vector<unsigned char>& bytes) ;
static	std::vector<unsigned char>	GetCP949(const std::vector<unsigned char>& bytes) ;
static	std::vector<unsigned char>	GetUTF_8(const std::vector<unsigned char>& bytes) ;

static	std::map<unsigned int, std::string>	GetAsciiTable() ;
static	std::map<unsigned int, std::string>	GetEUCKRTable() ;
static	std::map<unsigned int, std::string>	GetCP949Table() ;
static	std::map<unsigned int, std::string>	GetUnicodeTable() ;
static	std::map<unsigned int, std::string>	GetUTF_8Table() ;

#if defined(SCINTILLA_H)
private :
	HWND	GetCurrentScintilta() const ;
	void	ClearCash() const ;

private :
	mutable int		_encoding ;
	mutable int		_line_count ;
	mutable HWND	_scintilla ;
			NppData _nppData;
#endif
};
#endif	//#ifndef ZNPP_H