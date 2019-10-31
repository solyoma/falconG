#pragma once
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QMap>

using StyleRules=QMap<QString, QString>;			// (name, selector) like color: #FFF => ("color","#ff")
using StyleGroups = QMap<QString, StyleRules>;		// (selector,rules) rules are the part between braces ( {} ). 
													//			e.g.: QPushButton {...} => ("QPushButton", ...)
													// or using the object names in the selector
													//			e.g.: QFrame#myFrame {...} => ("QFrame#myFrame", ...)

class StyleHandler
{
	StyleGroups	_groups;
	StyleRules	_rules;

	QString _ssr;		// style sheet read
	int _pos;			// in _ss under processing
	QChar _NextCh() 
	{ 
		if(!_pos || _ssr[_pos-1] == ';' || _ssr[_pos-1] == '{')	// cut spaces between elements
			while (_ssr[_pos].isSpace()) ++_pos; 

		if (_pos < _ssr.length())
			return _ssr[_pos++];
		return QChar();
	}
	QString _getRule(QString &value, bool &finished);	// get _element from -ss, starting after an '{' for group
							// but it can occur outside of groups too
	QString _GetGroup();	// one group from _ss, starts with QString followed by '{'
							// then elements, then '}'  // no embedded groups
public:
	StyleHandler() {}
	StyleHandler(const QString & ss);					// creates gruups and elements 

	bool SaveAs(QString fileName);		// when name is w.o. extension extension is set to '.qcss'
	bool Read(QString fileName);		// when name is w.o. extension '.qcss' is assumed

	void WriteToSettings(QSettings &s);
	void ReadFromSettings(QSettings &s);

	void Set(const QString &css);						// _ssr from CSS string
	QString StyleSheet() const;							// create stylesheet text
	bool Exists(const QString selector, const QString nameOfRule) const;	// group contains object name
	bool Exists(const QString baseSelector, const QString objectName, const QString nameOfRule) const;	

	QString GetItem(const QString group, const QString nameOfRule) const;  // return element for group with name 'key'
	QString GetItem(const QString baseSelector, const QString objectName, const QString nameOfRule);
	void SetItem(const QString group, const QString key, QString newValue);
	void SetItem(const QString baseSelector, const QString objectName, const QString nameOfRule, QString newValue);
	void SetItem(const QString group, const QString nameOfRule, int newValue);
	void SetItem(const QString baseSelector, const QString objectName, const QString nameOfRule, int newValue);
	void RemoveItem(const QString group, const QString key);
	void RemoveItem(const QString baseSelector, const QString objectName, const QString nameOfRule);
	void RemoveGroup(const QString group);
	void RemoveGroup(const QString baseSelector, const QString objectName);
};
