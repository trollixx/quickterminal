/***************************************************************************
 *   Copyright (C) 2006 by Vladimir Kuznetsov                              *
 *   vovanec@gmail.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "application.h"
#include "mainwindow.h"
#include "preferences.h"

#include <QApplication>
#include <QSettings>

int main(int argc, char *argv[])
{
    setenv("TERM", "xterm", 1); // TODO/FIXME: why?

    QApplication::setApplicationName(QStringLiteral("qterminal"));
    QApplication::setApplicationVersion(QStringLiteral(STR_VERSION));
    QApplication::setOrganizationDomain(QStringLiteral("qterminal.org"));
    QSettings::setDefaultFormat(QSettings::IniFormat);

    QScopedPointer<QApplication> qapp(new QApplication(argc, argv));
    QScopedPointer<Application> app(new Application());

    return qapp->exec();
}
