/*
 *  kis_tool_select_elliptical.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt (boud@valdyas.org)
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


#include <qpainter.h>
#include <qpen.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_autobrush_resource.h"
#include "kis_canvas_controller.h"
#include "kis_canvas_subject.h"
#include "kis_cursor.h"
#include "kis_image.h"
#include "kis_painter.h"
#include "kis_tool_select_elliptical.h"
#include "kis_undo_adapter.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_selection.h"
#include "kis_selection_options.h"
#include "kis_selected_transaction.h"

KisToolSelectElliptical::KisToolSelectElliptical()
{
	setName("tool_select_elliptical");
	setCursor(KisCursor::selectCursor());

	m_subject = 0;
	m_selecting = false;
	m_centerPos = KisPoint(0, 0);
	m_startPos = KisPoint(0, 0);
	m_endPos = KisPoint(0, 0);
	m_optWidget = 0;
	m_selectAction = SELECTION_ADD;
}

KisToolSelectElliptical::~KisToolSelectElliptical()
{
}

void KisToolSelectElliptical::activate()
{
	super::activate();

	if (!m_optWidget)
		return;

	m_optWidget -> slotActivated();
}

void KisToolSelectElliptical::update(KisCanvasSubject *subject)
{
	m_subject = subject;
	super::update(m_subject);
}

void KisToolSelectElliptical::paint(QPainter& gc)
{
	if (m_selecting)
		paintOutline(gc, QRect());
}

void KisToolSelectElliptical::paint(QPainter& gc, const QRect& rc)
{
	if (m_selecting)
		paintOutline(gc, rc);
}

void KisToolSelectElliptical::clearSelection()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		KisImageSP img = m_subject -> currentImg();

		Q_ASSERT(controller);

// 		if (img && img -> floatingSelection().data() != 0) {
// 			img -> unsetFloatingSelection();
//                         controller -> canvas() -> update();
// 		}

		m_startPos = KisPoint(0, 0);
		m_endPos = KisPoint(0, 0);
		m_selecting = false;
	}
}

void KisToolSelectElliptical::buttonPress(KisButtonPressEvent *e)
{
	if (m_subject) {
		KisImageSP img = m_subject -> currentImg();

		if (img && img -> activeDevice() && e -> button() == LeftButton) {
			clearSelection();
			m_startPos = m_endPos = m_centerPos = e -> pos();
			m_selecting = true;
			paintOutline();
		}
	}
}

void KisToolSelectElliptical::move(KisMoveEvent *e)
{
	if (m_subject && m_selecting) {
		paintOutline();
		// move (alt) or resize ellipse
		if (e -> state() & Qt::AltButton) {
			KisPoint trans = e -> pos() - m_endPos;
			m_startPos += trans;
			m_endPos += trans;
		} else {
			KisPoint diag = e -> pos() - (e -> state() & Qt::ControlButton
					? m_centerPos : m_startPos);
			// circle?
			if (e -> state() & Qt::ShiftButton) {
				double size = QMAX(fabs(diag.x()), fabs(diag.y()));
				double w = diag.x() < 0 ? -size : size;
				double h = diag.y() < 0 ? -size : size;
				diag = KisPoint(w, h);
			}

			// resize around center point?
			if (e -> state() & Qt::ControlButton) {
				m_startPos = m_centerPos - diag;
				m_endPos = m_centerPos + diag;
			} else {
				m_endPos = m_startPos + diag;
			}
		}
		paintOutline();
		m_centerPos = KisPoint((m_startPos.x() + m_endPos.x()) / 2,
				(m_startPos.y() + m_endPos.y()) / 2);
	}
}

void KisToolSelectElliptical::buttonRelease(KisButtonReleaseEvent *e)
{
 	if (m_subject && m_selecting && e -> button() == LeftButton) {

		paintOutline();

		if (m_startPos == m_endPos) {
			clearSelection();
		} else {
			KisImageSP img = m_subject -> currentImg();

			if (!img)
				return;

			if (m_endPos.y() < 0)
				m_endPos.setY(0);

			if (m_endPos.y() > img -> height())
				m_endPos.setY(img -> height());

			if (m_endPos.x() < 0)
				m_endPos.setX(0);

			if (m_endPos.x() > img -> width())
				m_endPos.setX(img -> width());

			if (img) {
				KisLayerSP layer = img -> activeLayer();
								
				KisSelectedTransaction *t = new KisSelectedTransaction(i18n("Elliptical Selection"), layer.data());

				if(! layer -> hasSelection())
				{
					layer -> selection() -> clear();
					if(m_selectAction==SELECTION_SUBTRACT)
						layer -> selection()->invert();
					layer -> emitSelectionChanged();
				}
				QRect rc( m_startPos.floorQPoint(), m_endPos.floorQPoint());
				rc = rc.normalize();
				
				KisSelectionSP tmpSel = new KisSelection(layer.data(),"tmp sel");
				KisAutobrushCircleShape shape(rc.width(),rc.height(), 1, 1);
				Q_UINT8 value;
				for (int y = 0; y <= rc.height(); y++)
					for (int x = 0; x <= rc.width(); x++)
					{
						value = MAX_SELECTED - shape.valueAt(x,y);
						tmpSel -> setSelected( x+rc.x(), y+rc.y(), value);
					}
				switch(m_selectAction)
				{
					case SELECTION_ADD:
						layer->addSelection(tmpSel);
						break;
					case SELECTION_SUBTRACT:
						layer->subtractSelection(tmpSel);
						break;
				}
				
				m_optWidget -> ensureMaskColor();
					
				KisUndoAdapter *adapter = img -> undoAdapter();
				if (adapter)
					adapter -> addCommand(t);

				img -> notify(rc);
			}
		}
		m_selecting = false;
	}
}

void KisToolSelectElliptical::paintOutline()
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		QWidget *canvas = controller -> canvas();
		QPainter gc(canvas);
		QRect rc;

		paintOutline(gc, rc);
	}
}

void KisToolSelectElliptical::paintOutline(QPainter& gc, const QRect&)
{
	if (m_subject) {
		KisCanvasControllerInterface *controller = m_subject -> canvasController();
		RasterOp op = gc.rasterOp();
		QPen old = gc.pen();
		QPen pen(Qt::DotLine);
		QPoint start;
		QPoint end;

		Q_ASSERT(controller);
		start = controller -> windowToView(m_startPos).floorQPoint();
		end = controller -> windowToView(m_endPos).floorQPoint();

		gc.setRasterOp(Qt::NotROP);
		gc.setPen(pen);
		gc.drawEllipse(QRect(start, end));
		gc.setRasterOp(op);
		gc.setPen(old);
	}
}

void KisToolSelectElliptical::slotSetAction(int action) {
	if (action >= SELECTION_ADD && action <= SELECTION_SUBTRACT)
		m_selectAction =(enumSelectionMode)action;
}

void KisToolSelectElliptical::setup(KActionCollection *collection)
{
	m_action = static_cast<KRadioAction *>(collection -> action(name()));

	if (m_action == 0) {
		m_action = new KRadioAction(i18n("Tool &Elliptical Select"),
					    "tool_elliptical_selection" ,
					    Qt::Key_J,
					    this,
					    SLOT(activate()),
					    collection,
					    name());
		Q_CHECK_PTR(m_action);
		m_action -> setExclusiveGroup("tools");
		m_ownAction = true;
	}
}

QWidget* KisToolSelectElliptical::createOptionWidget(QWidget* parent)
{
	m_optWidget = new KisSelectionOptions(parent, m_subject);
	Q_CHECK_PTR(m_optWidget);
	m_optWidget -> setCaption(i18n("Elliptical Selection"));

	connect (m_optWidget, SIGNAL(actionChanged(int)), this, SLOT(slotSetAction(int)));

	return m_optWidget;
}

QWidget* KisToolSelectElliptical::optionWidget()
{
        return m_optWidget;
}



#include "kis_tool_select_elliptical.moc"
