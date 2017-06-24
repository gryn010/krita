/* This file is part of the KDE project
 * Copyright (C) 2017 Scott Petrovic <scottpetrovic@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "widgets/kis_paintop_presets_save.h"
#include <QDebug>

#include <KoFileDialog.h>
#include "KisImportExportManager.h"
#include "QDesktopServices"

KisPresetSaveWidget::KisPresetSaveWidget(QWidget * parent)
    : KisPaintOpPresetSaveDialog(parent)
{
}

KisPresetSaveWidget::~KisPresetSaveWidget()
{

}

void KisPresetSaveWidget::scratchPadSetup(KisCanvasResourceProvider* resourceProvider)
{
    m_resourceProvider = resourceProvider;

    this->brushPresetThumbnailWidget->setupScratchPad(m_resourceProvider, Qt::white);
}

void KisPresetSaveWidget::showDialog()
{

    setModal(true);

    // set the name of the current brush preset area.
    KisPaintOpPresetSP preset = m_resourceProvider->currentPreset();

    if (preset) {
        this->currentBrushNameLabel->setText(preset->name());
    }

    show();

    // this is setting the area we will "capture" for saving the brush preset. It can potentially be a different
    // area that the entire scratchpad
    this->brushPresetThumbnailWidget->setCutoutOverlayRect(QRect(0, 0, brushPresetThumbnailWidget->height(), brushPresetThumbnailWidget->width()));



    // we will default to reusing the previous preset thumbnail
    // have that checked by default, hide the other elements, and load the last preset image
    connect(clearBrushPresetThumbnailButton, SIGNAL(clicked(bool)), brushPresetThumbnailWidget, SLOT(fillDefault()));
    connect(useExistingThumbnailCheckbox, SIGNAL(clicked(bool)), this, SLOT(usePreviousThumbnail(bool)));

    connect(loadImageIntoThumbnailButton, SIGNAL(clicked(bool)), this, SLOT(loadImageFromFile()));


    this->useExistingThumbnailCheckbox->setChecked(true);
    usePreviousThumbnail(true);

    QDialog::DialogCode result = (QDialog::DialogCode)this->exec();

    if(result) {
        qDebug() << "stuff ran in the dialog";
    }
}

void KisPresetSaveWidget::usePreviousThumbnail(bool usePrevious)
{

    // hide other elements if we are using the previous thumbnail
    this->clearBrushPresetThumbnailButton->setVisible(!usePrevious);
    this->loadImageIntoThumbnailButton->setVisible(!usePrevious);

    // load the previous thumbnail if we are using the existing one
    if (usePrevious) {
        this->brushPresetThumbnailWidget->paintPresetImage();
    } else {
        brushPresetThumbnailWidget->fillDefault(); // fill with white if we want a new preview area
    }

    this->brushPresetThumbnailWidget->allowPainting(!usePrevious); // don't allow drawing if we are using the existing preset
}

void KisPresetSaveWidget::loadImageFromFile()
{
    // create a dialog to retrieve an image file.
    KoFileDialog dialog(0, KoFileDialog::OpenFile, "OpenDocument");
    dialog.setMimeTypeFilters(KisImportExportManager::mimeFilter(KisImportExportManager::Import));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
    QString filename = dialog.filename(); // the filename() returns the entire path & file name, not just the file name


    if (filename != "") { // empty if "cancel" is pressed
        // take that file and load it into the thumbnail are
        const QImage imageToLoad(filename);

        brushPresetThumbnailWidget->fillTransparent(); // clear the background in case our new image has transparency
        brushPresetThumbnailWidget->paintCustomImage(imageToLoad);
    }

}

#include "moc_kis_paintop_presets_save.cpp"
