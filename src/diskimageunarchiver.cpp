#include "diskimageunarchiver.h"

#include <QDebug>
#include <QDir>
#include <QStringList>
#include <QUuid>

namespace qtsparkle {

DiskImageUnarchiver::DiskImageUnarchiver(QString const &diskImage)
:
	d_diskImage(diskImage),
	d_mountPoint(getMountPoint()),
	d_isMounted(false)
{
	connect(this, SIGNAL(destroyed()),
		SLOT(aboutToBeDestroyed()));
}

void DiskImageUnarchiver::_start()
{
	mountDMG();

	emit started();
}

void DiskImageUnarchiver::_cancel()
{
	// sorry, does nothing.
}

bool DiskImageUnarchiver::_isCancelable() const
{
	return false;
}

QString DiskImageUnarchiver::getMountPoint()
{
	return QString("/Volumes/" + QUuid::createUuid().toString().mid(1,36));
}

void DiskImageUnarchiver::mountDMG()
{
	qDebug() << "DiskImageUnarchiver: mounting" << d_diskImage << "on" << d_mountPoint;

	QStringList args;
	args << "attach" << d_diskImage
		 << "-mountpoint" << d_mountPoint
		 << "-nobrowse"
		 << "-noautoopen";

	d_process = new QProcess(this);
	d_process->start("/usr/bin/hdiutil", args);
	d_process->write("yes\n");

	connect(d_process, SIGNAL(finished(int, QProcess::ExitStatus)),
		SLOT(processFinished(int, QProcess::ExitStatus)));

	connect(d_process, SIGNAL(error(QProcess::ProcessError)),
		SLOT(processError(QProcess::ProcessError)));
}

void DiskImageUnarchiver::ejectDMG()
{
	qDebug() << "DiskImageUnarchiver: ejecting " << d_mountPoint;

	QStringList args;
	args << "detach" << d_mountPoint
		 << "-force";

	QProcess::execute("/usr/bin/hdiutil", args);

	d_isMounted = false;
}

void DiskImageUnarchiver::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	if (exitCode != 0)
	{
		emit failed(d_process->readAll());

		emit ended();

		return;
	}

	d_isMounted = true;

	QString bundlePath(findBundle(d_mountPoint));

	if (bundlePath.isEmpty())
		emit failed("Could not find the application bundle in mounted disk image");
	else
		emit finished(bundlePath);
}

void DiskImageUnarchiver::processError(QProcess::ProcessError error)
{
	emit failed(d_process->errorString());

	emit ended();
}

QString DiskImageUnarchiver::findBundle(QString const &searchPath)
{
	QDir root(searchPath);

	foreach(QFileInfo const &folder, root.entryList())
		if (isApplicationBundle(folder))
			return folder.filePath();

	return QString();
}

bool DiskImageUnarchiver::isApplicationBundle(QFileInfo const &bundle)
{
	// Todo: compare bundle identifier in plist with the one of our
	// own application bundle.
	return bundle.isBundle();
}

void DiskImageUnarchiver::aboutToBeDestroyed()
{
	if (d_isMounted)
		ejectDMG();
}

}
