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

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "wpan/txrequest16.h"
#include "wpan/txrequest64.h"
#include "wpan/rxresponse16.h"
#include "wpan/rxresponse64.h"
#include "wpan/txstatusresponse.h"

#include <QSerialPort>
#include <QtWidgets/QScrollBar>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->sendButton->setEnabled(false);
    ui->data->setEnabled(false);

    xbee = new XBee(this);
    xbee->setMode(XBee::API1Mode);

    connect(xbee, SIGNAL(receivedTransmitStatus(Wpan::TxStatusResponse*)), this, SLOT(onReceivedTransmitStatus(Wpan::TxStatusResponse*)));
    connect(xbee, SIGNAL(receivedRxResponse16(Wpan::RxResponse16*)), this, SLOT(onReceivedRxResponse16(Wpan::RxResponse16*)));
    connect(xbee, SIGNAL(receivedRxResponse64(Wpan::RxResponse64*)), this, SLOT(onReceivedRxResponse64(Wpan::RxResponse64*)));

    connect(ui->openButton, SIGNAL(clicked()), this, SLOT(onOpenSerialPortButtonClicked()));
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(onSendCommandButtonClicked()));

    foreach(QSerialPortInfo port, QSerialPortInfo::availablePorts())
    {
        ui->serialCombo->addItem(port.portName(), port.systemLocation());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onOpenSerialPortButtonClicked()
{
    bool success = false;
    QString serialPort = ui->serialCombo->currentData().toString();
    success = xbee->setSerialPort(serialPort);

    ui->sendButton->setEnabled(success);
    ui->data->setEnabled(success);

    log(success ? QString("Serial port %1 opened").arg(serialPort) : QString("Failed to open serial port %1").arg(serialPort));
}

void MainWindow::onSendCommandButtonClicked()
{
    bool ok = false;
    quint64 addr = ui->addr->text().toULongLong(&ok, 16);
    if(!ok) {
        QMessageBox * mess = new QMessageBox(this);
        mess->setText("Error : Failed to get destination address");
        connect(mess, SIGNAL(accepted()), mess, SLOT(deleteLater()));
        connect(mess, SIGNAL(rejected()), mess, SLOT(deleteLater()));
        mess->show();
        return;
    }
    // 64 bits address
    if(addr > 0xFFFF) {
        //log(QString("Sending '%1' to %2 (64 bits addressing)").arg(ui->data->text()).arg(addr));
        Wpan::TxRequest64 req;
        req.setDestinationAddress(addr);
        req.setData(ui->data->text().toLatin1());
        xbee->send(&req);
    }
    // 16 bits address
    else {
        //log(QString("Sending '%1' to %2 (16 bits addressing)").arg(ui->data->text()).arg(addr));
        log(ui->data->text().prepend(">> "));
        Wpan::TxRequest16 req;
        req.setDestinationAddress(addr);
        req.setData(ui->data->text().toLatin1());
        req.setFrameId(20);
        xbee->send(&req);
    }
    ui->data->clear();
}

void MainWindow::log(const QString &log)
{
    QString text = ui->console->toPlainText();
    text.append(log).append("\n");
    ui->console->setPlainText(text);
    ui->console->verticalScrollBar()->setValue(ui->console->verticalScrollBar()->maximum());
}

void MainWindow::onReceivedTransmitStatus(Wpan::TxStatusResponse *)
{
    //log(r->toString());
}

void MainWindow::onReceivedRxResponse16(Wpan::RxResponse16 * r)
{
    log(r->data().prepend("<< "));
}

void MainWindow::onReceivedRxResponse64(Wpan::RxResponse64 * r)
{
    log(r->data().prepend("<< "));
}