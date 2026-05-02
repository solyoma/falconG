#pragma once

#include <QtCore>

class Logger
{
public:
	struct LogFileRecord
	{
		QString name;			// name without path but with extension
		QString creationDate;	// read from the file
		QString DisplayName() const { return name + " (" + creationDate + ")"; }
	};
	class LogFileList : public  QList<LogFileRecord>
	{
		int _latestFileIndex = -1;	// index of the latest log file in the list
	public:
		int LatestFileIndex();	// return index of the latest log file in the list
		void Clear() { QList<LogFileRecord>::clear(); _latestFileIndex = -1; }
		QStringList Names(bool withDates) const;	// return list of log file names + creation times in the list
		QStringList Dates() const;					// return list of log file creation times in the list
	};
public:

	Logger() {}
	LogFileList Setup(QString logName, QString logfilePathName, int fileSize = (10 * 1024 * 1024));
	void SetMaxSize(int maxSize);

																	 // for a non empty otherLogName open and close file
	bool Open(bool justToGetCreationTime = false);
	bool Close();

	bool Log(QString message);
																	 // file with otherLogname must be in the same folder
	QString Name() const { return _name; }
	QString LogFilesFolder() const { return _folderName; }
	QString PathName(int n=0) const { return _folderName + _lname + (n ? QString("%1").arg( int(n), 4, (int)10, QChar('0')) : "") + _ext; }
	QString CreationTimeString() const { return _creationTime; }
	LogFileList GetLogFileList();	// get creation time list of log files in the folder '_folderName'
	QStringList GetLogFileNames() { return GetLogFileList().Names(false); }

	bool IsOpen() const { return _f.isOpen(); }
	QString JustRotated() { QString when = _creationTimeOfPreviousRotated; _creationTimeOfPreviousRotated.clear(); return when; }	// if log file was just rotated in Open() call
	QString LogCreationDateTime(QString otherLogName = QString());	 // this log file opened for read already but not for use

private:
	friend class LogFileList;

	QString _name;			// name of this logger. coud be shown in log header

	int _size = 0;
	int _maxSize = 10 * 1024 * 1024;		// max size of one log file: 10 MiByte
	QString _creationTime;	// set when log file is created 
	QString _creationTimeOfPreviousRotated;	// set when file rotated

	QString _folderName;
	QString _lname;			// base name of log file (no folder path, no extension)
	QString _ext;			// log file extension including dot (e.g. ".log")
	LogFileList _logFileList;	// list of log files in the folder '_folderName' with their creation time read from the file

	QFile _f;				// when opened
	QTextStream _ofts;		// for this stream

	bool _Rotate();			// rename old log files and open new one
							// file must be open to check its size
							// otherwise no rotation is performed
																	 // for a non empty otherLogName open and close file
																	 // file 'otherLogname' must be in the same folder as this log file
	const QString _LOG_FILE_ID = "falconG log file\n";
	const QString _CREATED_PREFIX = "Created at ";
};

extern Logger logger;		// in support.cpp
