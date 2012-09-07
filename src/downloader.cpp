#include "appcast.h"
#include "downloader.h"
#include "followredirects.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

namespace qtsparkle {

Downloader::Downloader(AppCastPtr appcast)
:
	d_appcast(0),
	d_network(0),
	d_reply(0)
{
	d_appcast = appcast;
}

Downloader::~Downloader()
{
	qDebug() << "Downloader was deleted";
}

void Downloader::setNetworkAccessManager(QNetworkAccessManager *network)
{
	d_network = network;
}

void Downloader::start()
{
	qDebug() << "Downloader::start";

	if (!d_network)
		d_network = new QNetworkAccessManager(this);

	QNetworkRequest request(d_appcast->download_url());
	d_reply = new FollowRedirects(d_network->get(request));

	connect(d_reply, SIGNAL(downloadProgress(qint64, qint64)),
		SLOT(replyProgress(qint64, qint64)));

	connect(d_reply, SIGNAL(error(QNetworkReply::NetworkError)),
		SLOT(replyError()));

	// todo: handle redirect limit reached.

	connect(d_reply, SIGNAL(Finished()),
		SLOT(replyFinished()));
}

void Downloader::cancel()
{
	Q_ASSERT(d_reply != 0);
	d_reply->abort();

	emit downloadCanceled();

	clean();
}

void Downloader::replyProgress(qint64 progress, qint64 maximum)
{
	emit downloadProgress((progress * 100) / maximum);
}

void Downloader::replyFinished()
{
	// Todo: use Content-Disposition header or url to get to the filename.
	// We need the filename to determine the extension, which determines
	// how to handle the extraction step.
	qDebug() << "Downloader::replyFinished";

	QString path(QDir::tempPath() + "/todo.dmg");
	QFile file(path);

	if (!file.open(QIODevice::WriteOnly))
	{
		qDebug() << "Could not open" << path << "for writing.";

		emit downloadFailed(QString("Could not open file '%1' for writing.").arg(path));
		clean();
		return;
	}

	// Todo: this does not guarantee everything is written.
	file.write(d_reply->reply()->readAll());
	file.close();

	emit downloadFinished(path);

	qDebug() << "Emitted downloadFinished(" << path << ")";

	clean();
}

void Downloader::replyError()
{
	qDebug() << "Downloader::replyError:" << d_reply->reply()->errorString();

	emit downloadFailed(d_reply->reply()->errorString());

	clean();
}

void Downloader::clean()
{
	Q_ASSERT(d_reply != 0);

	d_reply->deleteLater();
	d_reply = 0;

	qDebug() << "Downloader::clean";

	emit downloadEnded();
}

} // end of namespace qtsparkle
