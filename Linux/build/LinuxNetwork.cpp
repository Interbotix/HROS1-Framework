/*
 *   LinuxNetwork.cpp
 *
 *   Author: ROBOTIS
 *
 */
#include <iostream>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sstream>
#include "LinuxNetwork.h"

using namespace Robot;
using namespace std;


LinuxSocket::LinuxSocket() : m_sock ( -1 )
{
    memset ( &m_addr, 0, sizeof ( m_addr ) );
}

LinuxSocket::~LinuxSocket()
{
    if ( is_valid() )
        ::close ( m_sock );
}

bool LinuxSocket::create()
{
    m_sock = socket ( AF_INET,
                      SOCK_STREAM,
                      0 );

    if ( ! is_valid() )
        return false;

    // TIME_WAIT - argh
    int on = 1;
    if ( setsockopt ( m_sock, SOL_SOCKET, SO_REUSEADDR, ( const char* ) &on, sizeof ( on ) ) == -1 )
        return false;

    return true;
}

bool LinuxSocket::bind ( const int port )  {
    if ( ! is_valid() )
    {
        return false;
    }

    m_addr.sin_family = AF_INET;
    m_addr.sin_addr.s_addr = INADDR_ANY;
    m_addr.sin_port = htons ( port );

    int bind_return = ::bind ( m_sock,
                             ( struct sockaddr * ) &m_addr,
                             sizeof ( m_addr ) );
    if ( bind_return == -1 )
    {
        return false;
    }

    return true;
}
bool LinuxSocket::listen() const
{
    if ( ! is_valid() )
    {
        return false;
    }

    int listen_return = ::listen ( m_sock, MAXCONNECTIONS );

    if ( listen_return == -1 )
    {
        return false;
    }
    return true;
}

bool LinuxSocket::accept ( LinuxSocket& new_socket ) const
{
    int addr_length = sizeof ( m_addr );
    new_socket.m_sock = ::accept ( m_sock, ( sockaddr * ) &m_addr, ( socklen_t * ) &addr_length );

    if ( new_socket.m_sock <= 0 )
        return false;
    else
        return true;
}

bool LinuxSocket::send ( const std::string s ) const
{
    int status = ::send ( m_sock, s.c_str(), s.size(), MSG_NOSIGNAL );
    if ( status == -1 )
    {
        return false;
    }
    else
    {
        return true;
    }
}

bool LinuxSocket::send ( void* data, int length ) const
{
    int status = ::send ( m_sock, data, length, MSG_NOSIGNAL );
    if ( status == -1 )
    {
        return false;
    }
    else
    {
        return true;
    }
}

int LinuxSocket::recv ( std::string& s ) const
{
    char buf [ MAXRECV + 1 ];

    s = "";

    memset ( buf, 0, MAXRECV + 1 );

    int status = ::recv ( m_sock, buf, MAXRECV, 0 );

    if ( status == -1 )
    {
        cout << "status == -1   errno == " << errno << "  in Socket::recv\n";
        return 0;
    }
    else if ( status == 0 )
    {
        return 0;
    }
    else
    {
        s = buf;
        return status;
    }
}

int LinuxSocket::recv ( void* data, int length ) const
{
	int status = ::recv ( m_sock, data, length, 0 );

    if ( status == -1 )
    {
        cout << "status == -1   errno == " << errno << "  in Socket::recv\n";
        return 0;
    }
    else if ( status == 0 )
    {
        return 0;
    }
    
    return status;
}

bool LinuxSocket::connect ( const std::string host, const int port )
{
    if ( ! is_valid() ) return false;

    m_addr.sin_family = AF_INET;
    m_addr.sin_port = htons ( port );

    int status = inet_pton ( AF_INET, host.c_str(), &m_addr.sin_addr );

    if ( errno == EAFNOSUPPORT ) return false;

    status = ::connect ( m_sock, ( sockaddr * ) &m_addr, sizeof ( m_addr ) );

    if ( status == 0 )
        return true;
    else
        return false;
}

void LinuxSocket::set_non_blocking ( const bool b )
{
    int opts;

    opts = fcntl ( m_sock, F_GETFL );

    if ( opts < 0 )
    {
        return;
    }

    if ( b )
        opts = ( opts | O_NONBLOCK );
    else
        opts = ( opts & ~O_NONBLOCK );

    fcntl ( m_sock,
            F_SETFL,opts );
}



LinuxServer::LinuxServer ( int port )
{
    if ( ! LinuxSocket::create() )
    {
        throw LinuxSocketException ( "Could not create server socket." );
    }

    if ( ! LinuxSocket::bind ( port ) )
    {
        throw LinuxSocketException ( "Could not bind to port." );
    }

    if ( ! LinuxSocket::listen() )
    {
        throw LinuxSocketException ( "Could not listen to socket." );
    }
}

LinuxServer::~LinuxServer()
{
}

const LinuxServer& LinuxServer::operator << ( const std::string& s ) const
{	
    if ( ! LinuxSocket::send ( s ) )
    {
        throw LinuxSocketException ( "Could not write to socket." );
    }

    return *this;
}

const LinuxServer& LinuxServer::operator << ( const int& i ) const
{
    std::stringstream ss;
    ss << i;

    if ( ! LinuxSocket::send ( ss.str() ) )
    {
        throw LinuxSocketException ( "Could not write to socket." );
    }

    return *this;
}

const LinuxServer& LinuxServer::operator >> ( std::string& s ) const
{
    if ( ! LinuxSocket::recv ( s ) )
    {
        throw LinuxSocketException ( "Could not read from socket." );
    }

    return *this;
}

void LinuxServer::accept ( LinuxServer& sock )
{
    if ( ! LinuxSocket::accept ( sock ) )
    {
        throw LinuxSocketException ( "Could not accept socket." );
    }
}

bool LinuxServer::send ( unsigned char* data, int length )
{
	return LinuxSocket::send(data, length);
}

int LinuxServer::recv ( unsigned char* data, int length )
{
	return LinuxSocket::recv(data, length);
}