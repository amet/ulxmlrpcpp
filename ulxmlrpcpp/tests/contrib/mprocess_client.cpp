/***************************************************************************
        mprocess_client.cpp.cpp  -  multi process rpc client test
                             -------------------
    begin                : Sun May 29 2005
    copyright            : (C) 2005 funtik <funt@alarit.com>

    $Id: mprocess_client.cpp 10942 2011-09-13 14:35:52Z korosteleva $

 ***************************************************************************/

/**************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 ***************************************************************************/

#include <ulxmlrpcpp/ulxmlrpcpp.h>

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <memory>
#include <iostream>
#include <fstream>

#include <ulxmlrpcpp/ulxr_tcpip_connection.h>  // first, don't move: msvc #include bug
#include <ulxmlrpcpp/ulxr_ssl_connection.h>
#include <ulxmlrpcpp/ulxr_http_protocol.h>
#include <ulxmlrpcpp/ulxr_requester.h>
#include <ulxmlrpcpp/ulxr_value.h>
#include <ulxmlrpcpp/ulxr_except.h>

#include <sys/types.h>
#include <sys/wait.h>


#define NUM_QUERY	5
#define NUM_CYCLES	100

int main(int argc, char **argv)
{
	try
	{
    	std::string host = "localhost";
		if (argc > 1)
			host = argv[1];

		unsigned port = 32000;
		if (argc > 2)
			port = atoi(argv[2]);

		unsigned int uiNumQuery=argc>3?atoi(argv[3]):NUM_QUERY;
		unsigned int uiNumCycles=argc>4?atoi(argv[4]):NUM_CYCLES;

		std::cout<<"Test Cycles: "<<uiNumCycles<<" Number query in cycle: "<<uiNumQuery<<std::endl;

	    std::auto_ptr<ulxr::TcpIpConnection> conn(new ulxr::TcpIpConnection (false, host, port));

    	ulxr::HttpProtocol prot(conn.get());
	    ulxr::Requester client(&prot);

    	ulxr::MethodCall list_methods ("system.listMethods");
	    std::cout << "call list_methods: \n";
    	ulxr::MethodResponse resp = client.call(list_methods, "/RPC2");
	    std::cout << "call result: \n";
    	std::cout << resp.getXml(0) << std::endl;

//--
	    ulxr::MethodCall get_signature ("system.methodSignature");
    	std::cout << "call system.methodSignature: \n";
	    get_signature.addParam(ulxr::RpcString("check_sig"));
    	ulxr::MethodResponse resp_get_sig = client.call(get_signature, "/RPC2");
	    std::cout << "call result: \n";
    	std::cout << resp_get_sig.getXml(0) << std::endl;

//    exit(0);
//--
	    ulxr::MethodCall check ("check");
		std::string ss = "Test string";
		ulxr::RpcString c(ss);
		check.addParam(c);
//    check.addParam(ulxr::RpcString("Test string."));
		for(unsigned int j=0;j<uiNumCycles;j++)
		{
    		for(unsigned int i=0;i<uiNumQuery;i++)
    		{
				if(!fork())
				{
					try{
		    			ulxr::MethodResponse test_resp = client.call(check, "/RPC2");
			    		std::cout << "Number request: "<<i<<" All "<<(j*uiNumQuery)+i<<std::endl;
				    	ulxr::RpcString ret = test_resp.getResult();
			    		std::cout <<ret.getString()<<std::endl;
			    		exit(0);
					}catch(std::exception &ex)
					{
						std::cout<<"Children Error: "<<ex.what()<<std::endl;
						exit(0);
					}
				}
	    	}

    		int status;
	    	pid_t exit_pid;
	    	while((exit_pid=wait(&status))>0)
    		{
				std::cout<<"End process: "<<exit_pid<<std::endl;
		    }
		}

	    ulxr::MethodCall finish_server ("finish_server");
    	resp = client.call(finish_server, "/RPC2");
	    std::cout << "call result: \n";
    	std::cout << resp.getXml(0) << std::endl;

	    int status;
    	pid_t exit_pid;
	    while((exit_pid=wait(&status))>0)
    	{
			std::cout<<"End process: "<<exit_pid<<std::endl;
	    }
	}//try

  catch(ulxr::Exception &ex)
  {
     std::cout << "Error occured: " << ex.why() << std::endl;
     return 1;
  }

  catch(...)
  {
     std::cout << "unknown Error occured.\n";
     return 1;
  }

  std::cout << "Well done, Ready.\n";
  return 0;
}
