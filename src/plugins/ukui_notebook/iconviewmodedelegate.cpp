/*
* Copyright (C) 2020 Tianjin KYLIN Information Technology Co., Ltd.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, see <http://www.gnu.org/licenses/&gt;.
*
*/

#include <QPainter>
#include <QEvent>
#include <QDebug>
#include <QApplication>
#include <QFontDatabase>
#include <QtMath>
#include <QPainterPath>

#include "notemodel.h"
#include "widget.h"
#include "iconviewmodedelegate.h"
#include "noteview.h"

iconViewModeDelegate::iconViewModeDelegate(QObject *parent)
    : QStyledItemDelegate(parent),
      m_titleFont(QStringLiteral(""), 14, 30),              //标题字体
      m_titleSelectedFont(QStringLiteral(""), 14),          //
      m_dateFont(QStringLiteral(""), 10),                   //日期字体
      m_titleColor(255, 255, 255),                          //标题颜色
      m_dateColor(255, 255, 255),                           //日期颜色
      m_ActiveColor(218, 233, 239),
      m_notActiveColor(175, 212, 228),                      //默认选中背景色
      m_hoverColor(80, 80, 80),                             //悬停颜色
      m_selectColor(43, 49, 60),                            //选中颜色
      m_applicationInactiveColor(207, 207, 207),            //应用程序可见，但未选择显示在前面时背景色
      m_separatorColor(221, 221, 221),
      m_defaultColor(0, 0, 0),
      m_noteColor(0, 0, 0),                                 //便签头颜色
      m_rowHeight(200),                                      //item宽度
      m_maxFrame(200),
      m_rowRightOffset(0),
      m_state(Normal),
      m_isActive(false)
{
    //QTimeLine拥有一个时间轴来控制时间动画和事件
    //参数1为毫秒级 总运行时间0.3秒
    m_timeLine = new QTimeLine(300, this);

    //对QTImeLine的时间轴设置动画帧数,共200张静态画面图组成
    //Qt默认40ms一帧，也就是一秒可以最多走25帧
    m_timeLine->setFrameRange(0,m_maxFrame);
    //时间轴走的时候，会不断发出frameChanged信号，setUpdateInterval控制多少时间发一次
    m_timeLine->setUpdateInterval(10);
    //设置你的时间变换曲线，即明确你的时间是先快后慢，还是先慢后快，或者线性
    m_timeLine->setCurveShape(QTimeLine::EaseInCurve);

    connect( m_timeLine, &QTimeLine::frameChanged, [this](){
        emit sizeHintChanged(m_animatedIndex);
    });

    connect(m_timeLine, &QTimeLine::finished, [this](){
        m_animatedIndex = QModelIndex();
        m_state = Normal;
    });
}

void iconViewModeDelegate::setState(States NewState, QModelIndex index)
{
    m_animatedIndex = index;

    auto startAnimation = [this](QTimeLine::Direction diretion, int duration){
        m_timeLine->setDirection(diretion);
        m_timeLine->setDuration(duration);
        m_timeLine->start();
    };

    switch ( NewState ){
    case Insert:
        startAnimation(QTimeLine::Forward, m_maxFrame);
        break;
    case Remove:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case MoveOut:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case MoveIn:
        startAnimation(QTimeLine::Backward, m_maxFrame);
        break;
    case Normal:
        m_animatedIndex = QModelIndex();
        break;
    }

    m_state = NewState;
}

void iconViewModeDelegate::setAnimationDuration(const int duration)
{
    m_timeLine->setDuration(duration);
}

void iconViewModeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    //qDebug() << "icon paint " << opt.rect.width() << opt.rect.height();

    //绘制第一层便签头背景
    int m_noteColor{index.data(NoteModel::NoteColor).toInt()};
    painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter->setBrush(QBrush(intToQcolor(m_noteColor)));

    painter->setPen(Qt::transparent);
    opt.rect.setLeft(opt.rect.left() + 3);
    opt.rect.setWidth(opt.rect.width() - 12);
    opt.rect.setHeight(opt.rect.height() - 46);
    {
        QPainterPath painterPath;
        painterPath.addRoundedRect(opt.rect, 7, 7);
        painter->drawPath(painterPath);
    }

    //绘制第二层底色背景
    painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
    painter->setBrush(opt.palette.color(QPalette::Base));

    //系统默认 255 、 248  深色模式 34 30
    if(painter->brush().color().value() == 255)
    {
        painter->setBrush(QColor(245,245,245));
    }else if(painter->brush().color().value() == 248)
    {
        painter->setBrush(QColor(245,245,245));
    }else if(painter->brush().color().value() == 30)
    {
        painter->setBrush(QColor(40,40,46));
    }else if(painter->brush().color().value() == 34)
    {
        painter->setBrush(QColor(40,40,46));
    }
//    if(sink == 0)
//    {
//        painter->setBrush(QColor(40,40,46));
//    }else{
//        painter->setBrush(QColor(245,245,245));
//    }
    painter->setPen(Qt::transparent);
    opt.rect.setHeight(opt.rect.height() - 0);
    opt.rect.setTop(opt.rect.top() + 5);
    {
        QPainterPath painterPath;
        painterPath.addRoundedRect(opt.rect, 0, 0);
        painter->drawPath(painterPath);
    }

    paintBackground(painter, opt, index);
    paintLabels(painter, option, index);
}

QSize iconViewModeDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    //qDebug() << "icon sizeHint";
    Q_UNUSED(option);
    Q_UNUSED(index);
    return QSize(227,246);
}

QTimeLine::State iconViewModeDelegate::animationState()
{
    return m_timeLine->state();
}

void iconViewModeDelegate::paintBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt = option;
    opt.rect.setWidth(option.rect.width() - m_rowRightOffset);
    if((option.state & QStyle::State_Selected) == QStyle::State_Selected)
    {
        //应用程序是可见的，并被选择在前面。
        if(qApp->applicationState() == Qt::ApplicationActive){      //返回应用程序的当前状态。
            if(m_isActive){//用指定的画笔填充给定的矩形。
                painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
                painter->setBrush(opt.palette.color(QPalette::Base));

                //系统默认 255 、 248  深色模式 34 30
                if(painter->brush().color().value() == 255)
                {
                    painter->setBrush(QColor(255,255,255));
                }
                else if(painter->brush().color().value() == 34)
                {
                    painter->setBrush(QBrush(m_selectColor));
                }
//                if(sink == 0){
//                painter->setBrush(QBrush(m_selectColor));
//                }else{
//                    painter->setBrush(QBrush(QColor(255,255,255)));
//                }
                painter->setPen(Qt::transparent);
                QPainterPath painterPath;
                painterPath.addRoundedRect(opt.rect, 0, 0);
                painter->drawPath(painterPath);
                //painter->fillRect(option.rect, QBrush(m_ActiveColor));//浅蓝
            }else{
                painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
                painter->setBrush(opt.palette.color(QPalette::Base));

                //系统默认 255 、 248  深色模式 34 30
                if(painter->brush().color().value() == 255)
                {
                    painter->setBrush(QColor(255,255,255));
                }
                else if(painter->brush().color().value() == 34)
                {
                    painter->setBrush(QBrush(m_selectColor));
                }
//                if(sink == 0){
//                painter->setBrush(QBrush(m_selectColor));
//                }else{
//                    painter->setBrush(QBrush(QColor(255,255,255)));
//                }
                painter->setPen(Qt::transparent);
                QPainterPath painterPath;
                painterPath.addRoundedRect(opt.rect, 0, 0);
                painter->drawPath(painterPath);
                //painter->fillRect(option.rect, QBrush(m_notActiveColor));//深蓝
            }
            //应用程序可见，但未选择显示在前面
        }else if(qApp->applicationState() == Qt::ApplicationInactive){
            painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
            painter->setBrush(opt.palette.color(QPalette::Base));

            //系统默认 255 、 248  深色模式 34 30
            if(painter->brush().color().value() == 255)
            {
                painter->setBrush(QColor(255,255,255));
            }
            else if(painter->brush().color().value() == 34)
            {
                painter->setBrush(QBrush(m_selectColor));
            }

//            if(sink == 0){
//            painter->setBrush(QBrush(m_selectColor));
//            }else{
//                painter->setBrush(QBrush(QColor(255,255,255)));
//            }
            //painter->setBrush(QBrush(m_defaultColor));
            painter->setPen(Qt::transparent);
            QPainterPath painterPath;
            painterPath.addRoundedRect(opt.rect, 0, 0);
            painter->drawPath(painterPath);
            //painter->fillRect(option.rect, QBrush(m_defaultColor));
        }
    }
    //鼠标悬停时颜色
    //用于指示小部件是否在鼠标下。
    else if((option.state & QStyle::State_MouseOver) == QStyle::State_MouseOver){
        //qDebug() << "当前文件 :" << __FILE__ << "当前函数 :" << __FUNCTION__ << "当前行号 :" << __LINE__;
        painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
        painter->setBrush(opt.palette.color(QPalette::Base));

        //系统默认 255 、 248  深色模式 34 30
        if(painter->brush().color().value() == 255)
        {
            painter->setBrush(QColor(205,205,205));
        }
        else if(painter->brush().color().value() == 34)
        {
            painter->setBrush(QBrush(m_hoverColor));
        }
//        if(sink == 0){
//        painter->setBrush(QBrush(m_hoverColor));
//        }else{
//           painter->setBrush(QBrush(QColor(205,205,205)));
//        }
        painter->setPen(Qt::transparent);
        QPainterPath painterPath;
        painterPath.addRoundedRect(opt.rect, 0, 0);
        painter->drawPath(painterPath);
        //painter->fillRect(option.rect, QBrush(m_hoverColor));//灰色
    //当前item未选中 未悬停时颜色
    }else if((index.row() !=  m_currentSelectedIndex.row() - 1)
             && (index.row() !=  m_hoveredIndex.row() - 1)){
        painter->setRenderHint(QPainter::Antialiasing);  // 反锯齿;
        //painter->fillRect(option.rect, QBrush(m_defaultColor));//黑色
    }

}

void iconViewModeDelegate::paintLabels(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    const int leftOffsetX = 20;        // 标题左边距
    //const int leftoffsetXdate = 75;    // 日期左边距
    const int topOffsetY = 18;         // 标题上方的空格
    const int spaceY = 160;            // 标题和日期之间的空格

    QStyleOptionViewItem opt = option;
    QString title{index.data(NoteModel::NoteFullTitle).toString()};

    QFont titleFont = (option.state & QStyle::State_Selected) == QStyle::State_Selected ? m_titleSelectedFont : m_titleFont;
    QFontMetrics fmTitle(titleFont);
    QRect fmRectTitle = fmTitle.boundingRect(title);

    QString date = parseDateTime(index.data(NoteModel::NoteLastModificationDateTime).toDateTime());
    QFontMetrics fmDate(m_dateFont);
    QRect fmRectDate = fmDate.boundingRect(date);

    double rowPosX = option.rect.x();
    double rowPosY = option.rect.y();
    double rowWidth = option.rect.width();

    double titleRectPosX = rowPosX + leftOffsetX;
    double titleRectPosY = rowPosY;
    double titleRectWidth = rowWidth - 2.0 * leftOffsetX;
    double titleRectHeight = fmRectTitle.height() + topOffsetY;
    //qDebug() << "titleRectHeight" << titleRectHeight << fmRectTitle.height();
    double dateRectPosX = rowPosX + (rowWidth / 2 - fmRectDate.width() / 2);
    //double dateRectPosY = rowPosY + fmRectTitle.height() + topOffsetY;
    double dateRectPosY = rowPosY + 26 + topOffsetY;
    double dateRectWidth = rowWidth;
    //double dateRectHeight = fmRectDate.height() + spaceY;
    double dateRectHeight = 18 + spaceY;

    double rowRate = m_timeLine->currentFrame()/(m_maxFrame * 1.0);
    double currRowHeight = m_rowHeight * rowRate;

    auto drawStr = [painter](double posX, double posY, double width, double height, QColor color, QFont font, QString str){
        QRectF rect(posX, posY, width, height);
        painter->setPen(color);
        painter->setFont(font);
        painter->drawText(rect, Qt::AlignBottom, str);
    };

    // 设置标题和日期字符串的边界矩形
    if(index.row() == m_animatedIndex.row()){
        if(m_state == MoveIn){
            titleRectHeight = topOffsetY + fmRectTitle.height() + currRowHeight;

            dateRectPosY = titleRectHeight;
            dateRectHeight = fmRectDate.height() + spaceY;

        }else{

            if((fmRectTitle.height() + topOffsetY) >= ((1.0 - rowRate) * m_rowHeight)){
                titleRectHeight = (fmRectTitle.height() + topOffsetY) - (1.0 - rowRate) * m_rowHeight;
                //qDebug() << "dateRectHeight1111" << dateRectHeight;
            }else{
                titleRectHeight = 0;

                double labelsSumHeight = fmRectTitle.height() + topOffsetY + fmRectDate.height() + spaceY;
                double bottomSpace = m_rowHeight - labelsSumHeight;

                if(currRowHeight > bottomSpace){
                    dateRectHeight = currRowHeight - bottomSpace;
                }else{
                    dateRectHeight = 0;
                }
            }

            dateRectPosY = titleRectHeight + rowPosY;
        }
    }
    //qDebug() << "dateRectHeight" << dateRectHeight << fmRectDate.height() << dateRectPosY << fmRectTitle.height();
    // 绘图标题和日期
    // 超出字符串转换为...
    title = fmTitle.elidedText(title, Qt::ElideRight, int(titleRectWidth));
    painter->setBrush(opt.palette.color(QPalette::Base));

    //系统默认 255 、 248  深色模式 34 30
    if(painter->brush().color().value() == 255)
    {
        drawStr(titleRectPosX, titleRectPosY, titleRectWidth, titleRectHeight, QColor(0,0,0), titleFont, title);
        drawStr(dateRectPosX, dateRectPosY, dateRectWidth, dateRectHeight, QColor(0,0,0), m_dateFont, date);
    }
    else if(painter->brush().color().value() == 34)
    {
        drawStr(titleRectPosX, titleRectPosY, titleRectWidth, titleRectHeight, QColor(244,244,244), titleFont, title);
        drawStr(dateRectPosX, dateRectPosY, dateRectWidth, dateRectHeight, QColor(244,244,244), m_dateFont, date);
    }
//    if(sink == 1)
//    {
//        drawStr(titleRectPosX, titleRectPosY, titleRectWidth, titleRectHeight, QColor(0,0,0), titleFont, title);
//        drawStr(dateRectPosX, dateRectPosY, dateRectWidth, dateRectHeight, QColor(0,0,0), m_dateFont, date);
//    }else{
//        drawStr(titleRectPosX, titleRectPosY, titleRectWidth, titleRectHeight, QColor(244,244,244), titleFont, title);
//        drawStr(dateRectPosX, dateRectPosY, dateRectWidth, dateRectHeight, QColor(244,244,244), m_dateFont, date);
//    }
}

void iconViewModeDelegate::paintSeparator(QPainter*painter, const QStyleOptionViewItem&option, const QModelIndex&index) const
{
    Q_UNUSED(index)

    painter->setPen(QPen(m_separatorColor));
    const int leftOffsetX = 11;
    int posX1 = option.rect.x() + leftOffsetX;
    int posX2 = option.rect.x() + option.rect.width() - leftOffsetX - 1;
    int posY = option.rect.y() + option.rect.height() - 1;

    painter->drawLine(QPoint(posX1, posY),
                      QPoint(posX2, posY));
}

QString iconViewModeDelegate::parseDateTime(const QDateTime &dateTime) const
{
    QLocale usLocale = QLocale::system();
    QString d;
    auto currDateTime = QDateTime::currentDateTime();

    if(dateTime.date() == currDateTime.date()){
        d = tr("Today ");
        d.append(usLocale.toString(dateTime.time(),"hh:mm"));
        return d;
    }else if(dateTime.daysTo(currDateTime) == 1){
        d = tr("Yesterday ");
        d.append(usLocale.toString(dateTime.time(),"hh:mm"));
        return d;
    }
//    else if(dateTime.daysTo(currDateTime) >= 2 &&
//             dateTime.daysTo(currDateTime) <= 7){
//        return usLocale.toString(dateTime.date(), "dddd");
//    }

    return dateTime.toString("yyyy/MM/dd hh:mm");
}

void iconViewModeDelegate::setActive(bool isActive)
{
    m_isActive = isActive;
}

void iconViewModeDelegate::setRowRightOffset(int rowRightOffset)
{
    m_rowRightOffset = rowRightOffset;
}

void iconViewModeDelegate::setHoveredIndex(const QModelIndex &hoveredIndex)
{
    m_hoveredIndex = hoveredIndex;
}

void iconViewModeDelegate::setCurrentSelectedIndex(const QModelIndex &currentSelectedIndex)
{
    m_currentSelectedIndex = currentSelectedIndex;
}

int iconViewModeDelegate::qcolorToInt(const QColor &color) const
{
    //将Color 从QColor 转换成 int
    return (int)(((unsigned int)color.blue()<< 16) | (unsigned short)(((unsigned short)color.green()<< 8) | color.red()));
}

QColor iconViewModeDelegate::intToQcolor(int &intColor) const
{
    int red = intColor & 255;
    int green = intColor >> 8 & 255;
    int blue = intColor >> 16 & 255;
    return QColor(red, green, blue);
}
