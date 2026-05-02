#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

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
QString StyleHandler::_GetSelector()
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
		sg = _ssr.mid(lookBack, _pos - lookBack-1).trimmed();
		// _pos points after the '{'
		_rules.clear();
	}
	else					// no group just elements
	{
		_pos = lookBack;
		ch = _NextCh();
		--_pos;	// need the first letter too
	}

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
	FromCss(ss);
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	ofs.setCodec("UTF-8");
#endif
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
	ifs.setCodec("UTF-8");
#endif
	ifs >> _ssr;
	_ssr.remove(QRegularExpression("\\s"));
	
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
	for (auto it = _selectors.begin(); it != _selectors.end(); ++it)
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
	for (auto &s : selectors)
	{
		ssr += s + "{\n";
		settings.beginGroup(s);
		keys = settings.childKeys();
		for (auto &k : keys)
		{
			ssr += settings.value(k, "").toString();
		}
		settings.endGroup();
	}
	settings.endGroup();
	if (!ssr.isEmpty())
		FromCss(ssr);
}

/*============================================================================
* TASK:set up style sheet handler from style sheet QString
* EXPECTS: style sheet QString (may be empty)
* REMARKS: style sheet group: a name followed by a '{' followed by
*				a list of StyleRules and closed by a closing brace
*			style element is a name followed by a colon then a value QString
*				ending with semicolon.
*---------------------------------------------------------------------------*/
void StyleHandler::FromCss(const QString & ss)
{
	QString qs;

	_selectors.clear();
	_rules.clear();

	_ssr = ss;

	_pos = 0;
	while ((qs = _GetSelector()).isEmpty() == false)
		_selectors[qs] = _rules;
}

QString StyleHandler::StyleSheet(bool bare)
{
	_ssr.clear();
	if (_selectors.size())
	{
		QString qs;
		for (auto it = _selectors.constBegin(); it != _selectors.constEnd(); ++it)
		{
			qs = it.key();
			if(bare)
				qs.replace(QRegularExpression("^[^:{* ]*"), "@");
			_ssr += qs + " {\n";
			auto &el = it.value();
			for (auto ite = el.constBegin(); ite != el.constEnd(); ++ite)
			{
				_ssr += "  " + ite.key() + ":" + ite.value() + ";\n";
			}
			_ssr += "}\n";
		}
	}
	else	 // no group just elements
	{
		for (auto ite = _rules.constBegin(); ite != _rules.constEnd(); ++ite)
		{
			_ssr += "  " + ite.key() + ":" + ite.value() + ";\n";
		}
	}
	return _ssr;
}

QStringList StyleHandler::GetListOfSelectors() const
{
	//QStringList qsl;
	//for (auto& g : _selectors.keys())
	//	qsl << g;
	//return qsl;
	return _selectors.keys();
}


/*========================================================
 * TASK:	Checks for existence of group and rule
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: -
 *-------------------------------------------------------*/
bool StyleHandler::Exists(QString selector, const QString key) const
{
	if (!selector.isEmpty())
	{
		return _selectors[selector].count(key);
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

QString StyleHandler::GetItem(QString selector, const QString key) const
{
	if (!selector.isEmpty())
	{
		if (_selectors.count(selector))
		{
			if (_selectors[selector].count(key))
				return _selectors[selector][key];
		}
	}
	else
	{
		if (_rules.count(key))
			return _rules[key];
	}
	return QString();
}

QString StyleHandler::GetItem(const QString baseSelector, const QString objectName, const QString nameOfRule) const
{
	return GetItem(baseSelector+objectName, nameOfRule);
}

void StyleHandler::SetItem(const QString selector, const QString key, QString newValue)
{
	newValue = newValue.trimmed();
	if (newValue.isEmpty() || (newValue[0] == '#' && newValue.length() < 4))	// empty or empty color
		return;

	if (!selector.isEmpty())
	{
		_selectors[selector][key] = newValue;
	}
	else
		_rules[key] = newValue;
}

void StyleHandler::SetItem(const QString baseSelector, const QString objectName, const QString nameOfRule, QString newValue)
{
	SetItem(baseSelector + "#" + objectName, nameOfRule, newValue);
}

void StyleHandler::SetItem(const QString selector, const QString key, int newValue)
{
	QString s; s.setNum(newValue);
	SetItem(selector, key, s);

}

void StyleHandler::SetItem(const QString baseSelector, const QString objectName, const QString nameOfRule, int newValue)
{
	SetItem(baseSelector + "#" + objectName, nameOfRule, newValue);
}

void StyleHandler::RemoveItem(const QString selector, const QString key)
{
	if (!selector.isEmpty())
		_selectors[selector].remove(key);
	if(!_rules.isEmpty())
		_rules.remove(key);

}

void StyleHandler::RemoveItem(const QString baseSelector, const QString objectName, const QString nameOfRule)
{
	RemoveItem(baseSelector + "#" + objectName, nameOfRule);
}

void StyleHandler::RemoveGroup(const QString selector)
{
	if (selector.isEmpty())
		return;
	if (_selectors.contains(selector))
		_selectors.remove(selector);
}

void StyleHandler::RemoveGroup(const QString baseSelector, const QString objectName)
{
	RemoveGroup(baseSelector + "#" + objectName);
}

QString StyleHandler::ColorToStr(QString& color)
{
	if (color.length() == 0 || color[0] == QChar('#'))
		return color;
	if (color.mid(0, 3) == "rgb")
	{
		int cma = color.indexOf('('), 
			cma1 = color.indexOf(',');
		auto c2i = [&]() -> QString
			{
				if (cma1 < 0)
					return "FF";
				QString s = color.mid(cma+1, cma1 - cma-1);
				if (s.isEmpty())
					return color;
				if (s.length() > 1 && s[1] == 'x')	  // hexadecimal string
						s = s.mid(2);
				else
						s = QString("%1").arg(s.toInt(), 2, 16, QChar('0'));

				cma = cma1;
				cma1 = color.indexOf(',', cma + 1);
				return s;
			};

		QString qs = '#' + c2i(); // can't do this by adding c2i()'s - order would be reversed
		qs += c2i();
		qs += c2i();
		qs += c2i();
		return qs;
	}
	return color;
}
