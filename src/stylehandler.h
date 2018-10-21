#pragma once
#include <QString>
#include <QStringList>
#include <QMap>

using StyleGroupElements=QMap<QString, QString>;			// like color: #FFF
using StyleGroups = QMap<QString, StyleGroupElements>;		// one style group in braces

class StyleHandler
{
	StyleGroups _groups;
	StyleGroupElements _elements;

	QString _ss;		// style sheet read
	int _pos;			// in _ss under processing
	QChar _NextCh() 
	{ 
		if(!_pos || _ss[_pos-1] == ';' || _ss[_pos-1] == '{')	// cut spaces between elements
			while (_ss[_pos].isSpace()) ++_pos; 

		if (_pos < _ss.length())
			return _ss[_pos++];
		return QChar();
	}
	QString _getElement(QString &value, bool &finished);	// get _element from -ss, starting after an '{' for group
							// but it can occur outside of groups too
	QString _GetGroup();	// one group from _ss, starts with QString followed by '{'
						// then elements, then '}'  // no embedded groups
public:
	StyleHandler() {}
	StyleHandler(const QString & ss);						// creates gruups and elements 
	void Set(const QString & ss);
	QString StyleSheet() const;							// create stylesheet QString
	bool Exists(const QString group, const QString key) const;

	QString GetItem(const QString group, const QString key) const;  // return element for group with name 'key'
	void SetItem(const QString group, const QString key, QString newValue);
	void SetItem(const QString group, const QString key, int newValue);
	void RemoveItem(const QString group, const QString key);
	void RemoveGroup(const QString group);
};
