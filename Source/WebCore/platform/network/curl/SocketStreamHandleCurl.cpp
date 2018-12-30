/*
 * Copyright (C) 2009 Brent Fulgham.  All rights reserved.
 * Copyright (C) 2009 Google Inc.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <wtf/MainThread.h>
#include "SocketStreamHandle.h"
#include "SocketStreamError.h"

#include "KURL.h"
#include "Logging.h"
#include "NotImplemented.h"
#include "SocketStreamHandleClient.h"
#include "wtf/text/cstring.h"
#include <winsock2.h>
#include <process.h>

#define SafeDeleteArray(p) {if(p){delete []((char*)p);p=0;}}

#define IpAddrIsOk(ip)  ((ip)!=0 && (ip)!=(~0))
#define IpAddrIsBad(ip) ((ip)==0 || (ip)==(~0))
#define SocketIsOk(s)   ((s)!=0 && (s)!=(~0))
#define SocketIsBad(s)  ((s)==0 || (s)==(~0))
#define IsSocketConnected( sock, nTimeout )        socket_select( sock, FALSE, nTimeout )

enum{
	ReasonForOnconnect,
	ReasonForReceivedData,
	ReasonForDisconnect,
};

enum WsConnectState{
	WsStateConnenting,
	WsStateConnented,
	WsStateDisconnected,
	WsStateDestroying
};

typedef struct _DList {
	struct _DList  *next,*prev;
}DList,*LPDList;

__forceinline void DListInit(DList& node){ node.next = &node; node.prev = &node; }

__forceinline void DListRemove( LPDList pList ){                            //删除一个节点
	if( pList->next ){
		pList->prev->next = pList->next;
		pList->next->prev = pList->prev;
		pList->next = 0;
		pList->prev = 0;
	}
}

class CDList{
public:
	__forceinline CDList()                               { DListInit(node); }
	__forceinline DList*  Ptr()                          { return &node; }
	__forceinline BOOL       IsEmpty()                   { return node.next == &node; }
	__forceinline LPDList FindFirst()                    { return node.next == &node ? 0 :node.next; }
	__forceinline LPDList FindLast()                     { return node.prev == &node ? 0 :node.prev; }
	__forceinline LPDList FindNext(LPDList pNode)        { return pNode->next == &node ? 0 :pNode->next; }
	__forceinline LPDList FindPrevious(LPDList pNode)    { return pNode->prev == &node ? 0 :pNode->prev; }
	__forceinline void    RemoveSelf()                   { DListRemove( &node ); }  // 自己从链表中断开
	__forceinline void    InsertHead(LPDList pNode){                            //在首位置增中一个节点
		pNode->prev = &node;
		pNode->next = node.next;
		node.next = pNode;
		pNode->next->prev = pNode;
	}
	__forceinline void InsertTail(LPDList pNode){                           //在尾位置增中一个节点
		pNode->next = &node;
		pNode->prev = node.prev;
		pNode->prev->next = pNode;
		node.prev = pNode;
	}

	ULONG Count(){                                              //获得节点总数
		ULONG iCount = 0;
		LPDList pNode = FindFirst();
		while( pNode ){
			iCount ++;
			pNode = FindNext( pNode );
		}
		return iCount;
	}
	ULONG GetTopCount(ULONG nTop){                                 //获得最小节点数
		ULONG iCount = 0;
		LPDList pNode = FindFirst();
		while( pNode ){
			iCount ++; if( iCount >= nTop )break;
			pNode = FindNext( pNode );
		}
		return iCount;
	}
public:
	DList node;
};

class CLockHandle{
public:
	CLockHandle( WTF::Mutex* lpCS){
		__try{ lpCS->lock(); lpcs = lpCS; mIsLocked = TRUE; }
		__except(1){ lpcs = NULL; mIsLocked = FALSE; }
	}
	~CLockHandle()    { if( mIsLocked && lpcs ) { lpcs->unlock(); mIsLocked = FALSE; } }
	void Unlock()     { if( mIsLocked && lpcs ) { lpcs->unlock(); mIsLocked = FALSE; } }
	void Lock()       { if( !mIsLocked && lpcs ){ lpcs->lock(); mIsLocked = TRUE;  } }
	BOOL               mIsLocked;
	WTF::Mutex*     lpcs;
};


template <class _Type>
class CxQueue{
public:
	~CxQueue()                      { Clear(); }
	_Type* InsertTail(_Type* pData) { CLockHandle lock(&m_Lock);m_Qs.InsertTail(&pData->node); return pData; }
	ULONG  GetCount()               { CLockHandle lock(&m_Lock);return m_Qs.Count(); }
	ULONG  GetTopCount(ULONG nTop)  { CLockHandle lock(&m_Lock);return m_Qs.GetTopCount(nTop);  }
	BOOL   IsEmpty()                { CLockHandle lock(&m_Lock);return m_Qs.FindFirst() == 0; }
	void   Clear(){
		_Type* q;
		//CLockHandle lock(&m_Lock);
		while( (q = GetWithRemoveFromHead()) != NULL){delete q;}
	}
	_Type* GetWithRemoveFromHead(){
		CLockHandle lock(&m_Lock);
		LPDList q = m_Qs.FindFirst();
		if( !q )return 0;
		DListRemove( q );
		return CONTAINING_RECORD(q,_Type,node);
	}
private:
	Mutex     m_Lock;
	CDList m_Qs;
};

struct WS_SendInfo{
	DList node;
	char*    data;
	int      len;
	WS_SendInfo( const char* buf, int buf_len)    {  data = new char[buf_len+1]; memcpy( data, buf, buf_len ); len = buf_len; data[buf_len] = 0; }
	~WS_SendInfo()                                { SafeDeleteArray( data ); }
};
typedef CxQueue<WS_SendInfo> CSendInfo;

struct SocketInfo{
	WebCore::SocketStreamHandle* ws;
	int                          reason;
	ULONG                        HostIp;
	USHORT                       wPort;
	WsConnectState               wsState;
	CSendInfo                    mQueue;
	char                         data[1024];
	int                          dataLen;
	SocketInfo( WebCore::SocketStreamHandle* p ){ ws = p; }
};

namespace WebCore {

void MainThreadCallForWebSocket( void* invocation ){
	SocketInfo* info = ((SocketInfo*)invocation);
	switch( info->reason ){
	case ReasonForOnconnect:
		if( info->wsState != WsStateConnenting ){ return; }
		info->wsState = WsStateConnented;
		info->ws->SetState_Opend();
		info->ws->client()->didOpenSocketStream( info->ws );
		break;
	case ReasonForReceivedData:
		if( info->wsState != WsStateConnented ){ return; }
		info->ws->client()->didReceiveSocketStreamData( info->ws, info->data, info->dataLen );
		break;
	case ReasonForDisconnect:
		if( info->wsState != WsStateConnented ){ return; }
		info->wsState = WsStateDisconnected;
		info->ws->SetState_Closed();
		info->ws->client()->didCloseSocketStream( info->ws );
		break;
	}
}

__inline BOOL  SetSocketNonBlock(SOCKET s,ULONG isTrue){
	return 0 == ioctlsocket(s, FIONBIO,&isTrue);
}


void FillSockAddr(void* p_sockaddr_in,ULONG ip,USHORT port,SHORT ifamily){
	sockaddr_in* remote=(sockaddr_in*)p_sockaddr_in;
	memset(remote,0,sizeof(sockaddr_in));
	remote->sin_family = ifamily;
	remote->sin_port = htons(port);
	remote->sin_addr.S_un.S_addr = ip;
	//remote->sin_addr.s_addr = ip;
}

void s_CloseSocket(SOCKET &s){
	if(SocketIsBad(s))return;
	LINGER lingerStruct={1,0};
	setsockopt(s,SOL_SOCKET, SO_LINGER,(char*)&lingerStruct,sizeof(lingerStruct));
	closesocket( s );
	s = INVALID_SOCKET;
}

BOOL socket_select( SOCKET s, BOOL isRead, LONG nTimeOut ){
	fd_set  fd;
	timeval timeout;
	FD_ZERO(&fd);
	FD_SET((ULONG)s, &fd);
	timeout.tv_sec = 0;  //秒
	timeout.tv_usec = nTimeOut * 1000;          // 1/1000秒 换算为 1/1000000秒 
	return ( isRead ) ? 1 == select( 0, &fd, NULL, NULL, &timeout ) : 1 == select( 0, NULL, &fd, NULL, &timeout );
}

__forceinline ULONG GetSocketRecvBytes( SOCKET sk ){
	ULONG uCount;
	return 0 == ioctlsocket( sk , FIONREAD , &uCount ) ? uCount : 0;
}

SOCKET SocketConnect(DWORD ip,WORD wPort,DWORD ipLocal,LONG iTimeOut){
	SOCKET sockfd = INVALID_SOCKET;
	do {

		if (IpAddrIsBad(ip))break;
		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if( sockfd ==  INVALID_SOCKET)break;

		sockaddr_in addr;
		FillSockAddr( &addr, ipLocal, htons(0), AF_INET);
		if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == INVALID_SOCKET)break;

		addr.sin_port = htons(wPort);
		addr.sin_addr.s_addr = ip;
		if(!SetSocketNonBlock(sockfd,true))break;//设置为非阻塞模式

		//Block模式在这里会返回SOCKET_ERROR,并且不会等待
		if(connect(sockfd, (const sockaddr*) &addr, sizeof(addr)) ==  INVALID_SOCKET){
			if( !IsSocketConnected( sockfd, iTimeOut ) ){
				break;// 在规定时间 connect 不成功
			}
		}
		if( !SetSocketNonBlock( sockfd, false ) )break;// 再设回阻塞模式
		return sockfd;
	} while(0);
	if(sockfd != INVALID_SOCKET)closesocket(sockfd);
	return INVALID_SOCKET;
}

ULONG GetIpaddr(const char *pszHost)
{
	hostent *lpHostent;
	__try
	{
		lpHostent = gethostbyname(pszHost);
		if(lpHostent==NULL)return 0;
		return  *(ULONG *)*(lpHostent->h_addr_list);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
    {
        return 0;
    }
}


unsigned __stdcall WebSocketThread( void* p ){
	SocketInfo* info = (SocketInfo*)p;
	//Sleep(100);

	SOCKET sock = SocketConnect( info->HostIp, info->wPort, INADDR_ANY, 5000 );

	// WTFReportError(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, "-------->WebSocket BEGIN %d.%d.%d.%d:%d SOCKET=%d", ipfmt(info->HostIp), info->wPort, sock );

	if( SocketIsOk(sock) ){
		info->reason = ReasonForOnconnect;
		callOnMainThreadAndWait( MainThreadCallForWebSocket, info );

		while( WsStateConnented == info->wsState && SocketIsOk(sock) ){
			ULONG bytes = GetSocketRecvBytes( sock );
			if( bytes ){
				// 有数据接收就优先接收
				memset( info->data, 0, sizeof(info->data) );
				if( bytes >= sizeof(info->data) ){
					bytes = ( sizeof(info->data) - 1 );
				}
				info->dataLen = recv( sock, info->data, bytes, 0 );
				if( info->dataLen > 0 ){
					info->data[info->dataLen] = 0;
					info->reason = ReasonForReceivedData;
					// WTFReportError(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, "-------->R %d.%s", info->dataLen, info->data );
					callOnMainThreadAndWait( MainThreadCallForWebSocket, info );
				}else{
					s_CloseSocket( sock );
				}
			}else{
				// 获取要发送的数据
				int iCount = 0;
				for( ;WsStateConnented == info->wsState; iCount++){
					WS_SendInfo* si = info->mQueue.GetWithRemoveFromHead();
					if( !si ){ break; }
					BOOL isSendOK = si->len == send( sock, si->data, si->len, 0 );
					// WTFReportError(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, "-------->S %d.%s SendOK=%d", si->len, si->data, isSendOK );
					delete si;
					if( !isSendOK ){
						s_CloseSocket( sock );
						break;
					}
				}
				if( !iCount ){
					Sleep(10);   // 没有接收和发送的数据就Sleep
				}
			}
		}
	}

	s_CloseSocket( sock );
	info->reason = ReasonForDisconnect;
	callOnMainThreadAndWait( MainThreadCallForWebSocket, info );

	// WTFReportError(__FILE__, __LINE__, WTF_PRETTY_FUNCTION, "-------->Websocket disconnected" );

	while( WsStateDestroying != info->wsState ){  Sleep(10); } // 等待关闭标志
	delete info;                                   // 删除Socket对象
	return 0;
}

ULONG SocketStreamHandle::GetHostIp(){
	return GetIpaddr( m_url.host().utf8().data() );
}

SocketStreamHandle::SocketStreamHandle(const KURL& url, SocketStreamHandleClient* client) : SocketStreamHandleBase(url, client), info(NULL) {
	info = new SocketInfo( this );
	info->HostIp = GetHostIp();
	info->wPort = info->ws->HostPort() ? info->ws->HostPort() : 80;
	info->wsState = WsStateConnenting;
	CloseHandle( (HANDLE)_beginthreadex( 0, 0, WebSocketThread, info, 0, 0 ) );
}

void SocketStreamHandle::platformClose(){
	if( info ){
		info->wsState = WsStateDestroying;
		SetState_Closed();
		info = NULL;
	}
}

SocketStreamHandle::~SocketStreamHandle(){
	platformClose();
}


int SocketStreamHandle::platformSend(const char* data, int dataLen ){
	if( info && info->wsState == WsStateConnented ){
		info->mQueue.InsertTail( new WS_SendInfo(data,dataLen) );
		return dataLen;
	}
	return 0;
}


void SocketStreamHandle::didReceiveAuthenticationChallenge(const AuthenticationChallenge&){
    notImplemented();
}

void SocketStreamHandle::receivedCredential(const AuthenticationChallenge&, const Credential&){
    notImplemented();
}

void SocketStreamHandle::receivedRequestToContinueWithoutCredential(const AuthenticationChallenge&){
    notImplemented();
}

void SocketStreamHandle::receivedCancellation(const AuthenticationChallenge&){
    notImplemented();
}

} // namespace WebCore
