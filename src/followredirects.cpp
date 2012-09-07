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

#include "followredirects.h"

namespace qtsparkle {

struct FollowRedirects::Private {
  Private()
    : reply_(NULL),
      redirect_count_(0)
  {
  }

  QNetworkReply* reply_;
  int redirect_count_;

  static const int kMaxRedirects = 5;
};

FollowRedirects::FollowRedirects(QNetworkReply* reply)
  : d(new Private)
{
  setNetworkReply(reply);
}

FollowRedirects::~FollowRedirects() {
  delete d->reply_;
}

QNetworkReply* FollowRedirects::reply() const {
  return d->reply_;
}

void FollowRedirects::FinishedSlot() {
  QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
  if (!reply || reply != d->reply_)
    return;

  QVariant redirect_target = reply->attribute(
        QNetworkRequest::RedirectionTargetAttribute);
  if (redirect_target.isValid()) {
    reply->deleteLater();

    if (d->redirect_count_ >= Private::kMaxRedirects) {
      d->reply_ = NULL;
      emit RedirectLimitReached();
      return;
    }

    d->redirect_count_ ++;

    QUrl target = redirect_target.toUrl();
    if (target.scheme().isEmpty() || target.host().isEmpty()) {
      QString path = target.path();
      target = reply->url();
      target.setPath(path);
    }

    QNetworkRequest req(target);

    // Copy the cache control attribute from the last request
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
        d->reply_->request().attribute(QNetworkRequest::CacheLoadControlAttribute));

    setNetworkReply(d->reply_->manager()->get(req));

    return;
  }

  emit Finished();
}

void FollowRedirects::setNetworkReply(QNetworkReply *reply)
{
  d->reply_ = reply;

  connect(d->reply_, SIGNAL(finished()),
    SLOT(FinishedSlot()));

  connect(d->reply_, SIGNAL(downloadProgress(qint64, qint64)),
    SLOT(proxyDownloadProgress(qint64, qint64)));

  connect(d->reply_, SIGNAL(error(QNetworkReply::NetworkError)),
    SLOT(proxyError(QNetworkReply::NetworkError)));
}

void FollowRedirects::proxyDownloadProgress(qint64 val, qint64 max)
{
  emit downloadProgress(val, max);
}

void FollowRedirects::proxyError(QNetworkReply::NetworkError errorCode)
{
  emit error(errorCode);
}

void FollowRedirects::abort()
{
  d->reply_->abort();
}

} // namespace qtsparkle
