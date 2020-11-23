#ifndef UDPTRANSCEIVER_H
#define UDPTRANSCEIVER_H

#include <QObject>
#include <QQueue>
#include <QtNetwork/QHostAddress>

class QUdpSocket;

typedef void (*DatagramProcessor)(QByteArray &data, void * usrParam);

class UDPTransceiver : public QObject
{
	Q_OBJECT

public:
	UDPTransceiver(QObject *parent);
	// For multicasting.
	UDPTransceiver(QObject *parent, const char* address, unsigned int port);
	// For broadcasting.
	UDPTransceiver(QObject *parent, unsigned int port);
	~UDPTransceiver();
	void SetDatagramProcessor(DatagramProcessor dp, void * usrParam);
	void SendDataNow(QByteArray data, QString addr, int port);

	static QByteArray GetNextMsg(QByteArray &dataBuf, QByteArray &header);
public slots:
	void SendData(QByteArray data, int port = 0);

	// Start transceiver process.
	void Start();
signals:
	void DataSendPending();
private slots:
	void DoSendData();
	void ProcessPendingDatagrams();
	void SocketError(QAbstractSocket::SocketError e);
private:
	
	QQueue<QByteArray> m_msgSendQueue;
	QUdpSocket * m_udpSocket;
	unsigned int m_port;
	QHostAddress m_localAddress;
	DatagramProcessor m_processor;
	void * m_usrParam;
};


#endif