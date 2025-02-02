/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "platformhelper.h"
#ifdef ANDROID

#include <QAndroidJniObject>

PlatformHelper::PlatformHelper(QObject *parent)
    : QObject(parent)
{
}

void PlatformHelper::share(const QUrl &url)
{
    QAndroidJniObject javaUrlString = QAndroidJniObject::fromString(url.toString());
    QAndroidJniObject::callStaticMethod<void>("com/rocksandpaper/syndic/NativeHelper", "sendUrl", "(Ljava/lang/String;)V", javaUrlString.object<jstring>());
}

#else
#include <QDesktopServices>

PlatformHelper::PlatformHelper(QObject *parent)
    : QObject(parent)
{
}

void PlatformHelper::share(const QUrl &url)
{
    QString encodedUrl = QUrl::toPercentEncoding(url.toString());
    QDesktopServices::openUrl(QLatin1String("mailto:?body=") + encodedUrl);
}

#endif
