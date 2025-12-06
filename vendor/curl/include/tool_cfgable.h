#ifndef HEADER_CURL_TOOL_CFGABLE_H
#define HEADER_CURL_TOOL_CFGABLE_H
/***************************************************************************
 *                                  _   _ ____  _
 *  Project                     ___| | | |  _ \| |
 *                             / __| | | | |_) | |
 *                            | (__| |_| |  _ <| |___
 *                             \___|\___/|_| \_\_____|
 *
 * Copyright (C) 1998 - 2012, Daniel Stenberg, <daniel@haxx.se>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution. The terms
 * are also available at http://curl.haxx.se/docs/copyright.html.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
#include "tool_setup.h"

#include "tool_sdecls.h"

#include "tool_metalink.h"

struct Configurable {
  CURL *easy;               /* once we have one, we keep it here */
  int remote_time;
  char *random_file;
  char *egd_file;
  char *useragent;
  char *cookie;             /* single line with specified cookies */
  char *cookiejar;          /* write to this file */
  char *cookiefile;         /* read from this file */
  int cookiesession;       /* new session? */
  int encoding;            /* Accept-Encoding please */
  int tr_encoding;         /* Transfer-Encoding please */
  unsigned long authtype;   /* auth bitmask */
  int use_resume;
  int resume_from_current;
  int disable_epsv;
  int disable_eprt;
  int ftp_pret;
  long proto;
  int proto_present;
  long proto_redir;
  int proto_redir_present;
  curl_off_t resume_from;
  char *postfields;
  curl_off_t postfieldsize;
  char *referer;
  long timeout;
  long connecttimeout;
  long maxredirs;
  curl_off_t max_filesize;
  char *headerfile;
  char *ftpport;
  char *iface;
  int localport;
  int localportrange;
  unsigned short porttouse;
  char *range;
  long low_speed_limit;
  long low_speed_time;
  int showerror; /* -1 == unset, default => show errors
                    0 => -s is used to NOT show errors
                    1 => -S has been used to show errors */
  char *userpwd;
  char *tls_username;
  char *tls_password;
  char *tls_authtype;
  char *proxyuserpwd;
  char *proxy;
  int proxyver;             /* set to CURLPROXY_HTTP* define */
  char *noproxy;
  char *mail_from;
  struct curl_slist *mail_rcpt;
  char *mail_auth;
  int sasl_ir;             /* Enable/disable SASL initial response */
  int proxytunnel;
  int ftp_append;          /* APPE on ftp */
  int mute;                /* don't show messages, --silent given */
  int use_ascii;           /* select ascii or text transfer */
  int autoreferer;         /* automatically set referer */
  int failonerror;         /* fail on (HTTP) errors */
  int include_headers;     /* send headers to data output */
  int no_body;             /* don't get the body */
  int dirlistonly;         /* only get the FTP dir list */
  int followlocation;      /* follow http redirects */
  int unrestricted_auth;   /* Continue to send authentication (user+password)
                               when following ocations, even when hostname
                               changed */
  int netrc_opt;
  int netrc;
  char *netrc_file;
  int noprogress;          /* don't show progress meter, --silent given */
  int isatty;              /* updated internally only if output is a tty */
  struct getout *url_list;  /* point to the first node */
  struct getout *url_last;  /* point to the last/current node */
  struct getout *url_get;   /* point to the node to fill in URL */
  struct getout *url_out;   /* point to the node to fill in outfile */
  char *cipher_list;
  char *cert;
  char *cert_type;
  char *cacert;
  char *capath;
  char *crlfile;
  char *key;
  char *key_type;
  char *key_passwd;
  char *pubkey;
  char *hostpubmd5;
  char *engine;
  int list_engines;
  int crlf;
  char *customrequest;
  char *krblevel;
  char *trace_dump;         /* file to dump the network trace to, or NULL */
  FILE *trace_stream;
  int trace_fopened;
  trace tracetype;
  int tracetime;           /* include timestamp? */
  long httpversion;
  int progressmode;         /* CURL_PROGRESS_BAR or CURL_PROGRESS_STATS */
  int nobuffer;
  int readbusy;            /* set when reading input returns EAGAIN */
  int globoff;
  int use_httpget;
  int insecure_ok;         /* set TRUE to allow insecure SSL connects */
  int create_dirs;
  int ftp_create_dirs;
  int ftp_skip_ip;
  int proxynegotiate;
  int proxyntlm;
  int proxydigest;
  int proxybasic;
  int proxyanyauth;
  char *writeout;           /* %-styled format string to output */
  int writeenv;            /* write results to environment, if available */
  FILE *errors;             /* errors stream, defaults to stderr */
  int errors_fopened;      /* whether errors stream isn't stderr */
  struct curl_slist *quote;
  struct curl_slist *postquote;
  struct curl_slist *prequote;
  long ssl_version;
  long ip_version;
  curl_TimeCond timecond;
  time_t condtime;
  struct curl_slist *headers;
  struct curl_httppost *httppost;
  struct curl_httppost *last_post;
  struct curl_slist *telnet_options;
  struct curl_slist *resolve;
  HttpReq httpreq;

  /* for bandwidth limiting features: */
  curl_off_t sendpersecond; /* send to peer */
  curl_off_t recvpersecond; /* receive from peer */

  int ftp_ssl;
  int ftp_ssl_reqd;
  int ftp_ssl_control;
  int ftp_ssl_ccc;
  int ftp_ssl_ccc_mode;

  char *socksproxy;         /* set to server string */
  int socksver;             /* set to CURLPROXY_SOCKS* define */
  char *socks5_gssapi_service;  /* set service name for gssapi principal
                                 * default rcmd */
  int socks5_gssapi_nec ;   /* The NEC reference server does not protect
                             * the encryption type exchange */

  int tcp_nodelay;
  long req_retry;           /* number of retries */
  long retry_delay;         /* delay between retries (in seconds) */
  long retry_maxtime;       /* maximum time to keep retrying */

  char *ftp_account;        /* for ACCT */
  char *ftp_alternative_to_user;  /* send command if USER/PASS fails */
  int ftp_filemethod;
  long tftp_blksize;        /* TFTP BLKSIZE option */
  int ignorecl;            /* --ignore-content-length */
  int disable_sessionid;

  char *libcurl;            /* output libcurl code to this file name */
  int raw;
  int post301;
  int post302;
  int post303;
  int nokeepalive;         /* for keepalive needs */
  long alivetime;
  int content_disposition; /* use Content-disposition filename */

  int default_node_flags;   /* default flags to search for each 'node', which
                               is basically each given URL to transfer */

  int xattr;               /* store metadata in extended attributes */
  long gssapi_delegation;
  int ssl_allow_beast;     /* allow this SSL vulnerability */

  int use_metalink;        /* process given URLs as metalink XML file */
  metalinkfile *metalinkfile_list; /* point to the first node */
  metalinkfile *metalinkfile_last; /* point to the last/current node */
}; /* struct Configurable */

void free_config_fields(struct Configurable *config);

#endif /* HEADER_CURL_TOOL_CFGABLE_H */

