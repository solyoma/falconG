#include <QFile>
#include <QTextStream>
#include "stylehandler.h"
//#include <QKeyValueIterator>
/*============================================================================
 * TASK:	get QString pair in form: name:value;[}]
 * EXPECTS: 'value' : put value here
 *			'finished'	: output, set to false when '} is reached
 * GLOBALS:	_ssr,_pos,_rules
 * RETURNS:	name of element and value of element in 'value'
 * REMARKS: - clear t_rules when first enters a group
 *			- _pos is left after the ';' if it is found, else at next character
 *			
*---------------------------------------------------------------------------*/
QString StyleHandler::_getRule(QString &qv, bool &finished)
{
	QString qe;
	QChar ch;
	qv.clear();		// new value
	finished = false;
	while (!(ch = _NextCh()).isNull() && ch != ':'&& ch != '}')
			qe += ch;
	if (ch != '}')
	{
		while (!(ch = _NextCh()).isNull() && ch != ';' && ch != '}')
			qv += ch;
	}
	if (ch.isNull() || ch == '}')
		finished = true;
	return qe;
}
/*============================================================================
* TASK:	   get one group of QT style 'selector:value' pairs
* EXPECTS: global parameters: _rules is cleared before first use
* RETURNS: string for name of group or empty QString if no group found
* REMARKS: 	-group name may contain ':'s !
*			-elements may be filled even when no group is found
*		   -_pos is left after the closing '}'
*---------------------------------------------------------------------------*/
QString StyleHandler::_GetGroup()
{
	QChar ch;
	QString sg, se, sv;
	int lookBack = _pos;

// DEBUG
//	if (_ssr.indexOf("qlinear") > 0)
//		ch = '@';

	// find if this is a group
	while (!(ch = _NextCh()).isNull() && ch != '{')
		;
	if (ch == '{')
	{
		sg = _ssr.mid(lookBack, _pos - lookBack-2).trimmed();
		// _pos points after the '{'
		_rules.clear();
	}
	else					// no group just elements
		_pos = lookBack;

	bool b = ch.isNull();
	while (!b)	//true-> finished with this elem
	{
		se = _getRule(sv,b ); // until _ssr end or ';' or '}'
		if(!se.isEmpty())
			_rules[se] = sv;
	}
	return sg;
}

/*============================================================================
 * TASK:constructor
 * EXPECTS: style sheet QString (may be empty)
 * REMARKS: style sheet group: a name followed by a '{' followed by
 *				a list of StyleRules and closed by a closing brace
 *			style element is a name followed by a colon then a value QString
 *				ending with semicolon.; 
 *---------------------------------------------------------------------------*/
StyleHandler::StyleHandler(const QString & ss)
{
	Set(ss);
}

/*========================================================
 * TASK: write a qt stylesheet into a file
 * EXPECTS: fileName - name of file with or without extension
 * GLOBALS:
 * RETURNS: true or false
 * REMARKS: - with no extension given it sets the extension
 *				to ',qcss'
 *-------------------------------------------------------*/
bool StyleHandler::SaveAs(QString fileName)
{
	if (fileName.lastIndexOf('.') < 0)
		fileName += ".qcss";
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return false;

	QTextStream ofs(&file);
	ofs.setCodec("UTF-8");
	ofs << StyleSheet();
	return true;
}


/*========================================================
 * TASK: reads a qt stylesheet from a file
 * EXPECTS: fileName - name of file with or without extension
 * GLOBALS:
 * RETURNS: true or false
 * REMARKS: - with no extension given it sets the extension 
 *				to ',qcss'
 *-------------------------------------------------------*/
bool StyleHandler::Read(QString fileName)
{
	if (fileName.lastIndexOf('.') < 0)
		fileName += ".qcss";
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QTextStream ifs(&file);
	ifs.setCodec("UTF-8");
	QString s;
	ifs >> s;
	_ssr = s;
	return true;
}


/*========================================================
 * TASK:	writes the style sheet in open settings
 * EXPECTS: all rules are in a group no free rules
 * GLOBALS:
 * RETURNS: nohing
 * REMARKS: - free rules are lost
 *			- rules are saved inside a group named 'CSS'
 *				format: <GUI type>/<object Name>=<value>
*-------------------------------------------------------*/
void StyleHandler::WriteToSettings(QSettings & s)
{
	QString selector, name, value;
	s.beginGroup("CSS");
	for (auto it = _groups.begin(); it != _groups.end(); ++it)
	{
		selector = it.key();
		s.beginGroup(selector);
		for (auto nit = it.value().begin(); nit != it.value().end(); ++nit)
		{
			s.setValue(nit.key(), nit.value());
		}
		s.endGroup();
	}
	s.endGroup();
}


/*========================================================
 * TASK:	reads back all CSS elements from settings
 * EXPECTS: s - open setings
 * GLOBALS:
 * RETURNS:
 * REMARKS: - all CSS values are under settings group [CSS]
 *				format: <GUI type>/<object Name>=<value>
 *-------------------------------------------------------*/
void StyleHandler::ReadFromSettings(QSettings & settings)
{
	QString ssr;

	settings.beginGroup("CSS");
	QStringList selectors = settings.childGroups(),
				keys;
	for (auto s : selectors)
	{
		ssr += s + "{\n";
		settings.beginGroup(s);
		keys = settings.childKeys();
		for (auto k : keys)
		{
			ssr += settings.value(k, "").toString();
		}
		settings.endGroup();
	}
	settings.endGroup();
	if (!ssr.isEmpty())
		Set(ssr);
}

/*============================================================================
* TASK:set up style sheet handler from style sheet QString
* EXPECTS: style sheet QString (may be empty)
* REMARKS: style sheet group: a name followed by a '{' followed by
*				a list of StyleRules and closed by a closing brace
*			style element is a name followed by a colon then a value QString
*				ending with semicolon.;
*---------------------------------------------------------------------------*/
void StyleHandler::Set(const QString & ss)
{
	QString qs;

	_groups.clear();
	_rules.clear();

	_ssr = ss;

	_pos = 0;
	while ((qs = _GetGroup()).isEmpty() == false)
		_groups[qs] = _rules;
}

QString StyleHandler::StyleSheet() const
{
	QString ss;

	if (_groups.size())
	{
		for (auto it = _groups.constBegin(); it != _groups.constEnd(); ++it)
		{
			ss += it.key() + " {\n";
			auto el = it.value();
			for (auto ite = el.constBegin(); ite != el.constEnd(); ++ite)
			{
				ss += "  " + ite.key() + ":" + ite.value() + ";\n";
			}
			ss += "}\n";
		}
	}
	else	 // no group just elements
	{
		for (auto ite = _rules.constBegin(); ite != _rules.constEnd(); ++ite)
		{
			ss += "  " + ite.key() + ":" + ite.value() + ";\n";
		}
	}
	return ss;
}


/*========================================================
 * TASK:	Checks for existence of group and rule
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
bool StyleHandler::Exists(QString group, const QString key) const
{
	if (!group.isEmpty())
	{
		return _groups[group].count(key);
	}
	else
	{
		return _rules.count(key);
	}
		
	return false;
}

bool StyleHandler::Exists(const QString baseSelector, const QString objectName, const QString nameOfRule) const
{
	return Exists(baseSelector + "#" + objectName,nameOfRule);
}

QString StyleHandler::GetItem(QString group, const QString key) const
{
	if (!group.isEmpty())
	{
		if (_groups.count(group))
		{
			if (_groups[group].count(key))
				return _groups[group][key];
		}
	}
	else
	{
		if (_rules.count(key))
			return _rules[key];
	}
	return QString();
}

QString StyleHandler::GetItem(const QString baseSelector, const QString objectName, const QString nameOfRule)
{
	return GetItem(baseSelector+"#"+objectName, nameOfRule);
}

void StyleHandler::SetItem(const QString group, const QString key, QString newValue)
{
	newValue = newValue.trimmed();
	if (newValue.isEmpty() || (newValue[0] == '#' && newValue.length() < 4))	// empty or empty color
		return;

	if (!group.isEmpty())
	{
		_groups[group][key] = newValue;
	}
	else
		_rules[key] = newValue;
}

void StyleHandler::SetItem(const QString baseSelector, const QString objectName, const QString nameOfRule, QString newValue)
{
	SetItem(baseSelector + "#" + objectName, nameOfRule, newValue);
}

void StyleHandler::SetItem(const QString group, const QString key, int newValue)
{
	QString s; s.setNum(newValue);
	SetItem(group, key, s);

}

void StyleHandler::SetItem(const QString baseSelector, const QString objectName, const QString nameOfRule, int newValue)
{
	SetItem(baseSelector + "#" + objectName, nameOfRule, newValue);
}

void StyleHandler::RemoveItem(const QString group, const QString key)
{
	if (!group.isEmpty())
		_groups[group].remove(key);
	_rules.remove(key);

}

void StyleHandler::RemoveItem(const QString baseSelector, const QString objectName, const QString nameOfRule)
{
	RemoveItem(baseSelector + "#" + objectName, nameOfRule);
}

void StyleHandler::RemoveGroup(const QString group)
{
	if (group.isEmpty())
		return;
	if (_groups.contains(group))
		_groups.remove(group);
}

void StyleHandler::RemoveGroup(const QString baseSelector, const QString objectName)
{
	RemoveGroup(baseSelector + "#" + objectName);
}
