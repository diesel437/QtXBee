/*
 * Copyright (C) 2015 ThomArmax (Thomas COIN)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Thomas COIN <esvcorp@gmail.com> 18/04/2015
 */

#ifndef RxBASERESPONSE_H
#define RxBASERESPONSE_H

#include "../XBeeResponse"

#include <QObject>

namespace QtXBee {
namespace Wpan {

/**
 * @brief The RxBaseResponse class is the base to implement a Wpan Response
 * @todo Handle options
 */
class RxBaseResponse : public XBeeResponse
{
    Q_OBJECT
public:
    explicit        RxBaseResponse  (QObject *parent = 0);
    virtual         ~RxBaseResponse ();

    // Reimplemented from XBeePacket
    virtual void    clear           () Q_DECL_OVERRIDE;
    virtual QString toString        () Q_DECL_OVERRIDE;

    void            setRSSI         (const qint8 rssi);
    void            setOptions      (const quint8 options);

    qint8           rssi            () const;
    quint8          options         () const;

protected:
    qint8           m_rssi;
    quint8          m_options;

};

}} // END namespace

#endif // RxBASERESPONSE_H
