/* This file is part of qtsparkle.
   Copyright (c) 2010 David Sansome <me@davidsansome.com>

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include "appcast.h"
#include "downloader.h"
#include "uicontroller.h"
#include "updatedialog.h"

#include <QCoreApplication>
#include <QIcon>
#include <QMessageBox>
#include <QProgressDialog>
#include <QtDebug>

namespace qtsparkle {

struct UiController::Private {

  void initProgressDialog();

  bool quiet_;
  QWidget* parent_widget_;

  QNetworkAccessManager* network_;
  QIcon icon_;
  QString version_;

  UpdateDialog* dialog_;
  QProgressDialog* progress_dialog_;

  AppCastPtr appcast_;
};

void UiController::Private::initProgressDialog()
{
  if (progress_dialog_)
    return;

  progress_dialog_ = new QProgressDialog(parent_widget_);
  progress_dialog_->setAutoClose(false);
  progress_dialog_->setAutoReset(false);  
}

UiController::UiController(bool quiet, QObject* parent, QWidget* parent_widget)
  : QObject(parent),
    d(new Private)
{
  d->quiet_ = quiet;
  d->parent_widget_ = parent_widget;
  d->progress_dialog_ = NULL;
}

UiController::~UiController() {
  qDebug() << "UiController was deleted";

  delete d->progress_dialog_;
}

void UiController::SetNetworkAccessManager(QNetworkAccessManager* network) {
  d->network_ = network;
}

void UiController::SetIcon(const QIcon& icon) {
  d->icon_ = icon;
}

void UiController::SetVersion(const QString& version) {
  d->version_ = version;
}

void UiController::CheckStarted() {
  if (!d->quiet_) {
    d->initProgressDialog();
    d->progress_dialog_->setRange(0, 0);
    d->progress_dialog_->setCancelButton(NULL);
    d->progress_dialog_->setWindowTitle(tr("Checking for updates"));
    d->progress_dialog_->setLabelText(tr("Checking for updates to %1, please wait...")
                                         .arg(qApp->applicationName()));
    d->progress_dialog_->show();
  }
}

void UiController::UpdateAvailable(AppCastPtr appcast) {
  if (d->progress_dialog_) {
    d->progress_dialog_->hide();
  }

  d->appcast_ = appcast;

  d->dialog_ = new UpdateDialog(d->parent_widget_);
  d->dialog_->setAttribute(Qt::WA_DeleteOnClose);
  d->dialog_->SetNetworkAccessManager(d->network_);
  d->dialog_->SetIcon(d->icon_);
  d->dialog_->SetVersion(d->version_);
  d->dialog_->ShowUpdate(appcast);

  connect(d->dialog_, SIGNAL(accepted()), SLOT(Download()));

  connect(d->dialog_, SIGNAL(rejected()), SLOT(deleteLater()));
}

void UiController::UpToDate() {
  if (d->progress_dialog_) {
    d->progress_dialog_->hide();
  }

  if (!d->quiet_) {
    QMessageBox::information(d->parent_widget_, tr("No updates available"),
                             tr("You already have the latest version of %1.")
                                .arg(qApp->applicationName()));
  }

  deleteLater();
}

void UiController::CheckFailed(const QString& reason) {
  qWarning() << "Update check failed:" << reason;

  if (d->progress_dialog_) {
    d->progress_dialog_->hide();
  }

  if (!d->quiet_) {
    QMessageBox::warning(d->parent_widget_, tr("Update check failed"), reason,
                         QMessageBox::Close, QMessageBox::NoButton);
  }

  deleteLater();
}

void UiController::Download()
{
  Downloader *downloader = new Downloader(d->appcast_);
  downloader->setNetworkAccessManager(d->network_);

  connect(downloader, SIGNAL(downloadFinished(QString const &)),
    SLOT(Extract(QString const &)));

  // Todo: handle failures.

  // Downloader is single use. Let it clean itself up after finishing/failing.
  connect(downloader, SIGNAL(downloadEnded()),
    downloader, SLOT(deleteLater()));

  // Progress dialog stuff
  d->initProgressDialog();
  d->progress_dialog_->setRange(0, 100);
  d->progress_dialog_->setValue(0);
  d->progress_dialog_->setCancelButtonText(tr("Cancel"));
  d->progress_dialog_->setWindowTitle(tr("Installing Update"));
  d->progress_dialog_->setLabelText(tr("Downloading %1 %2…")
    .arg(qApp->applicationName()).arg(d->appcast_->version()));
  d->progress_dialog_->show();

  // Connect cancel button
  connect(d->progress_dialog_, SIGNAL(canceled()),
    downloader, SLOT(cancel()));

  // Connect progress updates
  connect(downloader, SIGNAL(downloadProgress(int)),
    d->progress_dialog_, SLOT(setValue(int)));

  downloader->start();
}

void UiController::Extract(QString const &download)
{
  // Indeterminate mode
  d->progress_dialog_->setRange(0, 0);
  d->progress_dialog_->setValue(0);

  // Disable Cancel button (as that is not supported for now)
  d->progress_dialog_->setCancelButton(0);

  d->progress_dialog_->setLabelText(tr("Extracting %1…")
    .arg(download)); // Todo: only show basename.

  qDebug() << "Start extracting" << download;
}

} // namespace qtsparkle
