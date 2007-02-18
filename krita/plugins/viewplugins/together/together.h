/*
 *  Copyright (c) 2007 Cyrille Berger (cberger@cberger.net)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _TOGETHER_H_
#define _TOGETHER_H_

#include <kparts/plugin.h>

class KisView2;
class KisEventsRecorder;

/**
 * Template of view plugin
 */
class TogetherPlugin : public KParts::Plugin
{
    Q_OBJECT
public:
    TogetherPlugin(QObject *parent, const QStringList &);
    virtual ~TogetherPlugin();

private slots:

    void slotStart();
    void slotStop();
    void slotReplay();

private:

    KisView2 * m_view;
    KisEventsRecorder* m_recorder;
};

#endif // TogetherPlugin_H
