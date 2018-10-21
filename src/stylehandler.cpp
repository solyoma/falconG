#include "stylehandler.h"
//#include <QKeyValueIterator>
/*============================================================================
 * TASK:	get QString pair in form: name:value;[}]
 * EXPECTS: 'value' : put value here
 *			'finished'	: output, set to false when '} is reached
 * GLOBALS:	_ss,_pos,_elements
 * RETURNS:	name of element and value of element in 'value'
 * REMARKS: - clear t_elements when first enters a group
 *			- _pos is left after the ';' if it is found, else at next character
 *			
*---------------------------------------------------------------------------*/
QString StyleHandler::_getElement(QString &qv, bool &finished)
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
* EXPECTS: global parameters: _elements is cleared before first use
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
//	if (_ss.indexOf("qlinear") > 0)
//		ch = '@';

	// find if this is a group
	while (!(ch = _NextCh()).isNull() && ch != '{')
		;
	if (ch == '{')
	{
		sg = _ss.mid(lookBack, _pos - lookBack-2).trimmed();
		// _pos points after the '{'
		_elements.clear();
	}
	else					// no group just elements
		_pos = lookBack;

	bool b = ch.isNull();
	while (!b)	//true-> finished with this elem
	{
		se = _getElement(sv,b ); // until _ss end or ';' or '}'
		if(!se.isEmpty())
			_elements[se] = sv;
	}
	return sg;
}

/*============================================================================
 * TASK:constructor
 * EXPECTS: style sheet QString (may be empty)
 * REMARKS: style sheet group: a name followed by a '{' followed by
 *				a list of StyleGroupElements and closed by a closing brace
 *			style element is a name followed by a colon then a value QString
 *				ending with semicolon.; 
 *---------------------------------------------------------------------------*/
StyleHandler::StyleHandler(const QString & ss)
{
	Set(ss);
}

/*============================================================================
* TASK:set up style sheet handler from style sheet QString
* EXPECTS: style sheet QString (may be empty)
* REMARKS: style sheet group: a name followed by a '{' followed by
*				a list of StyleGroupElements and closed by a closing brace
*			style element is a name followed by a colon then a value QString
*				ending with semicolon.;
*---------------------------------------------------------------------------*/
void StyleHandler::Set(const QString & ss)
{
	QString qs;

	_groups.clear();
	_elements.clear();

	_ss = ss;

	_pos = 0;
	while ((qs = _GetGroup()).isEmpty() == false)
		_groups[qs] = _elements;
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
		for (auto ite = _elements.constBegin(); ite != _elements.constEnd(); ++ite)
		{
			ss += "  " + ite.key() + ":" + ite.value() + ";\n";
		}
	}
	return ss;
}

bool StyleHandler::Exists(QString group, const QString key) const
{
	if (!group.isEmpty())
	{
		return _groups[group].count(key);
	}
	else
	{
		return _elements.count(key);
	}
		
	return false;
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
		if (_elements.count(key))
			return _elements[key];
	}
	return QString();
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
		_elements[key] = newValue;
}

void StyleHandler::SetItem(const QString group, const QString key, int newValue)
{
	QString s; s.setNum(newValue);
	SetItem(group, key, s);

}

void StyleHandler::RemoveItem(const QString group, const QString key)
{
	if (!group.isEmpty())
		_groups[group].remove(key);
	_elements.remove(key);

}

void StyleHandler::RemoveGroup(const QString group)
{
	if (group.isEmpty())
		return;
	if (_groups.contains(group))
		_groups.remove(group);
}
