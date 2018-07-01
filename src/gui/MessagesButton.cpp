// Copyright (c) 2011-2016 The Cryptonote developers
// Copyright (c) 2015-2016 XDN developers
// Copyright (c) 2018-2019 xDrop developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <QPainter>
#include <QPixmap>
#include <QFont>
#include <QIcon>
#include <QStylePainter>
#include <QStyleOptionButton>

#include "MessagesButton.h"

MessagesButton::MessagesButton( QWidget *parent) :
	QPushButton(parent)
{

}

void MessagesButton::paintEvent(QPaintEvent* e)
{
  QPushButton::paintEvent(e);
  quint64 _number = unreadCount;
  if (_number){
    if (_number > 9) {
      QPainter paint(this);
      paint.setRenderHint(QPainter::Antialiasing);
      paint.setPen(Qt::NoPen);
      paint.setBrush(QColor("#ef5858"));
      paint.drawEllipse(QPointF(width()-27,height()/2-4), 12,12);
      QFont font;
      paint.setPen(QColor("#fbfbfc"));
      font.setPixelSize(12);
      font.setFamily("Lato Bold");
      paint.setFont(font);
      paint.drawText(width()-34,height()/2-11, 14, 14, Qt::AlignHCenter | Qt::AlignTop, QString("%1").arg(_number));
      paint.save();
      paint.end();
    }else{
      QPainter paint(this);
      paint.setRenderHint(QPainter::Antialiasing);
      paint.setPen(Qt::NoPen);
      paint.setBrush(QColor("#ef5858"));
      paint.drawEllipse(QPointF(width()-27,height()/2-4), 12,12);
      QFont font;
      paint.setPen(QColor("#fbfbfc"));
      font.setPixelSize(16);
      font.setFamily("Lato Black");
      paint.setFont(font);
      paint.drawText(width()-36,height()/2-14, 18, 18, Qt::AlignHCenter | Qt::AlignTop, QString("%1").arg(_number));
      paint.save();
      paint.end();
    }
  }
}

MessagesButton::~MessagesButton()
{

}

