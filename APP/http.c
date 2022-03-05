#include "malloc.h"
#include "string.h"
#include "4g.h"
#include "rtc.h"
#include "http.h"
#include "ussend.h"

u8 SETHTTP(void)
{
	if(enterAT()==0)
		{
			send_cmd("AT+WKMOD=HTTPD", "OK",100,1,0, NULL);
			send_cmd("AT+HTPTP=POST", "OK",100,1,0, NULL);
			send_cmd("AT+HTPURL=/demo/breath-file/testReceiveFile", "OK",100,1,0, NULL);
			send_cmd("AT+HTPSV=47.105.175.47,8085", "OK",100,1,0, NULL);
			send_cmd("AT+HTPHD=Content-Type: multipart/form-data[3B] boundary=----WebKitFormBoundary7MA4YWxkTrZu0gW[0D][0A]", "OK",100,1,0, NULL);
			send_cmd("AT+HTPTO=10", "OK",100,1,0, NULL);
			send_cmd("AT+HTPFLT=ON", "OK",100,1,0, NULL);
			send_cmd("AT+Z", "OK",100,1,0, NULL);
			quitAT();
			return 0;
		}
	else return 1;
}

u8 SETtanspar(void)
{
	if(enterAT()==0)
		{
			send_cmd("AT+WKMOD=NET", "OK",100,1,0, NULL);
			send_cmd("AT+SOCKAEN=ON", "OK",100,1, 0,NULL);
			send_cmd("AT+SOCKASL=LONG", "OK",100,1, 0,NULL);
			send_cmd("AT+SOCKA=TCP,47.105.175.47,8085", "OK",100,1, 0,NULL);
			send_cmd("AT+Z", "OK",100,1, 0,NULL);
			quitAT();
			return 0;
		}
	else return 1;
}

u8 ESCtanspar(void)
{
	if(enterATT()==0)
		{
			send_cmd("AT+SOCKAEN=OFF", "OK",200,1, 0,NULL);
			send_cmd("AT+Z", "OK",100,1, 0,NULL);
			quitAT();
			return 0;
		}
	else return 1;
}
u8 ONtanspar(void)
{
	if(enterATT()==0)
		{
			send_cmd("AT+SOCKAEN=ON", "OK",200,1, 0,NULL);
			send_cmd("AT+Z", "OK",100,1, 0,NULL);
			quitAT();
			return 0;
		}
	else return 1;
}


void datatocloud(void)
{
}
