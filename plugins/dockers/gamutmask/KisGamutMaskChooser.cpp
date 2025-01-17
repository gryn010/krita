/*
 *  Copyright (c) 2018 Anna Medonosova <anna.medonosova@gmail.com>
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

#include "KisGamutMaskChooser.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QAbstractItemDelegate>
#include <QMenu>
#include <QActionGroup>
#include <QToolButton>
#include <QFontMetrics>
#include <QTextDocument>
#include <QTextLayout>

#include <KoResourceServer.h>
#include <KoResourceServerProvider.h>
#include <KoResourceItemChooser.h>
#include <KoResourceServerAdapter.h>
#include <kis_icon_utils.h>
#include <kis_config.h>

/// The resource item delegate for rendering the resource preview
class KisGamutMaskDelegate: public QAbstractItemDelegate
{
public:
    KisGamutMaskDelegate(QObject * parent = 0) : QAbstractItemDelegate(parent)
      , m_mode(KisGamutMaskChooser::ViewMode::THUMBNAIL) {}
    ~KisGamutMaskDelegate() override {}

    /// reimplemented
    void paint(QPainter *, const QStyleOptionViewItem &, const QModelIndex &) const override;

    /// reimplemented
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex &) const override {
        return option.decorationSize;
    }

    void setViewMode(KisGamutMaskChooser::ViewMode mode) {
        m_mode = mode;
    }

private:
    KisGamutMaskChooser::ViewMode m_mode;
};

void KisGamutMaskDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    painter->save();
    painter->setRenderHint(QPainter::SmoothPixmapTransform, true);

    if (!index.isValid())
        return;

    KoResource* resource = static_cast<KoResource*>(index.internalPointer());
    KoGamutMask* mask = static_cast<KoGamutMask*>(resource);

    if (!mask) {
        return;
    }

    QImage preview = mask->image();

    if(preview.isNull()) {
        return;
    }

    QRect paintRect = option.rect.adjusted(1, 1, -1, -1);

    if (m_mode == KisGamutMaskChooser::ViewMode::THUMBNAIL) {
        painter->drawImage(paintRect.x(), paintRect.y(),
                           preview.scaled(paintRect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        if (option.state & QStyle::State_Selected) {
            painter->setCompositionMode(QPainter::CompositionMode_Overlay);
            painter->setOpacity(0.5);
            painter->fillRect(paintRect, Qt::white);
            painter->setCompositionMode(QPainter::CompositionMode_SourceOver);
            painter->setOpacity(1.0);
            painter->setPen(QPen(option.palette.highlight(), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
            QRect selectedBorder = option.rect.adjusted(1, 1, -1, -1);
            painter->drawRect(selectedBorder);
        }
    } else {
        QSize previewSize(paintRect.height(), paintRect.height());
        painter->drawImage(paintRect.x(), paintRect.y(),
                           preview.scaled(previewSize, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

        int leftMargin = 8;
        int rightMargin = 7;
        int vertMargin = 4;
        int descOffset = 7;
        QFont font = option.font;
        font.setBold(true);
        painter->setFont(font);
        QRectF titleRect(QPointF(previewSize.width() + leftMargin, paintRect.y() + vertMargin),
                       QPointF(paintRect.width() - rightMargin, paintRect.y() + descOffset + painter->fontMetrics().lineSpacing()));
        painter->drawText(titleRect, Qt::AlignLeft,
                          painter->fontMetrics().elidedText(
                              mask->title(), Qt::ElideRight, titleRect.width()
                              )
                          );

        if (!mask->description().isEmpty() && !mask->description().isNull()) {
            font.setPointSize(font.pointSize()-1);
            font.setBold(false);
            font.setStyle(QFont::StyleItalic);
            painter->setFont(font);
            QRectF descRect(QPointF(previewSize.width() + leftMargin, paintRect.y() + descOffset + painter->fontMetrics().lineSpacing()),
                            QPointF(paintRect.right() - rightMargin, paintRect.bottom() - vertMargin));

            int numLines = floor(((float)descRect.height() / (float)painter->fontMetrics().lineSpacing()));
            if (numLines > 0) {
                int elideWidth = 0;
                QTextLayout textLayout(mask->description());
                textLayout.beginLayout();
                for (int i = 0; i < numLines; i++) {
                    QTextLine line = textLayout.createLine();
                    if (line.isValid()) {
                        line.setLineWidth(descRect.width());
                        elideWidth += line.naturalTextWidth();
                    }
                }

                QString elidedText = painter->fontMetrics().elidedText(mask->description(), Qt::ElideRight, elideWidth);
                painter->drawText(descRect, Qt::AlignLeft|Qt::TextWordWrap, elidedText);
            }
        }
    }

    painter->restore();
}


KisGamutMaskChooser::KisGamutMaskChooser(QWidget *parent) : QWidget(parent)
{
    m_delegate = new KisGamutMaskDelegate(this);

    KoResourceServer<KoGamutMask>* rServer = KoResourceServerProvider::instance()->gamutMaskServer();
    QSharedPointer<KoAbstractResourceServerAdapter> adapter(new KoResourceServerAdapter<KoGamutMask>(rServer));
    m_itemChooser = new KoResourceItemChooser(adapter, this);
    m_itemChooser->setItemDelegate(m_delegate);
    m_itemChooser->showTaggingBar(true);
    m_itemChooser->showButtons(false);
    m_itemChooser->setColumnCount(4);
    m_itemChooser->setSynced(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);

    // TODO: menu for view mode change
    QMenu* menu = new QMenu(this);
    menu->setStyleSheet("margin: 6px");
    menu->addSection(i18n("Display"));

    QActionGroup *actionGroup = new QActionGroup(this);

    KisConfig cfg(true);
    m_mode = KisGamutMaskChooser::ViewMode(cfg.readEntry<quint32>("GamutMasks.viewMode", KisGamutMaskChooser::THUMBNAIL));

    QAction* action = menu->addAction(KisIconUtils::loadIcon("view-preview"),
                                      i18n("Thumbnails"), this, SLOT(slotSetModeThumbnail()));
    action->setCheckable(true);
    action->setChecked(m_mode == KisGamutMaskChooser::THUMBNAIL);
    action->setActionGroup(actionGroup);

    action = menu->addAction(KisIconUtils::loadIcon("view-list-details"),
                             i18n("Details"), this, SLOT(slotSetModeDetail()));
    action->setCheckable(true);
    action->setChecked(m_mode == KisGamutMaskChooser::DETAIL);
    action->setActionGroup(actionGroup);

    // setting the view mode
    setViewMode(m_mode);
    m_itemChooser->setViewModeButtonVisible(true);
    QToolButton* viewModeButton = m_itemChooser->viewModeButton();
    viewModeButton->setMenu(menu);

    layout->addWidget(m_itemChooser);
    setLayout(layout);

    connect(m_itemChooser, SIGNAL(resourceSelected(KoResource*)), this, SLOT(resourceSelected(KoResource*)));
}

KisGamutMaskChooser::~KisGamutMaskChooser()
{

}

void KisGamutMaskChooser::setCurrentResource(KoResource *resource)
{
    m_itemChooser->setCurrentResource(resource);
}

void KisGamutMaskChooser::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateViewSettings();
}

void KisGamutMaskChooser::setViewMode(KisGamutMaskChooser::ViewMode mode)
{
    m_mode = mode;
    updateViewSettings();
}

void KisGamutMaskChooser::updateViewSettings()
{
    KisConfig cfg(false);
    cfg.writeEntry("GamutMasks.viewMode", qint32(m_mode));

    if (m_mode == KisGamutMaskChooser::THUMBNAIL) {
        m_itemChooser->setSynced(true);
        m_delegate->setViewMode(m_mode);
    } else if (m_mode == KisGamutMaskChooser::DETAIL) {
        m_itemChooser->setSynced(false);
        m_itemChooser->setColumnCount(1);
        m_itemChooser->setRowHeight(this->fontMetrics().lineSpacing()*4);
        m_itemChooser->setColumnWidth(m_itemChooser->width());
        m_delegate->setViewMode(m_mode);
    }
}

void KisGamutMaskChooser::resourceSelected(KoResource* resource)
{
    emit sigGamutMaskSelected(static_cast<KoGamutMask*>(resource));
}

void KisGamutMaskChooser::slotSetModeThumbnail()
{
    setViewMode(KisGamutMaskChooser::THUMBNAIL);
}

void KisGamutMaskChooser::slotSetModeDetail()
{
    setViewMode(KisGamutMaskChooser::DETAIL);
}
