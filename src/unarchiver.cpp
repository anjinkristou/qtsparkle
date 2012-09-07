#include "unarchiver.h"
#include "diskimageunarchiver.h"

#include <QFileInfo>

namespace qtsparkle {

Unarchiver *Unarchiver::build(QString const &path)
{
	QFileInfo file(path);

	if (file.suffix().toLower() == "dmg")
		return new DiskImageUnarchiver(path);
	// else if (file.suffix().toLower() == "zip")
	// 	return new ZipUnarchiver(path);
	// else if (file.completeSuffix() == "tar.gz")
	//  return new TarballUnarchiver(path);
	else
		return 0;
}

void Unarchiver::start()
{
	_start();
}

void Unarchiver::cancel()
{
	_cancel();
}

bool Unarchiver::isCancelable() const
{
	return _isCancelable();
}

}
