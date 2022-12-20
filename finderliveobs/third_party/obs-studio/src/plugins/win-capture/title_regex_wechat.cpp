#include "title_regex_wechat.h"

#include <regex>

using namespace std;

const char *get_ppt_file_from_title(const char *src, int64_t length)
{
	dstr wnd_title = {0};

	// need match case:
	// Office 365:1.pptx - PowerPoint
	// Office2019 ~ 2013:1.pptx - PowerPoint
	// Office2010:1.pptx - Microsoft PowerPoint
	// 兼容模式:1.pptx [兼容模式]- Microsoft PowerPoint
	std::regex ppt_re("(.*) - (Microsoft[ ]?)?PowerPoint(.*)?",
			  std::regex::icase);
	std::smatch result;
	std::string s(src, length);
	if (std::regex_match(s, result, ppt_re)) {
		dstr_copy(&wnd_title, result[1].str().c_str());
	}

	return wnd_title.array;
}

const char *get_player_file_from_title(const char *src, int64_t length)
{
	dstr wnd_title = {0};

	// need match case:
	// Office 365:PowerPoint 幻灯片放映  -  1.pptx
	// Office2019 ~ 2010:PowerPoint 幻灯片放映 - [1.pptx]
	// 兼容模式:PowerPoint 幻灯片放映 - [1.pptx [兼容模式]]
	wchar_t w_re[] = L"PowerPoint 幻灯片放映 [ ]?- [ ]?((\\[(.*)\\])|(.*))";
	dstr w_re_str = {0};
	dstr_from_wcs(&w_re_str, w_re);
	std::regex player_re(w_re_str.array);

	std::smatch result;
	std::string s(src, length);
	if (std::regex_match(s, result, player_re)) {
		if (result[3].matched) {
			dstr_copy(&wnd_title, result[3].str().c_str());
		} else if (result[4].matched) {
			dstr_copy(&wnd_title, result[4].str().c_str());
		}
	}

	dstr_free(&w_re_str);
	return wnd_title.array;
}
