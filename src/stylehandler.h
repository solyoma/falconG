#pragma once
#include <QString>
#include <QStringList>
#include <QSettings>
#include <QMap>

using StyleRules=QMap<QString, QString>;			// (name, selector) like color: #FFF => ("color","#ff")
using StyleSelectors = QMap<QString, StyleRules>;		// (selector,rules) 
	//  selectors: e.g. QPushButton, QFrame#myFrame, #otherName, * (universal selector), 
	//				.classSelector, :pseudoSelector, ::pseudoElement
	// base selector : left of { and #, so for 'QFrame#myFrame' it is 'QFrame', and for #myFrame is empty)
	// 'object name'  is the part after the '#', so <selector> = <base selector> + # + <object name>
	// rules are the part between braces ( {} ). 

// nomenclature:
// Example CSS item:
//		QPushButton#myButton {
//			color: red;
//			border: 1px solid black;
//		}
//		then group =  base selector + # + object name = "QPushButton#myButton"
//			base _selector="QPushButton"
//			object name="myButton"
//          nameOfRule or key: "color" and "border"
//			value: "red" or "1px solid black"

class StyleHandler
{
	StyleSelectors	_selectors;	// they may have style rules
	StyleRules		_rules;		// bare rules without a selector

	QString _ssr;		// style sheet read	with spaces and line feeds stripped
	int _pos;			// in _ssr under processing
	QChar _NextCh() 
	{ 
		if(_pos < _ssr.length() && (!_pos || _ssr[_pos-1] == ';' || _ssr[_pos-1] == '{'))	// cut spaces between elements
			while (_pos < _ssr.length() && _ssr[_pos].isSpace()) 
				++_pos;

		if (_pos < _ssr.length())
			return _ssr[_pos++];
		return QChar();
	}
	QString _getRule(QString &value, bool &finished);	// get _element from _ssr, starting after an '{' for group
							// but it can occur outside of Selector too
	QString _GetSelector();	// one group from _ss, starts with QString followed by '{'
							// then elements, then '}'  // no embedded Selector
public:
	StyleHandler() :_pos(0) {}
	StyleHandler(const QString & ss);	// creates gruups and elements 

	bool SaveAs(QString fileName);		// when name is w.o. extension extension is set to '.qcss'
	bool Read(QString fileName);		// when name is w.o. extension '.qcss' is assumed

	void WriteToSettings(QSettings &s);
	void ReadFromSettings(QSettings &s);

	void FromCss(const QString &css);		// _ssr from CSS string -> creates Selector and rules
	QString StyleSheet(bool bare=false);	// create stylesheet text using Selector names and rules
											// when bare == true
											// in selector names  base and name selectors are replaced by '@', 
											// except the universal selector (*)
											// Class selector, pseudo selectors and pseudo elemenets are kept
											// E.g. QPushButton#GreenOne::first-line:hover { color: red; }
											// becomes "@::first-line:hover { color: red; }"
											// removed (inherited) selectors ara marked with an empty value
											// so when e.g. a rule looks like background-color:; the corresponding rule for the
											// group must be removed before applying the stylesheet
	StyleRules Rules() { return _rules; }
	QStringList GetListOfSelectors() const;
	QString BaseSelector(QString selector) { return selector.left(selector.indexOf('#')); }
	QString ObjectSelector(QString selector) { int n = selector.indexOf('#'); return n < 0 ? QString() : selector.mid(n + 1); }
	QString SubOrPseudoSelector(QString selector) // like 'hover', from QLabel#thisLabel:hover' or 'drop-down' in  'QComboBox::drop-down'
	{ 											  // but no property (like '[flat="false"]), class (.QLabel), descendant or child selector
		int n = selector.indexOf(':'); 
		if (n < 0)
			return QString();
		if (selector[n + 1] == ':')
			++n;
		return selector.mid(n + 1);
	}
	void SplitSelector(QString selector, QString& base, QString& object, QString& sub)
	{
		base = BaseSelector(selector);
		object = ObjectSelector(selector);
		sub = SubOrPseudoSelector(selector);
	}

	bool Exists(const QString selector, const QString nameOfRule) const;	// group contains object name
	bool Exists(const QString baseSelector, const QString objectName, const QString nameOfRule) const;	

	StyleRules RulesForSelector(QString name) const { return _selectors[name]; }
	QString GetItem(const QString selector, const QString nameOfRule) const;  // e.g. GetItem("#myControl","color"), GetItem("QSpinbox","color") 
	QString GetItem(const QString baseSelector, const QString objectName, const QString nameOfRule) const;
	void SetItem(const QString selector, const QString key, QString newValue);
	void SetItem(const QString baseSelector, const QString objectName, const QString nameOfRule, QString newValue);
	void SetItem(const QString selector, const QString nameOfRule, int newValue);
	void SetItem(const QString baseSelector, const QString objectName, const QString nameOfRule, int newValue);
	void RemoveItem(const QString selector, const QString key);
	void RemoveItem(const QString baseSelector, const QString objectName, const QString nameOfRule);
	void RemoveGroup(const QString selector);
	void RemoveGroup(const QString baseSelector, const QString objectName);
public:		// color helper
	QString ColorToStr(QString& color);
};
