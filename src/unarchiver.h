#ifndef UNARCHIVER_H
#define UNARCHIVER_H

#include <QObject>
#include <QString>

namespace qtsparkle {

class Unarchiver : public QObject
{
	Q_OBJECT

public:
	virtual ~Unarchiver() {};
	bool isCancelable() const;

	static Unarchiver *build(QString const &path);

signals:
	void started();
	void finished(QString const &path);
	void failed(QString const &reason);
	void ended();

public slots:
	void start();
	void cancel();

protected:
	virtual void _start() = 0;
	virtual void _cancel() = 0;
	virtual bool _isCancelable() const = 0;
};

}

#endif
