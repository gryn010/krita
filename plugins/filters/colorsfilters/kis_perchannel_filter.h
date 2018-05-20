/*
 * This file is part of Krita
 *
 * Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_PERCHANNEL_FILTER_H_
#define _KIS_PERCHANNEL_FILTER_H_

#include <QPair>
#include <QList>

#include <filter/kis_color_transformation_filter.h>
#include <filter/kis_color_transformation_configuration.h>
#include <kis_config_widget.h>
#include <kis_paint_device.h>

#include "virtual_channel_info.h"

#include "kis_multichannel_filter_base.h"

class KisPerChannelFilterConfiguration
        : public KisMultiChannelFilterConfigurationBase
{
public:
    KisPerChannelFilterConfiguration(int channelCount);
    ~KisPerChannelFilterConfiguration() override;

    /*
    using KisFilterConfiguration::fromXML;
    using KisFilterConfiguration::toXML;
    using KisFilterConfiguration::fromLegacyXML;
    */

    using KisFilterConfiguration::fromXML;
    using KisFilterConfiguration::toXML;
    void fromXML(const QDomElement& e) override;
    void toXML(QDomDocument& doc, QDomElement& root) const override;
};


/**
 * This class is a filter to adjust channels independently
 */
class KisPerChannelFilter
        : public KisColorTransformationFilter
{
public:
    KisPerChannelFilter();
public:
    KisConfigWidget * createConfigurationWidget(QWidget* parent, const KisPaintDeviceSP dev) const override;
    KisFilterConfigurationSP  factoryConfiguration() const override;

    KoColorTransformation* createTransformation(const KoColorSpace* cs, const KisFilterConfigurationSP config) const override;

    bool needsTransparentPixels(const KisFilterConfigurationSP config, const KoColorSpace *cs) const override;

    static inline KoID id() {
        return KoID("perchannel", i18n("Color Adjustment"));
    }
private:
};

class KisPerChannelConfigWidget : public KisMultiChannelConfigWidgetBase
{
    Q_OBJECT

public:
    KisPerChannelConfigWidget(QWidget * parent, KisPaintDeviceSP dev, Qt::WindowFlags f = 0);
    ~KisPerChannelConfigWidget() override;

    void setConfiguration(const KisPropertiesConfigurationSP config) override;
    KisPropertiesConfigurationSP configuration() const override;

protected:
    void updateChannelRange() override;
};

#endif
