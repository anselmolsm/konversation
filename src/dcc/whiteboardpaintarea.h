/*
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
*/

/*
  Copyright (C) 2009-2010 Bernd Buschinski <b.buschinski@web.de>
*/

#ifndef WHITEBOARDPAINTAREA_H
#define WHITEBOARDPAINTAREA_H

#include <QWidget>
#include <QPoint>
#include <QColor>
#include <QPen>

#include "whiteboardglobals.h"

class QPainter;
class QPaintEvent;
class QResizeEvent;
class QMouseEvent;
class QPixmap;
class QKeyEvent;

namespace Konversation
{
    namespace DCC
    {
        class WhiteBoardPaintArea : public QWidget
        {
            Q_OBJECT

        public:
            WhiteBoardPaintArea(QWidget* parent = 0);
            ~WhiteBoardPaintArea();

        public slots:
            void setTool(Konversation::DCC::WhiteBoardGlobals::WhiteBoardTool tool);
            void setForegroundColor(const QColor& color);
            void setBackgroundColor(const QColor& color);
            void swapColors(const QColor& newForeground, const QColor& newBackground);
            void setPenWidth(int width);
            void setFont(const QFont& font);

            void clear();

            void drawLine(int lineWidth, const QColor& penColor, const QColor& brushColor,
                          int xFrom, int yFrom, int xTo, int yTo);
            void drawRectangle(int lineWidth, const QColor& penColor,
                               int xFrom, int yFrom, int xTo, int yTo);
            void drawFilledRectangle(int lineWidth, const QColor& penColor, const QColor& brushColor,
                                     int xFrom, int yFrom, int xTo, int yTo);
            void drawEllipse(int lineWidth, const QColor& penColor,
                             int xFrom, int yFrom, int xTo, int yTo);
            void drawFilledEllipse(int lineWidth, const QColor& penColor, const QColor& brushColor,
                                   int xFrom, int yFrom, int xTo, int yTo);
            void drawArrow(int lineWidth, const QColor& penColor,
                           int xFrom, int yFrom, int xTo, int yTo);
            void useEraser(int lineWidth, int xFrom, int yFrom, int xTo, int yTo);
            void useFloodFill(int x, int y, const QColor& color);
            void useBlt(int x1src, int y1src, int x2src, int y2src, int xdest, int ydest);
            void useText(int x1, int y1, const QString& textString);
            void useTextExtended(int x1, int y1, const QFont& font, const QColor& foreGround, const QColor& backGround, const QString& textString);

            void save(const QString& fileName);

        signals:
            void drawedPencil(int lineWidth, const QColor& penColor, const QColor& brushColor,
                              int xFrom, int yFrom, int xTo, int yTo);
            void drawedLine(int lineWidth, const QColor& penColor, const QColor& brushColor,
                            int xFrom, int yFrom, int xTo, int yTo);
            void drawedRectangle(int lineWidth, const QColor& penColor,
                                 int xFrom, int yFrom, int xTo, int yTo);
            void drawedFilledRectangle(int lineWidth, const QColor& penColor, const QColor& brushColor,
                                       int xFrom, int yFrom, int xTo, int yTo);
            void drawedEllipse(int lineWidth, const QColor& penColor,
                               int xFrom, int yFrom, int xTo, int yTo);
            void drawedFilledEllipse(int lineWidth, const QColor& penColor, const QColor& brushColor,
                                     int xFrom, int yFrom, int xTo, int yTo);
            void drawedArrow(int lineWidth, const QColor& penColor,
                             int xFrom, int yFrom, int xTo, int yTo);
            void usedEraser(int lineWidth, int xFrom, int yFrom, int xTo, int yTo);
            void usedFloodFill(int x, int y, const QColor& color);
            void usedText(int x, int y, const QString& text);
            void usedTextExtended(int x, int y, const QFont& font, const QColor& textColor, const QColor& background, const QString& text);

        protected:
            virtual void paintEvent(QPaintEvent * event);
            virtual void resizeEvent(QResizeEvent * event);
            virtual void mousePressEvent(QMouseEvent * event);
            virtual void mouseReleaseEvent(QMouseEvent * event);
            virtual void mouseMoveEvent(QMouseEvent * event);
            virtual void keyPressEvent(QKeyEvent * event);

        private:
            inline void makeLastPosInvalid();
            inline bool isLastPosValid();
            inline void checkImageSize(int x1, int y1, int x2, int y2, int penWidth = 1);
            inline void resizeImage(int width, int height);

            inline QPen getPen(const QColor& color, int lineWidth, WhiteBoardGlobals::WhiteBoardTool tool);

            inline void floodfill(int x, int y, const QColor& fillColor);

            inline void arrow(QPainter* painter, int x1, int y1, int x2, int y2);

            inline void text(QPaintDevice* device, const QFont& font, const QColor& foreGround,
                             const QColor& backGround, int x1, int y1, const QString& textString, bool drawSelection = true,
                             Konversation::DCC::WhiteBoardGlobals::WhiteBoardTool tool = WhiteBoardGlobals::Text);
            inline void finishText();

            QPixmap* m_imagePixmap;
            QPixmap* m_overlayPixmap;

            bool m_mousePressed;
            QPoint m_lastPos;

            WhiteBoardGlobals::WhiteBoardTool m_tool;

            QColor m_foregroundColor;
            QColor m_backgroundColor;

            int m_penWidth;

            QFont m_font;
            QString m_writtenText;
        };
    }
}

#endif // WHITEBOARDPAINTAREA_H
