#include	"mongoose.h"
#include	<dirent.h>
#include	<iostream>
#include	<stdint.h>
#include	<fstream>
#include	<sstream>
#include	<vector>
using	namespace	std;
static struct mg_serve_http_opts s_http_server_opts;
vector<float>	w(250*7);

static void ev_handler(struct mg_connection *nc, int ev, void *p) {
	if (ev != MG_EV_HTTP_REQUEST)	return;
	struct http_message	*hm=(struct http_message *) p;
	vector<char>	buffer;	buffer.resize(0x100000);
	vector<int>	answer(50);	bool	ok=true;
	for(int	i=0;	i<50;	i++){
		char	q[256];	sprintf(q,	"A%d",	i);
		if(mg_get_http_var(&hm->body,	q, buffer.data(),	0x100000)>0)	answer[i]=atoi(buffer.data());
		else	ok=false;
	}	
	if(ok){
		double	sum[5]={};
		for(int	i=0;	i<50;	i++){
			for(size_t	j=0;	j<5;	j++){
				double	v=(answer[i]==j);
				size_t	x=i*5+j;
				v=(v-w[x*7])/w[x*7+1];
				for(size_t	k=0;	k<5;	k++)	sum[k]+=v*w[x*7+2+k];
			}
		}
		ostringstream	so;	so.precision(0);	so.setf(ios::fixed);
		so<<"<html>";
		so<<"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">";
		so<<"<h3>复旦大学大五类人格测试结果</h3>\n";
		so<<"<table border=\"1\">\n";
		so<<"<tr><td>开放性</td><td>"<<100/(1+exp(sum[0]))<<"</td>\n";
		so<<"<tr><td>外倾性</td><td>"<<100/(1+exp(-sum[1]))<<"</td>\n";
		so<<"<tr><td>责任心</td><td>"<<100/(1+exp(sum[2]))<<"</td>\n";
		so<<"<tr><td>神经质</td><td>"<<100/(1+exp(-sum[3]))<<"</td>\n";
		so<<"<tr><td>宜人性</td><td>"<<100/(1+exp(sum[4]))<<"</td>\n";
		so<<"</table></html>\n";
		mg_send_head(nc, 200, so.str().size(), "");
		mg_printf(nc,	"%s",	so.str().c_str());
	}
	mg_serve_http(nc, (struct http_message *) p, s_http_server_opts);
}

int main(int	ac,	char	**av) {
  if(ac!=2){	cerr<<"bigfive port\n";	return	0;	}
  ifstream	fi("weights.txt");
  string	line;
  getline(fi,line);
  for(size_t	i=0;	i<w.size();	i++)	fi>>w[i];
  fi.close();
  struct mg_mgr mgr;
  struct mg_connection *nc;
  mg_mgr_init(&mgr, NULL);
  nc = mg_bind(&mgr, av[1], ev_handler);
  mg_set_protocol_http_websocket(nc);
  s_http_server_opts.document_root = "files";
  s_http_server_opts.enable_directory_listing = "yes";
  for(;;)	mg_mgr_poll(&mgr, 1000);
  mg_mgr_free(&mgr);
  return 0;
}
