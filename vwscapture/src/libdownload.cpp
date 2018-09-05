#include "vwscapture/libdownload.h"

HTTPDownload::HTTPDownload()
{
    timeval _tv{2,0};
    tv = _tv;
}

vector<string> HTTPDownload::dns_lookup(const string &host_name, int ipv) //ipv: default=4
{
    vector<string> output;

    struct addrinfo hints, *res, *p;
    int status, ai_family;
    char ip_address[INET6_ADDRSTRLEN];
//    char *ip_address = (char*)malloc(INET6_ADDRSTRLEN * sizeof(char));

    ai_family = ipv==6 ? AF_INET6 : AF_INET; //v4 vs v6?
    ai_family = ipv==0 ? AF_UNSPEC : ai_family; // AF_UNSPEC (any), or chosen
    memset(&hints, 0, sizeof hints);
    hints.ai_family = ai_family;
    hints.ai_socktype = SOCK_STREAM;

    if ((status = getaddrinfo(host_name.c_str(), NULL, &hints, &res)) != 0) {
        //cerr << "getaddrinfo: "<< gai_strerror(status) << endl;
        return output;
    }

    //cout << "DNS Lookup: " << host_name << " ipv:" << ipv << endl;

    for(p = res;p != NULL; p = p->ai_next) {
        void *addr;
        if (p->ai_family == AF_INET) { // IPv4
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
            addr = &(ipv4->sin_addr);
        } else { // IPv6
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
            addr = &(ipv6->sin6_addr);
        }

        // convert the IP to a string
        inet_ntop(p->ai_family, addr, ip_address, sizeof ip_address);
        output.push_back(ip_address);
    }

    freeaddrinfo(res); // free the linked list

    return output;
}

bool HTTPDownload::is_ipv6_address(const string& str)
{
    struct sockaddr_in6 sa;
    return inet_pton(AF_INET6, str.c_str(), &(sa.sin6_addr))!=0;
}

bool HTTPDownload::is_ipv4_address(const string& str)
{
    struct sockaddr_in sa;
    return inet_pton(AF_INET, str.c_str(), &(sa.sin_addr))!=0;
}

int HTTPDownload::socket_connect(string ip_address, int port)
{
    int err=-1,sd=-1;
    struct sockaddr_in sa;

    memset (&sa, '\0', sizeof(sa));
    sa.sin_family      = AF_INET;
    sa.sin_addr.s_addr = inet_addr (ip_address.c_str());   /* Server IP */
    sa.sin_port        = htons     (port);   /* Server Port number */

    sd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sd)
    {
        err = ::connect(sd, (struct sockaddr*) &sa, sizeof(sa));
    }
    if (err!=-1)//success
    {
        return sd;
    }
    cout << "socket connect error: " << endl;
    return -1;
    //if (errno==EINPROGRESS) { return 0; }//errno is a global set by connect
}

ws_download_infor HTTPDownload::parse_dl_header(string header)
{

    ws_download_infor hd_info;
    size_t start =  header.find_first_of("//");
    string sub_str = header.substr(start+2,header.size()-1);

    start =  sub_str.find_first_of("/");

    hd_info.dl_host = sub_str.substr(0,start);

    hd_info.dl_get = sub_str.substr(start+1,sub_str.size()-1);

    hd_info.dl_request << "GET /" << hd_info.dl_get << " HTTP/1.1\r\n";
    hd_info.dl_request << "Host: " <<  hd_info.dl_host << "\r\n\r\n";

    return hd_info;
}

int HTTPDownload::download(const string& url, vector<char>& buffer)
{
    ws_download_infor hd_infor =  parse_dl_header(url);
    int ipv;
    vector<string> ip_addresses;
    if (hd_infor.dl_host.length()>0 && !is_ipv6_address(hd_infor.dl_host))
    {
        if (is_ipv4_address(hd_infor.dl_host))
        {
            ip_addresses.push_back(hd_infor.dl_host);
        }
        else //if (!is_ipv4_address(domain))
        {
            ip_addresses = dns_lookup(hd_infor.dl_host, ipv=4);
        }
    }
    return http_get(hd_infor.dl_request.str(), ip_addresses[0], 80, buffer);
}

string HTTPDownload::header_value(const string& full_header, const string& header_name)
{
    size_t pos = full_header.find(header_name);//case sensitive but probably shouldn't be
    string r;
    if (pos!=string::npos)
    {
        size_t begin = full_header.find_first_not_of(": ", pos + header_name.length());
        size_t until = full_header.find_first_of("\r\n\t ", begin + 1);
        if (begin!=string::npos && until!=string::npos)
        {
            r = full_header.substr(begin,until-begin);
        }
    }
    return r;
}

int HTTPDownload::http_get(const string& request, const string& ip_address, int port, vector<char>& bufferIn)
{
    stringstream header;
    char delim[] = "\r\n\r\n";
    char *buffer = (char*)malloc(16384 * sizeof(char));
    bufferIn.clear();
    int sd, bytes_received=-3,bytes_sofar=0,bytes_expected=-1,i,state=0;

    struct timeval t0;
    gettimeofday(&t0, NULL);
    if ((sd = socket_connect(ip_address, port))) {
        ::send(sd, request.c_str(), request.length(), 0);
        setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
        while (bytes_sofar!=bytes_expected && (bytes_received = ::recv(sd, buffer, sizeof(buffer), 0))>0)
        {
            if (state<(int)sizeof(delim)-1)//read header
            {
                for(i=0; i<bytes_received && state<(int)sizeof(delim)-1; i++)
                {
                    header << buffer[i];
                    state = buffer[i]==delim[state] ? state+1 : 0;
                }
                bytes_received = state==sizeof(delim)-1 ? bytes_received-i : bytes_received;
            }
            if (bytes_expected==-1 && state==sizeof(delim)-1)//parse header
            {
                bytes_expected=-2;
                string h = header.str();
                stringstream(header_value(h, "Content-Length"))>>bytes_expected;
            }
            if (state==sizeof(delim)-1)//read body
            {
                bytes_sofar+=bytes_received;
                bufferIn.insert( bufferIn.end(), buffer+i, buffer+ bytes_received+i);
                i=0;
            }
        }

        close(sd);
    }
    struct timeval t1;
    gettimeofday(&t1, NULL);

    free(buffer);
    return bytes_sofar;
}
