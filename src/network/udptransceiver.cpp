#include <QDebug>
#include <QEventLoop>
#include <QUdpSocket>
#include <QByteArray>
#include "udptransceiver.h"

UDPTransceiver::UDPTransceiver(QObject *parent)
	: QObject(parent),m_udpSocket(0),m_port(0),m_processor(0)
{
	m_udpSocket = new QUdpSocket();
	connect(this, SIGNAL(DataSendPending()), this, SLOT(DoSendData()));
	connect(m_udpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
		SLOT(SocketError(QAbstractSocket::SocketError)));
}

UDPTransceiver::UDPTransceiver(QObject *parent, unsigned int port)
	: QObject(parent),m_udpSocket(0),m_port(port),m_processor(0)
{
	m_udpSocket = new QUdpSocket();
	m_udpSocket->bind(port, QUdpSocket::ShareAddress);
	connect(this, SIGNAL(DataSendPending()),this, SLOT(DoSendData()));
	connect(m_udpSocket, SIGNAL(readyRead()),
		this, SLOT(ProcessPendingDatagrams()));
}

UDPTransceiver::UDPTransceiver(QObject *parent,const char* address, unsigned int port)
	: QObject(parent),m_udpSocket(0),m_port(port),m_processor(0)
{
	m_udpSocket = new QUdpSocket();
	m_localAddress = QHostAddress(address);
	m_udpSocket->bind(QHostAddress::AnyIPv4, port, QUdpSocket::ShareAddress);
	bool ret=m_udpSocket->joinMulticastGroup(m_localAddress);

	m_udpSocket->setSocketOption(QAbstractSocket::MulticastTtlOption, 1);//

	connect(this, SIGNAL(DataSendPending()),this, SLOT(DoSendData()));
	connect(m_udpSocket, SIGNAL(readyRead()),
		this, SLOT(ProcessPendingDatagrams()));
}

UDPTransceiver::~UDPTransceiver()
{
	m_udpSocket->close();
	delete m_udpSocket;
}

void UDPTransceiver::SetDatagramProcessor(DatagramProcessor dp)
{
	m_processor = dp;
}

QByteArray UDPTransceiver::GetNextMsg(QByteArray &dataBuf, QByteArray &header)
{
	QByteArray msgRet;
	int pos = dataBuf.indexOf(header);

	if (pos != -1)
	{
		bool ok;
		int len = dataBuf.mid(pos + 2, 2).toHex().toInt(&ok, 16);
		dataBuf.remove(0,pos);

		// Get full message
		if (dataBuf.size() >= len)
		{
			msgRet = dataBuf.mid(0,len);
			dataBuf.remove(0, len);
		}
	}
	else	// header lost. Drop all the data.
	{
		dataBuf.clear();
	}

	return msgRet;
}

void UDPTransceiver::SendData(QByteArray data, int port)
{
	m_msgSendQueue.enqueue(data);
	if(port != 0)
		m_port = port;
	emit DataSendPending();
}

void UDPTransceiver::Start()
{
	while (1)
	{
		qDebug() << "Transceiver process started!";
		QEventLoop loop;
		loop.exec();
	}
}

void UDPTransceiver::SendDataNow(QByteArray data, QString addr, int port){
	m_udpSocket->writeDatagram(data.data(), data.size(),
		QHostAddress(addr), port);
}

void UDPTransceiver::DoSendData()
{
	while(!m_msgSendQueue.isEmpty())
	{
		if (m_port != 0)
		{
			QByteArray msg = m_msgSendQueue.dequeue();
			QString local = m_udpSocket->localAddress().toString();
			qDebug() << local;
			if (m_localAddress.isNull())
			{
				m_udpSocket->writeDatagram(msg.data(), msg.size(),
					QHostAddress::Broadcast, m_port);
			}
			else
			{
				int err = m_udpSocket->writeDatagram(msg.data(), msg.size(),
					m_localAddress, m_port);
			}
		}
		else {
			qDebug() << "Udp socket Error, port was not set.";
			m_msgSendQueue.clear();
		}
	}
}

void UDPTransceiver::ProcessPendingDatagrams()
{
	while (m_udpSocket->hasPendingDatagrams()) {
		QByteArray datagram;
		datagram.resize(m_udpSocket->pendingDatagramSize());
		m_udpSocket->readDatagram(datagram.data(), datagram.size());
		if(m_processor)
			m_processor(datagram);
		//qDebug() << "UDP package received: " << datagram;
	}
}

void UDPTransceiver::SocketError(QAbstractSocket::SocketError e)
{
	qDebug() << m_udpSocket->errorString();
}
