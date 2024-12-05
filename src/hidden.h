#pragma once

#ifndef _HIDDEN_H
#define _HIDDEN_H

#include "ui_hidden.h"

#include <cctype>
#include <vector>
#include <string>
#include <list>
#include <algorithm>
#include <filesystem>

#include "support.h"
#include "config.h"
#include "albums.h"


namespace fs = std::filesystem;
inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}


// lower case string
inline std::string tolower(const std::string &s)
{
	std::string _s;
	for (auto& a : s)
		_s += std::tolower((unsigned char)a);
	return _s;
}
// lower case string
inline std::wstring tolower(const std::wstring& s)
{
	std::wstring _s;
	for (auto& a : s)
		_s += std::tolower((wchar_t)a);
	return _s;
}
//----------------------------------------------

// std::enable_if_t< std::is_same_v<int, T>, int> Value(QSettings& s, QString name, T def) { 
template<class STR>
std::enable_if_t<std::is_same_v<std::wstring, STR>, std::wstring> __ToString(fs::path fp)
{
	return fp.wstring();
}
template<class STR>
std::enable_if_t<std::is_same_v<std::string, STR>, std::string> __ToString(fs::path fp)
{
	return fp.string();
}
template<class STR>
std::enable_if_t<std::is_same_v<std::wstring, STR>, std::wstring> __ToString(QString qs)
{
	return qs.toStdWString();
}
template<class STR>
std::enable_if_t<std::is_same_v<std::string, STR>, std::string> __ToString(QString qs)
{
	return qs.toStdString();
}
template<class STR>
std::enable_if_t<std::is_same_v<std::wstring, STR>, QString> __ToQString(std::wstring qs)
{
	return QString::fromWCharArray(qs.c_str());
}
template<class STR>
std::enable_if_t<std::is_same_v<std::string, STR>, QString> __ToQString(std::string qs)
{
	return QString::fromCharArray(qs.c_str());
}

template<class STR>
std::enable_if_t<std::is_same_v<std::wstring, STR>, int> __IndexOf(STR haystack, const wchar_t *needle)
{
	wchar_t* pc = wcsstr(const_cast<wchar_t*>(haystack.c_str()), needle);
	if (pc == nullptr) return -1;
	return pc - haystack.c_str();
}

template<class STR>
std::enable_if_t<std::is_same_v<std::string, STR>, int> __IndexOf(STR haystack, const char* needle)
{
	char* pc = strstr(const_cast<char*>(haystack.c_str()), needle);
	if (pc == nullptr) return -1;
	return pc - haystack.c_str();
}

/*=============================================================
 *  class FileLister
 *------------------------------------------------------------*/
using charType = wchar_t;
using stringType = std::wstring;
#ifndef TEXT
	#define TEXT(a) L#a
#endif
class FileLister : public QDialog
{
// undefined MetaObject in linking stage, no moc is run on this file
// smust set  item type in properties from c, c++ to moc
 	Q_OBJECT

	Ui::HiddenFileListerClass ui;
public:
	FileLister(QWidget* parent = Q_NULLPTR) : QDialog(parent)
	{
		ui.setupUi(this);
	}
	//~FileLister() noexcept {}
	void SetState(QString dir, QString sExt)
	{
		if (!dir.isEmpty())
			ui.edtRootFolder->setText(dir);

		if (!sExt.isEmpty())
		{
			ui.edtIncludeExtensions->setText(sExt);
			_params.sExtensions = __ToString<stringType>(sExt);
			_params.Prepare();
		}
	}
	void GetState(QString &dir, QString &sExt)
	{
		if (!ui.edtRootFolder->text().isEmpty())
			dir = ui.edtRootFolder->text();
		if (!ui.edtIncludeExtensions->text().isEmpty())
			sExt = ui.edtIncludeExtensions->text();
	}

	bool CreateList()
	{
		// collect all file names from struct file and correct
		// them using params case sensitivity members
		// into _specs
		// 
		// recursively parse root directory (either in edtRootFolder or in dsSrc)
		// and check each file against _specs. If found add path name of it to opaths
		// 
		// save the list in file "<gallery name>-originals.txt"

		std::string root = ui.edtRootFolder->text().toStdString();
		if (root.empty())
			root = config.dsSrc.ToString().toStdString();
		bool res1=false, res2=false, res3=false;
		if (((res1=_CollectGalleryList())) && (res2=_CollectFileNamesFrom(root)))
			res3=_SaveList(); 
		return res1 && res2 && res3;
	}

private:
	template <class CH, class STR>
	struct StringList : public std::vector<STR>
	{
		char separator = '|';
		StringList() {}
		StringList(const STR &list, bool caseSens, CH separator):separator(separator)
		{
			SetFrom(list, caseSens);
		}

		void SetFrom(STR list, bool caseSens, bool dropEmpty=true)
		{
			clear(); 
			for (size_t pos0=0,pos=1; pos < list.length(); ++pos)
			{
				if (list.at(pos) == separator)
				{
					if (list.at(pos0) == separator)
						++pos0;
					if (pos == pos0)
					{
						if (!dropEmpty)
							push_back("");
						pos0 = pos;
					}
					else
					{
						String s = list.substr(pos0, pos - pos0);
						trim(s);

						if (!caseSens)
							s = tolower(s);
						push_back(s);

						pos0 = pos + 1;
					}
				}
			}
		}
	};
	template<class CH, class String>struct Params
	{
		bool caseSens		= true,
			 caseSenseExt	=false;
		String sExtensions,	// store in original case
				sExts;		// but this may be converted to LC
		bool bAllFiles = false;
		// .*: any extensions, !.ext not this extension do not use with .*!
		Params() {}
		Params(const String &sext, bool cs, bool cse):
			caseSens(cs),caseSenseExt(cse), sExtensions(sext)
		{
			Prepare();
		}
		void Prepare()
		{
			if (caseSenseExt)
				sExts = tolower(sExtensions);
			else
				sExts = sExtensions;
			bAllFiles = __IndexOf<String>(sExts, TEXT(".*")) >=0;
		}
		int CheckExt(String sext) const // -1: exclude, +1 include, 0: not in list
		{
			constexpr CH NOT = TEXT("!")[0];
			int pch = __IndexOf<String>(sExts, sext.c_str());
			if(pch < 0)
			{
				if (sExts.empty())
					return 0;
				if (bAllFiles)
					return 1;
				return -1;
			}
			//else if (it > sExts.begin() && *(--it) == '!')
			else if(pch>0 && sExts[pch-1] == NOT)
				return -1;
			return 1;

		}
		int CheckExt(fs::path fp) const // -1: exclude, +1 include, 0: not in list
		{
			return CheckExt(__ToString<String>(fp.extension()) );
		}
	};

	template<class Char, class String> 
	struct Spec
	{
		String pathName;
		fs::path p, n, e;	// root path, name, extension in gallery
		std::list<fs::path> opaths;		// original full path names for this file
		Spec() {}
		Spec(const Spec& o) { *this = o; }
		//Spec &operator=(const Spec& o)
		//{
		//	pathName = o.pathName;
		//	return *this;
		//}
		Spec(const String filepath, const Params<Char,String> &params):pathName(filepath)
		{
			Prepare(params);
		}

		bool Store(fs::path ph)
		{
			if (std::find(opaths.begin(), opaths.end(), ph) != opaths.end())
				return false;
			opaths.push_back(ph);
			return true;
		}

		bool SameAs(const Spec &o, const Params<Char, String>&params)
		{
			if (n != o.n)
				return false;

			if (params.sExtensions.empty())	// no included extension: use complete name
				return n == o.n && e == o.e;
			int res = params.CheckExt(e.string());
			if (res < 0)
				return false;

			return true;
		}

		void Prepare(const Params<Char, String>&params)
		{
			p = pathName;
			_separate(p, n, e, params);
		}

		fs::path Path() const 
		{ 
			fs::path tmp(p); 
			tmp /= n;
			tmp +=e;
			return tmp;
		}

		bool operator==(const Spec& o)
		{
			return n == o.n;	 // only check base name
		}
	private:
		void _separate(fs::path &fp, fs::path& fn, fs::path& fe, const Params<Char, String>&params)
		{
			fn = fp.stem();
			fe = fp.extension();
			fp = fp.parent_path();	// fp =="/var/tmp/example.txt" => "/var/tmp", "/" => "/", "/var/tmp/." => "/var/tmp"

			if (!params.caseSens)
			{
				fn = tolower(__ToString<String>(fn));
				fp = tolower(__ToString<String>(fp));

			}
			if (!params.caseSenseExt)
				fe = tolower(__ToString<String>(fe));
		}
	};

	template <class C, class S> struct SpecList : public std::vector<Spec<C,S>>
	{
		int IndexOf(Spec<C,S> spec, const Params<C,S> &params)
		{
			std::vector<Spec<C,S>>::iterator it;
			for (it = begin(); it != end(); ++it)
			{
				if (spec == *it)
					return it - begin();
			}
			return -1;
		}
	};

	Params    <charType,stringType> _params;
	SpecList  <charType,stringType> _specs;
	StringList<charType,stringType> _foundList;

	size_t _CollectGalleryList()		// into _specs
	{
		for (auto& o : albumgen.Images())
		{
			QString s = o.Path() + o.name;
			Spec<charType, stringType> tmp(__ToString<stringType>(s), _params);
			if (std::find(_specs.begin(), _specs.end(), tmp) == _specs.end())	// not yet in vector
				_specs.push_back(tmp);
		}
		for (auto& o : albumgen.Videos())
		{
			QString s = o.Path() + o.name;
			Spec<charType, stringType> tmp(__ToString<stringType>(s), _params);
			if (std::find(_specs.begin(), _specs.end(),tmp) == _specs.end())	// not yet in vector
				_specs.push_back(tmp);
		}
		return _specs.size();
	}

	size_t _CollectFileNamesFrom(const std::string rootDir)	// recursively parse all folder to see if a file is present there
	{														// when no allowed file extensions set complete path name equality required
															// if '*' is set every non-excluded extension is ok, 
		std::string s = ui.edtRootFolder->text().toLocal8Bit().toStdString();
		fs::path sroot(s); 
		int cnt = 0;
		if (fs::exists(sroot) && fs::is_directory(sroot))		 // if a list of extensions is given exclusion is not checked
		{
			for (const auto& f : fs::recursive_directory_iterator(sroot, fs::directory_options::skip_permission_denied)) // symlinks not followed
			{
				if (fs::is_regular_file(f))
				{
					fs::path fp = f.path();
					if (_params.CheckExt(fp) >= 0)
					{							  
						Spec tmp(fp.wstring(), _params);
						int ix = _specs.IndexOf(tmp, _params);
						if (ix >= 0 && _specs[ix].Store(fp))	// ix< 0: file which has no pair in source directory
								++cnt;
					}
				}
			}
		}
		else
			QMessageBox::warning(this, tr("FalconG - Warning"), tr("Root directory \n'%1'\n doesn't exist or unreadable").arg(s.c_str()) );
		return cnt;
	}
	bool _SaveList()
	{				 // create 2 files:
					 // for files present in both folders
					 // for files not present in original folder
		QString sFiles = PROGRAM_CONFIG::NameForConfig(true, ".files.txt"),
			    sNoFiles = PROGRAM_CONFIG::NameForConfig(true, ".orphans.txt");
		QFile f(sFiles), fm(sNoFiles);
		f.open(QIODevice::WriteOnly);
		fm.open(QIODevice::WriteOnly);
		QTextStream ofs(&f), ofsm(&fm);
		ofs << "FalconG - List of files in gallery source whose originals were found in "
			<< ui.edtRootFolder->text() << "\n\n";
		ofsm << "FalconG - List of files in gallery source whose originals were NOT found in "
			<< ui.edtRootFolder->text() << "\n\n";
		int cnt1 = 0, cnt2 = 0, cnt3 = 0;
		for (auto& specs : _specs)
		{
			if (specs.opaths.empty())
			{
				ofsm << __ToQString<stringType>(specs.pathName) << "\n";
				++cnt1;
			}
			else
			{
				ofs << __ToQString<stringType>(specs.pathName) << "\n";
				++cnt2;
				for (auto& a : specs.opaths)
				{
					ofs << " -> " << __ToQString<stringType>(__ToString<stringType>(a)) << "\n";
					++cnt3;
				}
			}
		}
		ofs << "\n\n-----Count of files in a) gallery: " << cnt2 << ", b) source: " << cnt3 << "\n";
		ofsm << "\n\n-----Count of orphan files in gallery: " << cnt1 << "\n";
		f.close();
		fm.close();
		
		return true;
	}
private slots:
	void on_btnSelectSourceFolder_clicked()
	{
		QString sDir = QFileDialog::getExistingDirectory(this, tr("FalconG - Open Folder"));
		if (!sDir.isEmpty())
		{
			ui.edtRootFolder->setText(sDir);
		}
	}

	void on_chkCaseSensNames_toggled(bool b) 
	{
		_params.caseSens = b;
	}

	void on_chkCaseSensExtnsn_toggled(bool b)
	{
		_params.caseSenseExt = b;
		_params.Prepare();
	}
	void on_edtIncludeExtensions_textChanged()
	{
		QString qs = ui.edtIncludeExtensions->text();
		_params.sExtensions = __ToString<stringType>(qs);
		_params.Prepare();
	}
};
	  // _HIDDEN_H
#endif 