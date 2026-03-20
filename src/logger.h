#pragma once

#include <QtCore>

constexpr const char LOG_HEADER[] = "falconG Log file\n";


class Logger
{
	QString _name;			// name of this logger. coud be shown in log header

	int _size = 0;
	int _maxSize = 10 * 1024 * 1024;		// max size of one log file: 10 MiByte
	QString _creationTime;	// set when file created 
	QString _creationTimeofRotated;	// set when file rotated

	QString _folderName;
	QString _lname;			// base name of log file (no folder path, no extension)
	QString _ext;			// log file extension including dot (e.g. ".log")

	QFile _f;				// when opened
	QTextStream _ofts;		// for this stream

	bool _Rotate();			// rename old log files and open new one
							// file must be open to check its size
							// otherwise no rotation is performed
	QString _LogCreationDateTime(QString otherLogName = QString());	 // this log file opened for read already but not for use
																	 // for a non empty otherLogName open and close file
																	 // file with otherLogname must be in the same folder
	const QString _LOG_FILE_ID = "falconG log file\n";
	const QString _CREATED_PREFIX = "Created at ";
public:
	struct LogFileRecord
	{
		QString name;			// name without path but with extension
		QString creationDate;	// from the file
	};
	typedef QList<LogFileRecord> LogFileList ;

	Logger() 
	{
		;
	}
	void Setup(QString logName, QString logfilePathName, int fileSize = (10 * 1024 * 1024));

	QString LogCreationDateTime() const { return _creationTime; }	 // this log file opened for read already but not for use
																	 // for a non empty otherLogName open and close file
																	 // file with otherLogname must be in the same folder
	QString Name() const { return _name; }
	QString LogFilesFolder() const { return _folderName; }
	QString PathName(int n=0) const { return _folderName + _lname + (n ? QString("%1").arg( int(n), 4, (int)10, QChar('0')) : "") + _ext; }
	LogFileList GetLogFileList();	// get creation time list of log files in the folder '_folderName'

	bool Open(bool justToGetCreationTime = false);

	bool IsOpen() const { return _f.isOpen(); }
	QString JustRotated() { QString when = _creationTimeofRotated; _creationTimeofRotated.clear(); return when; }	// if log file was just rotated in Open() call

	bool Close();

	bool Log(QString message);

	void Resize(int maxSize);

};

extern Logger logger;		// in support.cpp