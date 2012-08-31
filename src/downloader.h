#ifndef QTSPARKLE_DOWNLOADER_H
#define QTSPARKLE_DOWNLOADER_H

#include "appcast.h"

class QFile;
class QNetworkAccessManager;
class QNetworkReply;

namespace qtsparkle {

class Downloader : public QObject
{
	Q_OBJECT

public:
	Downloader(AppCastPtr appcast);
	~Downloader();
	void setNetworkAccessManager(QNetworkAccessManager *network);

public slots:
	void start();
	void cancel();

private slots:
	void replyProgress(qint64, qint64);
	void replyFinished();
	void replyError();

signals:
	void downloadProgress(int value);
	void downloadFinished(QString const &path);
	void downloadCanceled();
	void downloadFailed(QString const &reason);
	void downloadEnded();

private:
	void clean();

	AppCastPtr d_appcast;
	QNetworkAccessManager *d_network;
	QNetworkReply *d_reply;
	
};

}

#endif
