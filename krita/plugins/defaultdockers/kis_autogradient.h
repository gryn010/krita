/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2004 Sven Langkamp <longamp@reallygood.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _KIS_AUTOGRADIENT_H_ 
#define _KIS_AUTOGRADIENT_H_

#include "wdgautogradient.h"
#include "kis_gradient.h"
#include "kis_autogradient_resource.h"

class KisAutogradient : public KisWdgAutogradient
{
	Q_OBJECT

	public:
		KisAutogradient(QWidget *parent, const char* name, const QString& caption);;
	signals:
		void activatedResource(KisResource *r);
	private:
		KisAutogradientResource* m_autogradientResource;
	private slots:
		void slotSelectedSegment(KisGradientSegment* segment);
		void slotChangedSegment(KisGradientSegment* segment);
		void slotChangedInterpolation(int type);
		void slotChangedColorInterpolation(int type);
		void slotChangedLeftColor( const QColor& color);
		void slotChangedRightColor( const QColor& color);
		void slotChangedLeftOpacity( int value );
		void slotChangedRightOpacity( int value );
		void paramChanged();
};

#endif
