#include "logger.h"
#include <QMessageBox>

bool Logger::_Rotate()			// rename old log files and open new one
{						// file must be open to check its size
						// otherwise no rotation is performed
	if (!_f.isOpen() || (_f.isOpen() && (_f.size() <= _maxSize)))
		return true;

	Close();

	QDir dir(_folderName);
	QStringList sln;
	sln << _lname + "*" + _ext;	// like "mylog.log", "mylog001.log", etc
	dir.setNameFilters(sln);
	QStringList entries = dir.entryList(QDir::Files, QDir::Name);
	int lastIndex = 0;
	if (entries.size() > 1)
	{
		int n = entries.size() - 1;
		QString qs = entries[n].mid(_lname.length());
		int ix = qs.length() - _ext.length();
		lastIndex = qs.left(ix).toInt();
		for (; n > 0; --n)
			dir.rename(entries[n], _lname + QString("%1").arg(n + 1, 4, 10, QChar('0')) + _ext);
		// TODO: compress just rotated file (originally named as _lname + _ext)
	}
	dir.rename(_lname + _ext, _lname + 0001 + _ext);
	return Open();
}

/*=============================================================
 * TASK   :	get creation date and time for actual or another
 *			log file
 * PARAMS : otherLogName - if not empty name of an existing 
 *				log file
 * EXPECTS:
 * GLOBALS:
 * RETURNS:
 * REMARKS: - if no otherLogName then just returns creation date 
 *			  string for actual log file, which must be open
 *------------------------------------------------------------*/
QString Logger::_LogCreationDateTime(QString otherLogName)	 // this log file opened for read already but not for use
{																 // for a non empty otherLogName open and close file
	if (otherLogName == _lname + _ext)					// same as for the actual log file
		return _creationTime;

	QString line;												 // file with otherLogname must be in the same folder
	QFile* pf = &_f;

	if (!otherLogName.isEmpty())
		pf = new QFile(_folderName + "/" + otherLogName);
	line = pf->readLine();
	if (line == LOG_HEADER)
	{
		line = pf->readLine();
		if (!otherLogName.isEmpty())
			pf->close();

		if (line.startsWith("Created at "))
			return line.mid(11).trimmed();
	}
	return QString();
}

void Logger::Setup(QString logName, QString logfilePathName, int fileSize)
{
	_name = logName; 		   // not the file name: may be put into log file header (not yet) may be empty
	_lname = QDir::fromNativeSeparators(logfilePathName);	// full path name of log file
	QFileInfo info(_lname);
	_folderName = info.absolutePath() + "/";
	_lname = info.completeBaseName();
	_ext = QString('.') + info.suffix();
}

Logger::LogFileList Logger::GetLogFileList()	// get creation time list of log files in the same folder
{												// may be empty
	LogFileList result;
	QDir dir(_folderName);
	QStringList sln;
	sln << _lname + "*" + _ext;	// like "mylog.log", "mylog001.log", etc
	dir.setNameFilters(sln);
	QStringList slNames = dir.entryList(QDir::Files, QDir::Name);

	for (auto& n : slNames)
		result.append({n, _LogCreationDateTime(n) } );
	
	return result;
}

bool Logger::Open(bool justToGetCreationTime)
{
	if (!_f.isOpen())
	{
		QString path = _folderName + "/" + _lname;

		_f.setFileName(path + _ext);
		bool bInitted = _f.exists();
		if (bInitted && _f.open(QIODevice::ReadOnly))
		{
			_creationTime = _f.readLine();	
			if (_creationTime != _LOG_FILE_ID)
			{
				_f.close();
				QMessageBox::warning(nullptr, "falconG - Warning", QObject::tr("Bad log file format of '%1'").arg(_lname + _ext));
				return false;
			}
			_creationTime = _f.readLine().trimmed().mid(_CREATED_PREFIX.length());		// = length of "Created at "
			_f.close();
		}
		if (justToGetCreationTime)
			return true;

		if (!_f.open(QIODevice::WriteOnly | QIODevice::Append))
			return false;

		_ofts.setDevice(&_f);
		_ofts.setCodec(QTextCodec::codecForName("UTF-8"));
		if (!bInitted)
			_ofts << _LOG_FILE_ID << _CREATED_PREFIX << (_creationTime = QDateTime::currentDateTime().toString()) << Qt::endl;
	}
	return _Rotate();	  // only rotates if too large
}

bool Logger::Close()
{
	if (_f.isOpen())
	{
		_ofts.flush();
		_ofts.setDevice(nullptr);
		_f.close();
	}
	return true;
}

bool Logger::Log(QString message)
{
	if (!_f.isOpen())
		return false;
	_ofts << QDateTime::currentDateTime().toString() << ' ';
	_ofts << message << Qt::endl;
	return _Rotate();
}

void Logger::Resize(int maxSize)
{
	if (_maxSize == maxSize)
		return;
	_maxSize = maxSize;
	_Rotate();
}
