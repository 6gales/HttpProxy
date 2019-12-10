//#include "EstablishedConnection.h"
//#include <cstring>
//
//void EstablishedConnection::eventTriggeredCallback(short events)
//{
//	if (canRead() && !buffer.isFull() && !buffer.eof)
//	{
//		ssize_t bytesRead = recv(sockFd, buffer.getEmptyData(), buffer.countEmpty(), 0);
//		if (bytesRead < 0)
//		{
//			throw std::runtime_error(std::string("recv: ") + strerror(errno));
//		}
//		if (0 == bytesRead)
//		{
//			buffer.eof = true;
//		}
//		else
//		{
//			buffer.addRead(bytesRead);
//		}
//	}
//	if (d("full write", d("is write to:", context->getRegistrar().isSetWrite(toFd)) && buffer.hasData()))
//	{
//		ssize_t bytesWrote = send(toFd, buffer.getUnread(), buffer.countUnread(), 0);
//		if (bytesWrote < 0 && errno != EWOULDBLOCK)
//		{
//			throw std::runtime_error(std::string("send: ") + strerror(errno));
//		}
//		if (bytesWrote > 0)
//			buffer.addWrote(bytesWrote);
//
//		if (buffer.isFull() && !buffer.hasData())
//		{
//			buffer.reset();
//		}
//	}
//
//	if (clientBuffer.isFinished() && servBuffer.isFinished())
//	{
//		finished = true;
//	}
//	else
//	{
//		context->getRegistrar().registerFd(clientSock, FdRegistrar::Options::Both);
//		context->getRegistrar().registerFd(servSock, FdRegistrar::Options::Both);
//	}
//
//	return nullptr;
//}