#include "ZNpp.h"
#include "Windows.h"

#ifdef _DEBUG
#	include "zutil.h"
#endif

#if defined(SCINTILLA_H)
#include "menuCmdID.h"

ZNpp::ZNpp(NppData nppData)
	: _nppData(nppData), _scintilla(nullptr), _line_count(0), _encoding(0)
{
}

void ZNpp::NewFile()
{
	::SendMessage(_nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FILE_NEW);
	ClearCash() ;
}

std::string ZNpp::GetFullCurrentPath() const
{
	TCHAR path[MAX_PATH] ;
	::SendMessage(_nppData._nppHandle, NPPM_GETFULLCURRENTPATH, 0, (LPARAM)path);	// 얻기.... 다음은.. 현재창의 내용물.. Logging하기..

#ifdef UNICODE
	int encoding = (int)::SendMessage(GetCurrentScintilta(), SCI_GETCODEPAGE, 0, 0);
	char pathA[MAX_PATH];
	WideCharToMultiByte(encoding, 0, path, -1, pathA, MAX_PATH, NULL, NULL);
	return static_cast<std::string>(pathA) ;
#else
	return static_cast<std::string>(path) ;
#endif
}

void ZNpp::ClearCash() const
{
	_scintilla = nullptr ;
	_encoding = 0 ;
	_line_count = 0 ;
}

HWND ZNpp::GetCurrentScintilta() const
{
	if(_scintilla == nullptr) {
		int currentEdit;
		::SendMessage(_nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&currentEdit);
		_scintilla = (currentEdit == 0) ? _nppData._scintillaMainHandle : _nppData._scintillaSecondHandle ;
	}
	return _scintilla ;
}

int ZNpp::GetCodePage() const
{
	if (_encoding == 0) {
		_encoding = (int)::SendMessage(GetCurrentScintilta(), SCI_GETCODEPAGE, 0, 0);
	}
	return _encoding ;
}

void ZNpp::ToAnsi()
{
	::SendMessage(_nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FORMAT_ANSI);
	_encoding = 0 ;
}

void ZNpp::ToUTF8()
{
	::SendMessage(_nppData._nppHandle, NPPM_MENUCOMMAND, 0, IDM_FORMAT_UTF_8);
	_encoding = 0 ;
}

int ZNpp::GetLineCount() const
{
	if(_line_count == 0) {
		_line_count = (int)::SendMessage(GetCurrentScintilta(), SCI_GETLINECOUNT, 0, 0) ;
	}
	return _line_count ;
}

std::vector<unsigned char> ZNpp::GetLine(int line) const
{
	std::vector<unsigned char> ret ;
	if(line < 0)	return ret ;

	if(line < GetLineCount()) {
		int line_text_count = (int)::SendMessage(GetCurrentScintilta(), SCI_LINELENGTH, line, 0);
		if(line_text_count > 0) {
			ret.resize(line_text_count) ;
			::SendMessage(_scintilla, SCI_GETLINE, line, (LPARAM)&ret[0]);
		}
	}

	return ret ;
}

void ZNpp::AddText(const std::string& text)
{
	::SendMessage(GetCurrentScintilta(), SCI_ADDTEXT, text.length(), (LPARAM)text.data()) ;
	_line_count = 0 ;
}

void ZNpp::GotoLine(int line)
{
	::SendMessage(GetCurrentScintilta(), SCI_GOTOLINE, line, 0) ;
}

void ZNpp::SetSel(int anchor, int caret)
{
	::SendMessage(GetCurrentScintilta(), SCI_SETSEL, anchor, caret) ;
}

void ZNpp::Annotation(int line, const std::string& annotation)
{
	if(annotation.length() == 0)	return ;

	::SendMessage(GetCurrentScintilta(), SCI_ANNOTATIONSETVISIBLE, ANNOTATION_BOXED, 0);
	::SendMessage(GetCurrentScintilta(), SCI_ANNOTATIONSETTEXT, line, (LPARAM)annotation.c_str());
}

void ZNpp::Annotation(COLORREF fore, COLORREF back, int line, const std::string& annotation)
{
	if(annotation.length() == 0)	return ;
	Annotation(line, annotation) ;

	::SendMessage(GetCurrentScintilta(), SCI_STYLESETFORE, STYLE_LASTPREDEFINED + 1, fore);
	::SendMessage(GetCurrentScintilta(), SCI_STYLESETBACK, STYLE_LASTPREDEFINED + 1, back);

	std::vector<char> styles ;
	//styles.resize(annotation.length(), STYLE_DEFAULT) ;
	styles.resize(annotation.length(), STYLE_LASTPREDEFINED + 1) ;
	::SendMessage(GetCurrentScintilta(), SCI_ANNOTATIONSETSTYLES, line, (LPARAM)&styles[0]) ;
}

void ZNpp::Annotation(COLORREF fore, COLORREF back, int line, const std::string& annotation, const std::pair<int, int>& color_indexes)
{
	std::vector<std::pair<int, int>> indexes ;
	indexes.push_back(color_indexes) ;
	Annotation(fore, back, line, annotation, indexes) ;
}

void ZNpp::Annotation(COLORREF fore, COLORREF back, int line, const std::string& annotation, const std::vector<std::pair<int, int>>& color_indexes)
{
	if(annotation.length() == 0)	return ;
	Annotation(line, annotation) ;

	::SendMessage(GetCurrentScintilta(), SCI_STYLESETFORE, STYLE_LASTPREDEFINED + 1, fore);
	::SendMessage(GetCurrentScintilta(), SCI_STYLESETBACK, STYLE_LASTPREDEFINED + 1, back);

	std::vector<char> styles ;
	styles.resize(annotation.length(), STYLE_DEFAULT) ;

	for(const auto& indexes : color_indexes) {
		if(indexes.first >= indexes.second)		continue ;
		if(indexes.first >= (int)styles.size())	continue ;
		if(indexes.second > (int)styles.size())	continue ;

		for(int i = indexes.first; i < indexes.second; ++i)
			styles[i] = STYLE_LASTPREDEFINED + 1 ;
	}

	::SendMessage(GetCurrentScintilta(), SCI_ANNOTATIONSETSTYLES, line, (LPARAM)&styles[0]) ;
}
#endif

#ifdef _DEBUG
std::vector<unsigned char> ZNpp::GetKoreanCharset(const std::vector<unsigned char>& bytes, unsigned char& checkCharset, bool isLogging)
#else
std::vector<unsigned char> ZNpp::GetKoreanCharset(const std::vector<unsigned char>& bytes, unsigned char& checkCharset)
#endif
{
	std::vector<unsigned char> ret ;
	unsigned char first ;
	unsigned char ch ;
	wchar_t value = 0 ;
	size_t len = bytes.size() ;
	size_t utf8_index = (checkCharset & UTF8_Charset) ? 0 : len ;
	size_t han_index = 0 ;
	ret.resize(len, UnknownCharset) ;
	std::string log_str ;
	for (size_t i = 0; i < len;++i) {
		ch = bytes[i] ;

#ifdef _DEBUG
		if(isLogging)
			log("bytes[i:%zd , han_index:%zd, utf8_index:%zd] = 0x%02X\n", i, han_index, utf8_index, bytes[i]) ;
#endif

		if(!(ch & 0x80)) {
			if(isgraph(ch))			ret[i] |= ZCharSetBytes::Ascii_1Byte ;
			else if(isspace(ch))	ret[i] |= ZCharSetBytes::Ascii_1Byte ;
			else				    ret[i] |= ZCharSetBytes::Ascii_Binary ;
#ifdef _DEBUG
			if(isLogging)
				log("bytes[%zd~%d] = %02X = %c\n", i, i, bytes[i], isgraph(ch) ? ch : ' ') ;
#endif
			
			if(i == han_index)	++han_index ;
			if(i == utf8_index)	++utf8_index ;
			continue ;
		}
		
		if (i + 1 >= len) {
			break ;
		}
		
		/// UTF-8 Check
		if(utf8_index == i) {
			value = 0 ;			
			if (((ch & 0xF0) == 0xE0) && (i + 2 < len)) { // UTF-8 3Byte
				if ((bytes[i+1] & 0xC0) == 0x80 && (bytes[i+2] & 0xC0) == 0x80) {
					value = ch & 0x0F ;
					value <<= 6 ;
					value += (bytes[i+1] & 0x3F) ;
					value <<= 6 ;
					value += (bytes[i+2] & 0x3F) ;
				}
#ifndef _DEBUG
				if (0xAC00 <= value && value <= 0xD7A3) {
					ret[i+0] |= ZCharSetBytes::UTF8_Charset;
					ret[i+1] |= ZCharSetBytes::UTF8_Charset;
					ret[i+2] |= ZCharSetBytes::UTF8_Charset;
					utf8_index = i + 3 ;
					value = 0 ;
				}
#endif
			} else if(((ch & 0xE0) == 0xC0) && (i + 1 < len)) { // UTF-8 2Byte (5+6)
				if ((bytes[i+1] & 0xC0) == 0x80) {
					value = ch & 0x1F ;
					value <<= 6 ;
					value += (bytes[i+1] & 0x3F) ;
				}
			}

			if(value) {
				char str[3]  = { 0x00, };
				if (::WideCharToMultiByte(CP_ACP, 0, &value, 1, str, 3, NULL, NULL) == 2) {
					if ((str[0] & 0x80) && isgraph(str[0] & 0x7F) && (str[1] & 0x80) && isgraph(str[1] & 0x7F)) {
						ret[i+0] |= ZCharSetBytes::UTF8_Charset;
						ret[i+1] |= ZCharSetBytes::UTF8_Charset;
						utf8_index = i + 2 ;
						if(value >= 0x0800) {	// UTF-8 3Byte
							ret[i+2] |= ZCharSetBytes::UTF8_Charset;
							utf8_index = i + 3 ;
						}
					}
#ifdef _DEBUG
					else if (0xAC00 <= value && value <= 0xD7A3) {
						ret[i+0] |= ZCharSetBytes::UTF8_Charset;
						ret[i+1] |= ZCharSetBytes::UTF8_Charset;
						ret[i+2] |= ZCharSetBytes::UTF8_Charset;
						utf8_index = i + 3 ;
					}

					if(isLogging)
						log("bytes[%zd~%d] = %02X %02X %02X = %04X = %s\n", i, i+2, bytes[i], bytes[i+1], bytes[i+2], value, str) ;
#endif
				}
			}
		}

		/// CP949(EUC-KR) Check
		if(ch == 0x80 || ch == 0xC9 || ch > 0xFD) {
			han_index = i+1;
			continue ;	// 한글아님
		}
		else {
			first = ch ;
			ch = bytes[i+1] ;
			if((ch & 0x80) == 0x00 && isalpha(ch) == 0x00)	{
				han_index = i+1;
				continue ;	// 한글아님
			} else if(first > 0xC6) {		// 0xC652 > 0 || 한자영역
				if(isgraph(ch & 0x7F) && (ch & 0x80)) {
					ret[i+0] |= ZCharSetBytes::EUCKR_2Byte ;
					ret[i+1] |= ZCharSetBytes::EUCKR_1Byte ;
#ifdef _DEBUG
					if(isLogging) {
						log_str = "  " ;
						log_str[0] = bytes[i+0] ;
						log_str[1] = bytes[i+1] ;
						log("bytes[%zd~%d] = %02X %02X = %s\n", i, i+1, bytes[i], bytes[i+1], log_str.c_str()) ;
					}
#endif
					han_index = i + 2 ;
				} else {
					han_index = i+1;
				}
				continue ;
			} else {						// 특수기호 및 한글
				unsigned char str[3] = { first, ch, 0x00 } ;
				wchar_t wstr[2] = { 0x0000, 0x0000 } ;
				if(::MultiByteToWideChar(CP_ACP, 0, (LPCCH)str, 2, wstr, 2) != 1) {
					han_index = i+1;
					continue ;
				}

				if(wstr[0] == 0x3F)	{
					han_index = i+1;
					continue ;
				}

				if ((ch & 0x80) && isgraph(ch & 0x7F) && ((0xB0 <= first && first <= 0xC8) || (0xA1 <= first && first <= 0xAC))) {
					ret[i+0] |= ZCharSetBytes::EUCKR_2Byte ;	// 0xA1 ~ 0xAC, 0xB0 ~ 0xC8, 0xCA ~ 0xFD
					ret[i+1] |= ZCharSetBytes::EUCKR_1Byte ;	// 0xA1 ~ 0xFE
#ifdef _DEBUG
					if(isLogging) {
						log_str = "  " ;
						log_str[0] = bytes[i+0] ;
						log_str[1] = bytes[i+1] ;
						log("bytes[%zd~%d] = %02X %02X = %s\n", i, i+1, bytes[i], bytes[i+1], log_str.c_str()) ;
					}
#endif
					han_index = i + 2 ;
				}
				else if(((0x81 <= first && first <= 0xA0) && (isalpha(ch) || (0x81 <= ch && ch <= 0xFE))) 
				|| ((0xA1 <= first && first <= 0xC6) && (isalpha(ch) || (0x81 <= ch && ch <= 0xA0))))
				{
					ret[i+0] |= ZCharSetBytes::CP949_2Byte ;	// 0x81 ~ 0xA0									0xA1 ~ 0xC6
					ret[i+1] |= ZCharSetBytes::CP949_1Byte ;	// 0x41 ~ 0x5A, 0x61 ~ 0x7A, 0x81 ~ 0xFE        0x41 ~ 0x5A, 0x61 ~ 0x7A, 0x81 ~ 0xA0
#ifdef _DEBUG
					if(isLogging) {
						log_str = "  " ;
						log_str[0] = bytes[i+0] ;
						log_str[1] = bytes[i+1] ;
						log("bytes[%zd~%d] = %02X %02X = %s\n", i, i+1, bytes[i], bytes[i+1], log_str.c_str()) ;
					}
#endif
					han_index = i + 2 ;
				} else {
					han_index = i+1;
				}
			}
		}
	}

	return ret ;
}

std::vector<unsigned char> ZNpp::GetAscii(const std::vector<unsigned char>& bytes)
{
	std::vector<unsigned char> ret ;
	for(const auto& byte : bytes) {
		if(byte & 0x80)			ret.push_back(ZCharSetBytes::UnknownCharset) ;
		else if(isgraph(byte))	ret.push_back(ZCharSetBytes::Ascii_1Byte) ;
		else if(isspace(byte))	ret.push_back(ZCharSetBytes::Ascii_1Byte) ;
		else				    ret.push_back(ZCharSetBytes::Ascii_Binary) ;
	}
	return ret ;
}

std::vector<unsigned char> ZNpp::GetEUCKR(const std::vector<unsigned char>& bytes)
{
	std::vector<unsigned char> ret ;
	size_t len = bytes.size() ;
	unsigned char first;
	unsigned char ch ;
	for (size_t i = 0; i < len;) {
		ch = bytes[i] ;
		if (ch & 0x80) {
			if(isgraph(ch & 0x7F) == 0x00) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	i++;	continue ;
			}
			else if (ch < 0xA1) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	i++;	continue ;
			}
			else if(0xAC < ch && ch < 0xB0) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	i++;	continue ;
			}
			else if(ch == 0xC9 || ch == 0xFE) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	i++;	continue ;
			}

			if(++i == len) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	continue ;
			}

			first = ch ;
			ch = bytes[i] ;
			if((ch & 0x80) == 0x00)	{
				ret.push_back(ZCharSetBytes::UnknownCharset) ;
				continue ;
			} else if(isgraph(ch & 0x7F) == 0x00) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;
				ret.push_back(ZCharSetBytes::UnknownCharset) ;
				i++;
				continue ;
			} else if(0xB0 <= first && first <= 0xC8) {		// 순수한글
				ret.push_back(ZCharSetBytes::EUCKR_2Byte) ;
				ret.push_back(ZCharSetBytes::EUCKR_1Byte) ;
				i++ ;
			} else if(0xCA <= first && first <= 0xFD) {	// 한자영역
				ret.push_back(ZCharSetBytes::EUCKR_2Byte) ;
				ret.push_back(ZCharSetBytes::EUCKR_1Byte) ;
				i++ ;
			} else if(0xA1 <= first && first <= 0xAC) { // 특수기호
				unsigned char str[3] = { first, ch, 0x00 } ;
				wchar_t wstr[2] = { 0x0000, 0x0000 } ;
				if(::MultiByteToWideChar(CP_ACP, 0, (LPCCH)str, 2, wstr, 2) != 1) {
					ret.push_back(ZCharSetBytes::UnknownCharset) ;
					continue ;
				}

				if(wstr[0] == 0x3F)	{
					ret.push_back(ZCharSetBytes::UnknownCharset) ;
					continue ;
				}

				ret.push_back(ZCharSetBytes::EUCKR_2Byte) ;
				ret.push_back(ZCharSetBytes::EUCKR_1Byte) ;
				i++;
			}
			else {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;
				continue ;
			}
		} 
		else {
			if(isgraph(ch))			ret.push_back(ZCharSetBytes::Ascii_1Byte) ;
			else if(isspace(ch))	ret.push_back(ZCharSetBytes::Ascii_1Byte) ;
			else				    ret.push_back(ZCharSetBytes::Ascii_Binary) ;
			i++ ;
		}
	}
	return ret ;
}

std::vector<unsigned char> ZNpp::GetCP949(const std::vector<unsigned char>& bytes)
{
	std::vector<unsigned char> ret ;
	unsigned char first;
	unsigned char ch ;
	for (std::vector<unsigned char>::const_iterator ci = bytes.cbegin(); ci != bytes.cend();) {
		ch = *ci ;
		if (ch & 0x80) {
			if(ch == 0x80 || ch == 0xC9 || ch >= 0xFE) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	++ci;	continue ;
			}

			if(++ci == bytes.cend()) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	continue ;
			}

			first = ch ;
			ch = *ci ;
			if((ch & 0x80) == 0x00 && isalpha(ch) == 0x00)	{
				ret.push_back(ZCharSetBytes::UnknownCharset) ;
				continue ;
			} else if(first > 0xC6) {		// 0xC652 > 0 || 한자영역
				if(isgraph(ch & 0x7F) && (ch & 0x80)) {
					ret.push_back(ZCharSetBytes::EUCKR_2Byte) ;
					ret.push_back(ZCharSetBytes::EUCKR_1Byte) ;
					++ci;
				}
				else {
					ret.push_back(ZCharSetBytes::UnknownCharset) ;
				}
				continue ;
			} else {						// 특수기호 및 한글
				unsigned char str[3] = { first, ch, 0x00 } ;
				wchar_t wstr[2] = { 0x0000, 0x0000 } ;
				if(::MultiByteToWideChar(CP_ACP, 0, (LPCCH)str, 2, wstr, 2) != 1) {
					ret.push_back(ZCharSetBytes::UnknownCharset) ;
					continue ;
				}

				if(wstr[0] == 0x3F)	{
					ret.push_back(ZCharSetBytes::UnknownCharset) ;
					continue ;
				}

				if ((ch & 0x80) && isgraph(ch & 0x7F) && ((0xB0 <= first && first <= 0xC8) || (0xA1 <= first && first <= 0xAC))) {
					ret.push_back(ZCharSetBytes::EUCKR_2Byte) ;	// 0xA1 ~ 0xAC, 0xB0 ~ 0xC8, 0xCA ~ 0xFD
					ret.push_back(ZCharSetBytes::EUCKR_1Byte) ;	// 0xA1 ~ 0xFE
				}
				else if(((0x81 <= first && first <= 0xA0) && (isalpha(ch) || (0x81 <= ch && ch <= 0xFE))) 
				|| ((0xA1 <= first && first <= 0xC6) && (isalpha(ch) || (0x81 <= ch && ch <= 0xA0))))
				{
					ret.push_back(ZCharSetBytes::CP949_2Byte) ;	// 0x81 ~ 0xA0									0xA1 ~ 0xC6
					ret.push_back(ZCharSetBytes::CP949_1Byte) ;	// 0x41 ~ 0x5A, 0x61 ~ 0x7A, 0x81 ~ 0xFE        0x41 ~ 0x5A, 0x61 ~ 0x7A, 0x81 ~ 0xA0
				}
				++ci;
			}
		} 
		else {
			if(isgraph(ch))			ret.push_back(ZCharSetBytes::Ascii_1Byte) ;
			else if(isspace(ch))	ret.push_back(ZCharSetBytes::Ascii_1Byte) ;
			else				    ret.push_back(ZCharSetBytes::Ascii_Binary) ;
			++ci;
		}
	}
	return ret ;
}

std::vector<unsigned char> ZNpp::GetUTF_8(const std::vector<unsigned char>& bytes)
{
	std::vector<unsigned char> ret ;
	unsigned char ch ;
	wchar_t value = 0 ;
	for (std::vector<unsigned char>::const_iterator ci = bytes.cbegin(); ci != bytes.cend();) {
		ch = *ci ;
		if ((ch & 0xF0) == 0xE0) {		// UTF-8 3Byte
			if (ci + 2 >= bytes.cend()) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	++ci;	continue ;
			}
			value = (ch & 0x0F) ;

			ch = *(ci + 1) ;
			if ((ch & 0xC0) != 0x80) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	++ci;	continue ;
			}
			value <<= 6 ;
			value += (ch & 0x3F) ;	

			ch = *(ci + 2) ;
			if ((ch & 0xC0) != 0x80) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	++ci;	continue ;
			}
			value <<= 6 ;
			value += (ch & 0x3F) ;

			if (0xAC00 <= value && value <= 0xD7A3) {
				ret.push_back(ZCharSetBytes::UTF8_3Byte) ;	++ci;
				ret.push_back(ZCharSetBytes::UTF8_2Byte) ;	++ci;
				ret.push_back(ZCharSetBytes::UTF8_1Byte) ;	++ci;
			}
			else {
				char str[3] ;
				if (::WideCharToMultiByte(CP_ACP, 0, &value, 1, str, 3, NULL, NULL) != 2) {
					ret.push_back(ZCharSetBytes::UnknownCharset) ;	++ci;	continue ;	
				}

				if ((str[0] & 0x80) && isgraph(str[0] & 0x7F) && (str[0] & 0x80) && isgraph(str[0] & 0x7F)) {
					ret.push_back(ZCharSetBytes::UTF8_3Byte) ;	++ci;
					ret.push_back(ZCharSetBytes::UTF8_2Byte) ;	++ci;
					ret.push_back(ZCharSetBytes::UTF8_1Byte) ;	++ci;
				}
				else {
					++ci ;	continue ;	// Exception
				}
			}
		}
		else if((ch & 0xE0) == 0xC0) {	// UTF-8 2Byte
			if (ci + 1 == bytes.cend()) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	++ci;	continue ;
			}
			value = (ch & 0x1F) ;

			ch = *(ci + 1) ;
			if ((ch & 0xC0) != 0x80) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	++ci;	continue ;
			}
			value <<= 6 ;
			value += (ch & 0x3F) ;
			
			char str[3] ;
			if (::WideCharToMultiByte(CP_ACP, 0, &value, 1, str, 3, NULL, NULL) != 2) {
				ret.push_back(ZCharSetBytes::UnknownCharset) ;	++ci;	continue ;	
			}

			if ((str[0] & 0x80) && isgraph(str[0] & 0x7F) && (str[0] & 0x80) && isgraph(str[0] & 0x7F)) {
				ret.push_back(ZCharSetBytes::UTF8_2Byte) ;	++ci;
				ret.push_back(ZCharSetBytes::UTF8_1Byte) ;	++ci;
			}
			else {
				++ci ;	continue ;	// Exception
			}
		}
		else if(ch & 0x80) {			// Not UTF-8
			ret.push_back(ZCharSetBytes::UnknownCharset) ;	++ci;	continue ;
		} 
		else {
			if(isgraph(ch))			ret.push_back(ZCharSetBytes::Ascii_1Byte) ;
			else if(isspace(ch))	ret.push_back(ZCharSetBytes::Ascii_1Byte) ;
			else				    ret.push_back(ZCharSetBytes::Ascii_Binary) ;
			++ci;
		}
	}
	return ret ;
}

std::map<unsigned int, std::string> ZNpp::GetAsciiTable()
{
	std::map<unsigned int, std::string> ret ;
	std::string value ;
	char asc[0x21][4] = {	"NUL", "SOH", "STX", "EOT", "EOT", "ENQ", "ACK", "BEL",	// 0x00 ~ 0x07
							"BS", "TAB", "LF", "VT", "FF", "CR", "SO", "SI",		// 0x08 ~ 0x0F
							"DLE", "DC1", "DC2", "DC3", "DC4", "NAK", "SYN", "ETB", // 0x10 ~ 0x17
							"CAN", "EM", "SUB", "ESC", "FS", "GS", "RS", "US",		// 0x20 ~ 0x1F
							"SP" } ;

 	for (int i = 0; i < 0x80; ++i) {
		if(isgraph(i)) {
			ret[i] = value + (char)i ;
		}
		else if (i == 0x7F) {
			ret[i] = "DEL" ;
		}
		else {
			ret[i] = asc[i] ;
		}
	}
	return ret ;
}

std::map<unsigned int, std::string> ZNpp::GetEUCKRTable()
{
	std::map<unsigned int, std::string> ret(GetAsciiTable()) ;

	unsigned char str[3] = { 0x00, 0x00, 0x00 } ;
	wchar_t wstr[2] = { 0x0000, 0x0000 } ;
	for (unsigned char MSB = 0xA1; MSB <= 0xAC; ++MSB) {
		str[0] = MSB ;
		for (unsigned char LSB = 0xA1; LSB <= 0xFE; ++LSB)
		{
			str[1] = LSB ;
			if(::MultiByteToWideChar(CP_ACP, 0, (LPCCH)str, 2, wstr, 2) == 1)
				if(wstr[0] != 0x3F)
					ret[ MSB << 8 | LSB ] = (const char *)str ;
		}
	}

	for (unsigned char MSB = 0xB0; MSB <= 0xC8; ++MSB) {
		str[0] = MSB ;
		for (unsigned char LSB = 0xA1; LSB <= 0xFE; ++LSB)
		{
			str[1] = LSB ;
			ret[ MSB << 8 | LSB ] = (const char *)str ;
		}
	}

	for (unsigned char MSB = 0xCA; MSB <= 0xFD; ++MSB) {
		str[0] = MSB ;
		for (unsigned char LSB = 0xA1; LSB <= 0xFE; ++LSB)
		{
			str[1] = LSB ;
			ret[ MSB << 8 | LSB ] = (const char *)str ;
		}
	}

	return ret ;
}

std::map<unsigned int, std::string> ZNpp::GetCP949Table()
{
	std::map<unsigned int, std::string> ret(GetAsciiTable()) ;

	unsigned char str[3] = { 0x00, 0x00, 0x00 } ;
	wchar_t wstr[2] = { 0x0000, 0x0000 } ;
	for (unsigned char MSB = 0xA1; MSB <= 0xAC; ++MSB) {
		str[0] = MSB ;
		for (unsigned char LSB = 0xA1; LSB <= 0xFE; ++LSB) {
			str[1] = LSB ;
			if(::MultiByteToWideChar(CP_ACP, 0, (LPCCH)str, 2, wstr, 2) == 1)
				if(wstr[0] != 0x3F)
					ret[ MSB << 8 | LSB ] = (const char *)str ;
		}
	}

	// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
	//"ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎ"
	wchar_t unicode = 0xAC00 ;
	for (unsigned int cho = 0; cho < 19; ++cho) {
		// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		//"ㅏㅐㅑㅒㅓㅔㅕㅖㅗㅘㅙㅚㅛㅜㅝㅞㅟㅠㅡㅢㅣ"
		for (unsigned int jung = 0; jung < 21; ++jung) {
			// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8
			//"　ㄱㄲㄳㄴㄵㄶㄷㄹㄺㄻㄼㄽㄾㄿㅀㅁㅂㅄㅅㅆㅇㅈㅊㅋㅌㅍㅎ"
			for (unsigned int jong = 0; jong < 28; ++jong) {
				wstr[0] = unicode++ ;
				if(::WideCharToMultiByte(CP_ACP, 0, wstr, 1, (char *)str, 3, NULL, NULL) == 2)
					ret[str[0] << 8 | str[1]] = (const char *)str ;
			}
		}
	}

	str[2] = 0x00 ;
	for (unsigned char MSB = 0xCA; MSB <= 0xFD; ++MSB) {
		str[0] = MSB ;
		for (unsigned char LSB = 0xA1; LSB <= 0xFE; ++LSB) {
			str[1] = LSB ;
			ret[ MSB << 8 | LSB ] = (const char *)str ;
		}
	}

	return ret ;
}

std::map<unsigned int, std::string>	ZNpp::GetUnicodeTable()
{
	std::map<unsigned int, std::string> ret(GetAsciiTable()) ;

	unsigned char str[3] = { 0x00, 0x00, 0x00 } ;
	wchar_t wstr[2] = { 0x0000, 0x0000 } ;
	for (unsigned char MSB = 0xA1; MSB <= 0xAC; ++MSB) {
		str[0] = MSB ;
		for (unsigned char LSB = 0xA1; LSB <= 0xFE; ++LSB) {
			str[1] = LSB ;
			if(::MultiByteToWideChar(CP_ACP, 0, (LPCCH)str, 2, wstr, 2) == 1)
				if(wstr[0] != 0x3F)
					ret[wstr[0]] = (const char *)str ;
		}
	}

	// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9
	//"ㄱㄲㄴㄷㄸㄹㅁㅂㅃㅅㅆㅇㅈㅉㅊㅋㅌㅍㅎ"
	wchar_t unicode = 0xAC00 ;
	for (unsigned int cho = 0; cho < 19; ++cho) {
		// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		//"ㅏㅐㅑㅒㅓㅔㅕㅖㅗㅘㅙㅚㅛㅜㅝㅞㅟㅠㅡㅢㅣ"
		for (unsigned int jung = 0; jung < 21; ++jung) {
			// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8
			//"　ㄱㄲㄳㄴㄵㄶㄷㄹㄺㄻㄼㄽㄾㄿㅀㅁㅂㅄㅅㅆㅇㅈㅊㅋㅌㅍㅎ"
			for (unsigned int jong = 0; jong < 28; ++jong) {
				wstr[0] = unicode++ ;
				if(::WideCharToMultiByte(CP_ACP, 0, wstr, 1, (char *)str, 3, NULL, NULL) == 2)
					ret[wstr[0]] = (const char *)str ;
			}
		}
	}

	str[2] = 0x00 ;
	for (unsigned char MSB = 0xCA; MSB <= 0xFD; ++MSB) {
		str[0] = MSB ;
		for (unsigned char LSB = 0xA1; LSB <= 0xFE; ++LSB)
		{
			str[1] = LSB ;
			if(::MultiByteToWideChar(CP_ACP, 0, (LPCCH)str, 2, wstr, 2) == 1)
				if(wstr[0] != 0x3F)
					ret[wstr[0]] = (const char *)str ;
		}
	}

	return ret ;
}

std::map<unsigned int, std::string> ZNpp::GetUTF_8Table()
{
	std::map<unsigned int, std::string> tmp(ZNpp::GetUnicodeTable()) ;
	std::map<unsigned int, std::string> ret;
	unsigned int unicode = 0 ;
	unsigned int utf8 = 0 ;
	for (const auto& code : tmp) {
		if (code.first < 0x80) {
			ret[code.first] = code.second ;
		} else if(code.first < 0x800) {
			unicode = code.first ;

			utf8 = (0x80 | (unicode & 0x3F)) ;
			unicode >>= 6 ;

			utf8 = ((0xC0 | (unicode & 0x1F)) << 8) + utf8 ;
			ret[utf8] = code.second ;
		}
		else if (code.first < 0x10000) {
			unicode = code.first ;

			utf8 = (0x80 | (unicode & 0x3F)) ;
			unicode >>= 6 ;

			utf8 = ((0x80 | (unicode & 0x3F)) << 8) + utf8 ;
			unicode >>= 6 ;

			utf8 = ((0xE0 | (unicode & 0x0F)) << 16) + utf8 ;
			unicode >>= 6 ;

			ret[utf8] = code.second ;
		}
		else {
			unicode = code.first ;

			utf8 = (0x80 | (unicode & 0x3F)) ;
			unicode >>= 6 ;

			utf8 = ((0x80 | (unicode & 0x3F)) << 8) + utf8 ;
			unicode >>= 6 ;

			utf8 = ((0x80 | (unicode & 0x3F)) << 16) + utf8 ;
			unicode >>= 6 ;

			utf8 = ((0xF0 | (unicode & 0x07)) << 24) + utf8 ;
			unicode >>= 6 ;

			ret[utf8] = code.second ;
		}
	}
	return ret ;
}
