#pragma once
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QMap>

using StyleRules=QMap<QString, QString>;			// (name, selector) like color: #FFF => ("color","#ff")
using StyleGroups = QMap<QString, StyleRules>;		// (selector,rules) 
	//  selectors: e.g. QPushButton, QFrame#myFrame, #otherName, * (universal selector), 
	//				.classSelector, :pseudoSelector, ::pseudoElement
	// rules are the part between braces ( {} ). 

class StyleHandler
{
	StyleGroups	_groups;
	StyleRules	_rules;

	QString _ssr;		// style sheet read
	int _pos;			// in _ssr under processing
	QChar _NextCh() 
	{ 
		if(_pos < _ssr.length() && (!_pos || _ssr[_pos-1] == ';' || _ssr[_pos-1] == '{'))	// cut spaces between elements
			while (_ssr[_pos].isSpace()) ++_pos; 

		if (_pos < _ssr.length())
			return _ssr[_pos++];
		return QChar();
	}
	QString _getRule(QString &value, bool &finished);	// get _element from _ssr, starting after an '{' for group
							// but it can occur outside of groups too
	QString _GetGroup();	// one group from _ss, starts with QString followed by '{'
							// then elements, then '}'  // no embedded groups
public:
	StyleHandler() :_pos(0) {}
	StyleHandler(const QString & ss);	// creates gruups and elements 

	bool SaveAs(QString fileName);		// when name is w.o. extension extension is set to '.qcss'
	bool Read(QString fileName);		// when name is w.o. extension '.qcss' is assumed

	void WriteToSettings(QSettings &s);
	void ReadFromSettings(QSettings &s);

	void FromCss(const QString &css);			// _ssr from CSS string -> creates groups and rules
	QString StyleSheet(bool bare=false);		// create stylesheet text using groups names and rules
			// when bare == true
			// in group names  base and name selectors are replaced by '@', 
			// except the universal selector (*)
			// Class selector, pseudo selectors and pseudo elemenets are kept
			// E.g. QPushButton#GreenOne::first-line:hover { color: red; }
			// becomes "@::first-line:hover { color: red; }"
			// removed (inherited) selectors ara marked with an empty value
			// so when e.g. a rule looks like background-color:; the corresponding rule for the
			// group must be removed before applying the stylesheet
	QStringList GetListOfGroups() const;

	bool Exists(const QString selector, const QString nameOfRule) const;	// group contains object name
	bool Exists(const QString baseSelector, const QString objectName, const QString nameOfRule) const;	

	QString GetItem(const QString group, const QString nameOfRule) const;  // return element for group with name 'key'
	QString GetItem(const QString baseSelector, const QString objectName, const QString nameOfRule) const;
	void SetItem(const QString group, const QString key, QString newValue);
	void SetItem(const QString baseSelector, const QString objectName, const QString nameOfRule, QString newValue);
	void SetItem(const QString group, const QString nameOfRule, int newValue);
	void SetItem(const QString baseSelector, const QString objectName, const QString nameOfRule, int newValue);
	void RemoveItem(const QString group, const QString key);
	void RemoveItem(const QString baseSelector, const QString objectName, const QString nameOfRule);
	void RemoveGroup(const QString group);
	void RemoveGroup(const QString baseSelector, const QString objectName);
public:		// color helper
	QString ColorToStr(QString& color);
};
