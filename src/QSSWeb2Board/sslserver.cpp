#include "sslserver.h"
#include "QtWebSockets/QWebSocketServer"
#include <QtCore/QDebug>
#include <QtCore/QFile>
#include <QtNetwork/QSslCertificate>
#include <QtNetwork/QSslKey>

QT_USE_NAMESPACE

Web2BoardSocket::Web2BoardSocket(QWebSocket* ws, int clientID):
                       m_pWeb2Board{Q_NULLPTR},
                       m_pWebSocket{Q_NULLPTR}
{
    m_pWebSocket = ws;
    m_pWeb2Board = new Web2Board(clientID);

}

Web2BoardSocket::~Web2BoardSocket(){

    if(m_pWeb2Board) delete m_pWeb2Board;
    if(m_pWebSocket) delete m_pWebSocket;
}


//! [constructor]
SSLServer::SSLServer(quint16 port, QObject *parent) :
    QObject(parent),
    m_pWebSocketServer{Q_NULLPTR},
    m_web2BoardSocketClients(),
    __clientID{0}
{
    m_pWebSocketServer = new QWebSocketServer(QStringLiteral("SSL Web2Board Server"),
                                              QWebSocketServer::SecureMode,
                                              parent);
    QSslConfiguration sslConfiguration;

#ifdef ONLINE_COMPILER
    qInfo() << "Cert: star_bq_com";
    QFile certFile(QStringLiteral(":/res/star_bq_com.crt"));
    QFile keyFile(QStringLiteral(":/res/star_bq_com.key"));
#else
    qInfo() << "Cert: www.web2board.es";
    QFile certFile(QStringLiteral(":/res/www.web2board.es.crt"));
    QFile keyFile(QStringLiteral(":/res/www.web2board.es.key"));
#endif
    certFile.open(QIODevice::ReadOnly);
    keyFile.open(QIODevice::ReadOnly);
    QSslCertificate certificate(&certFile, QSsl::Pem);
    QSslKey sslKey(&keyFile, QSsl::Rsa, QSsl::Pem);
    certFile.close();
    keyFile.close();
    sslConfiguration.setPeerVerifyMode(QSslSocket::VerifyNone);
    sslConfiguration.setLocalCertificate(certificate);
    sslConfiguration.setPrivateKey(sslKey);
    sslConfiguration.setProtocol(QSsl::TlsV1SslV3);

    m_pWebSocketServer->setSslConfiguration(sslConfiguration);

    if (m_pWebSocketServer->listen(QHostAddress::Any, port))
    {
        qInfo() << "======================================================";
        qInfo() << "  SSL Web2Board Server listening on port" << port;
        qInfo() << "  DO NOT CLOSE THIS WINDOW // NO CIERRES ESTA VENTANA";
        qInfo() << "======================================================";

        connect(m_pWebSocketServer, &QWebSocketServer::newConnection,
                this, &SSLServer::onNewConnection);
        connect(m_pWebSocketServer, &QWebSocketServer::sslErrors,
                this, &SSLServer::onSslErrors);
    }else{
        qCritical() << "Error opening server";
    }
}
//! [constructor]

SSLServer::~SSLServer()
{
    m_pWebSocketServer->close();
    qDeleteAll(m_web2BoardSocketClients.begin(), m_web2BoardSocketClients.end());
}


//! [onNewConnection]
void SSLServer::onNewConnection()
{
    Web2BoardSocket *pWeb2BoardSocket = new Web2BoardSocket(m_pWebSocketServer->nextPendingConnection(), __clientID++);

#ifdef ONLINE_COMPILER
    // Allow only connections from  *.bq.com
    if(!pWeb2BoardSocket->m_pWebSocket->origin().endsWith(".bq.com")){
        qCritical() << "Unknown client connected. Rejected: " << pWeb2BoardSocket->m_pWebSocket->origin();
        pWeb2BoardSocket->m_pWebSocket->close();
        return;
    }
#endif

    qDebug() << "Client connected:" << pWeb2BoardSocket->m_pWebSocket->peerName() << pWeb2BoardSocket->m_pWebSocket->origin();
    connect(pWeb2BoardSocket->m_pWebSocket, &QWebSocket::textMessageReceived, pWeb2BoardSocket->m_pWeb2Board, &Web2Board::handleTextMessage);
    connect(pWeb2BoardSocket->m_pWebSocket, &QWebSocket::disconnected, this, &SSLServer::socketDisconnected);
    m_web2BoardSocketClients << pWeb2BoardSocket;
}
//! [onNewConnection]

//! [socketDisconnected]

void SSLServer::socketDisconnected()
{
    qDebug() << "Client disconnected";
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (pClient)
    {
        //remove clients with that socket client
        QMutableListIterator<Web2BoardSocket*> i(m_web2BoardSocketClients);
        while (i.hasNext()) {
            qDebug() << "client";
            if (i.next()->m_pWebSocket == pClient){
                qDebug() << "disconnected";
                i.value()->deleteLater();
                i.remove();
            }
        }

    }
}

void SSLServer::onSslErrors(const QList<QSslError> &)
{
    qDebug() << "Ssl errors occurred";
}
//! [socketDisconnected]
