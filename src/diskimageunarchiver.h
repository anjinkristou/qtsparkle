#ifndef DISKIMAGEUNARCHIVER_H
#define DISKIMAGEUNARCHIVER_H

#include "unarchiver.h"
#include <QString>
#include <QProcess>

class QFileInfo;

namespace qtsparkle {

class DiskImageUnarchiver : public Unarchiver
{
	Q_OBJECT

public:
	DiskImageUnarchiver(QString const &diskImage);

protected:
	virtual void _start();
	virtual void _cancel(); 
	virtual bool _isCancelable() const;

private slots:
	void processFinished(int, QProcess::ExitStatus);
	void processError(QProcess::ProcessError);
	void aboutToBeDestroyed();

private:
	void mountDMG();
	void ejectDMG();
	void copyBundleToTemp();
	QString findBundle(QString const &searchPath);
	bool isApplicationBundle(QFileInfo const &bundle);

	QString getMountPoint();

	QString const &d_diskImage;
	QString d_mountPoint;
	bool d_isMounted;
	QProcess *d_process;
};

}

#endif
