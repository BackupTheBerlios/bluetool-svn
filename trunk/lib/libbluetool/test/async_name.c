 /*
  *
  *  Request the remote name of an incoming HCI connection
  *
  *  Copyright (C) 2003  Marcel Holtmann <marcel@ho...>
  *
  *
  *  This program is free software; you can redistribute it and/or modify
  *  it under the terms of the GNU General Public License as published by
  *  the Free Software Foundation; either version 2 of the License, or
  *  (at your option) any later version.
  *
  *  This program is distributed in the hope that it will be useful,
  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  *  GNU General Public License for more details.
  *
  *  You should have received a copy of the GNU General Public License
  *  along with this program; if not, write to the Free Software
  *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  *
  */
 
 #include <stdio.h>
 #include <errno.h>
 #include <unistd.h>
 #include <signal.h>
 #include <sys/poll.h>
 #include <sys/socket.h>
 
 #include <bluetooth/bluetooth.h>
 #include <bluetooth/hci.h>
 #include <bluetooth/hci_lib.h>
 
 
 static volatile sig_atomic_t __io_canceled = 0;
 
 static void sig_hup(int sig)
 {
 	return;
 }
 
 static void sig_term(int sig)
 {
 	__io_canceled = 1;
 }
 
 static void inquiry_result(int dd, unsigned char *buf, int len)
 {
 	inquiry_info *info;
 	uint8_t num;
 	char addr[18];
 	int i;
 
 	num = buf[0];
 	printf("inquiry_result:\tnum %d\n", num);
 
 	for (i = 0; i < num; i++) {
 		info = (void *) buf + (sizeof(*info) * i) + 1;
 		ba2str(&info->bdaddr, addr);
 		printf("inquiry_result:\tbdaddr %s\n", addr);
 	}
 }
 
 static void conn_request(int dd, unsigned char *buf, int len)
 {
 	evt_conn_request *evt = (void *) buf;
 	remote_name_req_cp cp;
 	char addr[18];
 
 	ba2str(&evt->bdaddr, addr);
 	printf("conn_request:\tbdaddr %s\n", addr);
 
 	memset(&cp, 0, sizeof(cp));
 	bacpy(&cp.bdaddr, &evt->bdaddr);
 	cp.pscan_rep_mode = 0x01;
 	cp.pscan_mode     = 0x00;
 	cp.clock_offset   = 0x0000;
 
 	//hci_send_cmd(dd, OGF_LINK_CTL, OCF_REMOTE_NAME_REQ,
 	//			REMOTE_NAME_REQ_CP_SIZE, &cp);
 }
 
 static void conn_complete(int dd, unsigned char *buf, int len)
 {
 	evt_conn_complete *evt = (void *) buf;
 	remote_name_req_cp cp;
 
 	printf("conn_complete:\tstatus 0x%02x\n", evt->status);
 
 	if (evt->status)
 		return;
 
 	memset(&cp, 0, sizeof(cp));
 	bacpy(&cp.bdaddr, &evt->bdaddr);
 	cp.pscan_rep_mode = 0x01;
 	cp.pscan_mode     = 0x00;
 	cp.clock_offset   = 0x0000;
 
 	hci_send_cmd(dd, OGF_LINK_CTL, OCF_REMOTE_NAME_REQ,
 				REMOTE_NAME_REQ_CP_SIZE, &cp);
 }
 
 static void name_complete(int dd, unsigned char *buf, int len)
 {
 	evt_remote_name_req_complete *evt = (void *) buf;
 	char addr[18];
 
 	printf("name_complete:\tstatus 0x%02x\n", evt->status);
 
 	if (evt->status)
 		return;
 
 	ba2str(&evt->bdaddr, addr);
 	printf("name_complete:\tbdaddr %s %s\n", addr, evt->name);
 }
 
 int main(int argc, char *argv[])
 {
 	unsigned char buf[HCI_MAX_EVENT_SIZE], *ptr;
 	hci_event_hdr *hdr;
 	struct hci_filter flt;
 	struct sigaction sa;
 	struct pollfd p;
 	int dd, dev = 0, len;
 
 	dd = hci_open_dev(dev);
 	if (dd < 0) {
 		perror("Can"t open HCI device");
 		exit(1);
 	}
 
 	hci_filter_clear(&flt);
 	hci_filter_set_ptype(HCI_EVENT_PKT, &flt);
 	hci_filter_set_event(EVT_INQUIRY_RESULT, &flt);
 	hci_filter_set_event(EVT_CONN_REQUEST, &flt);
 	hci_filter_set_event(EVT_CONN_COMPLETE, &flt);
 	hci_filter_set_event(EVT_REMOTE_NAME_REQ_COMPLETE, &flt);
 	if (setsockopt(dd, SOL_HCI, HCI_FILTER, &flt, sizeof(flt)) < 0) {
 		perror("Can"t set HCI filter");
 		exit(1);
 	}
 
 	memset(&sa, 0, sizeof(sa));
 	sa.sa_flags   = SA_NOCLDSTOP;
 	sa.sa_handler = SIG_IGN;
 	sigaction(SIGCHLD, &sa, NULL);
 	sigaction(SIGPIPE, &sa, NULL);
 
 	sa.sa_handler = sig_term;
 	sigaction(SIGTERM, &sa, NULL);
 	sigaction(SIGINT,  &sa, NULL);
 
 	sa.sa_handler = sig_hup;
 	sigaction(SIGHUP, &sa, NULL);
 
 	p.fd = dd;
 	p.events = POLLIN | POLLERR | POLLHUP;
 
 	while (!__io_canceled) {
 		p.revents = 0;
 		if (poll(&p, 1, 100) > 0) {
 			len = read(dd, buf, sizeof(buf));
 			if (len < 0)
 				continue;
 
 			hdr = (void *) (buf + 1);
 			ptr = buf + (1 + HCI_EVENT_HDR_SIZE);
 			len -= (1 + HCI_EVENT_HDR_SIZE);
 
 			switch (hdr->evt) {
 			case EVT_INQUIRY_RESULT:
 				inquiry_result(dd, ptr, len);
 				break;
 			case EVT_CONN_REQUEST:
 				conn_request(dd, ptr, len);
 				break;
 			case EVT_CONN_COMPLETE:
 				conn_complete(dd, ptr, len);
 				break;
 			case EVT_REMOTE_NAME_REQ_COMPLETE:
 				name_complete(dd, ptr, len);
 				break;
 			}
 		}
 	}
 
 	if (hci_close_dev(dd) < 0) {
 		perror("Can't close HCI device");
 		exit(1);
 	}
 
 	return 0;
 }
