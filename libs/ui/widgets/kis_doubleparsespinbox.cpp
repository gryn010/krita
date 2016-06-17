/*
 *  Copyright (c) 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_doubleparsespinbox.h"

#include "kis_numparser.h"

#include <QLabel>
#include <QPixmap>
#include <QIcon>

KisDoubleParseSpinBox::KisDoubleParseSpinBox(QWidget *parent) :
	QDoubleSpinBox(parent),
	_isLastValid(true)
{

	_lastExprParsed = new QString("0.0");

	connect(this, SIGNAL(noMoreParsingError()),
					this, SLOT(clearErrorStyle()));

	//hack to let the clearError be called, even if the value changed method is the one from QDoubleSpinBox.
	connect(this, SIGNAL(valueChanged(double)),
					this, SLOT(clearError()));

	connect(this, SIGNAL(errorWhileParsing(QString)),
					this, SLOT(setErrorStyle()));

	_oldValue = value();

	_warningIcon = new QLabel(this);
	_warningIcon->setPixmap(QIcon(":/./16_light_warning.svg").pixmap(16, 16));
	_warningIcon->setStyleSheet("background:transparent;");
	_warningIcon->move(1, 1);
	_warningIcon->setVisible(false);

}

KisDoubleParseSpinBox::~KisDoubleParseSpinBox()
{

	//needed to avoid a segfault during destruction.
	delete _lastExprParsed;

}

double KisDoubleParseSpinBox::valueFromText(const QString & text) const
{

	*_lastExprParsed = text;

	bool ok;

	double ret = KisNumericParser::parseSimpleMathExpr(text, &ok);

	if (!ok) {
		if (_isLastValid) {
			_oldValue = value();
		}

		_isLastValid = false;
		ret = _oldValue; //in case of error set to minimum.
	} else {

		if (!_isLastValid) {
			_oldValue = ret;
		}

		_isLastValid = true;
	}

	return ret;

}
QString KisDoubleParseSpinBox::textFromValue(double val) const
{

	if (!_isLastValid) {
			emit errorWhileParsing(*_lastExprParsed);
			return *_lastExprParsed;
	}

	emit noMoreParsingError();
	return QDoubleSpinBox::textFromValue(val);

}

QValidator::State KisDoubleParseSpinBox::validate ( QString & input, int & pos ) const
{

	Q_UNUSED(input);
	Q_UNUSED(pos);

	return QValidator::Acceptable;

}

void KisDoubleParseSpinBox::stepBy(int steps)
{

	_isLastValid = true; //reset to valid state so we can use the up and down buttons.
	emit noMoreParsingError();

	QDoubleSpinBox::stepBy(steps);

}

void KisDoubleParseSpinBox::setValue(double value)
{
	if (!hasFocus()) {
		clearError();
	}
	QDoubleSpinBox::setValue(value);
}

void KisDoubleParseSpinBox::setErrorStyle()
{
	if (!_isLastValid) {
		setStyleSheet("Background: red; color: white; padding-left: 18px;");
		_warningIcon->move(-14, size().height()/2 - 16/2);
		_warningIcon->setVisible(true);
	}
}

void KisDoubleParseSpinBox::clearErrorStyle()
{
	if (_isLastValid) {
		_warningIcon->setVisible(false);
		setStyleSheet("");
	}
}
void KisDoubleParseSpinBox::clearError()
{
	_isLastValid = true;
	emit noMoreParsingError();
	_oldValue = value();
	clearErrorStyle();
}
