/*
 * iperf, Copyright (c) 2014, 2016, 2017, The Regents of the University of
 * California, through Lawrence Berkeley National Laboratory (subject
 * to receipt of any required approvals from the U.S. Dept. of
 * Energy).  All rights reserved.
 *
 * If you have questions about your rights to use or distribute this
 * software, please contact Berkeley Lab's Technology Transfer
 * Department at TTD@lbl.gov.
 *
 * NOTICE.  This software is owned by the U.S. Department of Energy.
 * As such, the U.S. Government has been granted for itself and others
 * acting on its behalf a paid-up, nonexclusive, irrevocable,
 * worldwide license in the Software to reproduce, prepare derivative
 * works, and perform publicly and display publicly.  Beginning five
 * (5) years after the date permission to assert copyright is obtained
 * from the U.S. Department of Energy, and subject to any subsequent
 * five (5) year renewals, the U.S. Government is granted for itself
 * and others acting on its behalf a paid-up, nonexclusive,
 * irrevocable, worldwide license in the Software to reproduce,
 * prepare derivative works, distribute copies to the public, perform
 * publicly and display publicly, and to permit others to do so.
 *
 * This code is distributed under a BSD style license, see the LICENSE
 * file for complete information.
 */
#include <errno.h>
#include <assert.h>
#ifdef HAVE_STDINT_H
#include <stdint.h>
#endif
#include <sys/time.h>

#include "lwipconf.h" //realtek add
#include "platform_stdlib.h"

#include "iperf.h"
#include "iperf_api.h"
#include "iperf_util.h"
#include "iperf_udp.h"
#include "timer.h"
#include "net.h"
#include <cJSON.h>
#include "portable_endian.h"

/* iperf_udp_recv
 *
 * receives the data for UDP
 */
int
iperf_udp_recv(struct iperf_stream *sp)
{
	//IPERF3_DBG("\n%s: iperf_udp_recv \n",__FUNCTION__);

	uint32_t  sec, usec;
	uint64_t  pcount;
	int       r;
	int       size = sp->settings->blksize;
	int       first_packet = 0;
	double    transit = 0, d = 0;
	struct timeval sent_time, arrival_time;

	r = Nread(sp->socket, sp->buffer, size, Pudp);

	/*
	 * If we got an error in the read, or if we didn't read anything
	 * because the underlying read(2) got a EAGAIN, then skip packet
	 * processing.
	 */
	if (r <= 0) {
		IPERF3_DBG("iperf_udp_recv error ,r:%d\n", r);
		return r;
	}

	/* Only count bytes received while we're in the correct state. */
	if (sp->test->state == TEST_RUNNING) {

		/*
		 * For jitter computation below, it's important to know if this
		 * packet is the first packet received.
		 */
		if (sp->result->bytes_received == 0) {
			first_packet = 1;
		}

		sp->result->bytes_received += r;
		sp->result->bytes_received_this_interval += r;

		if (sp->test->udp_counters_64bit) {
			memcpy(&sec, sp->buffer, sizeof(sec));
			memcpy(&usec, sp->buffer + 4, sizeof(usec));
			memcpy(&pcount, sp->buffer + 8, sizeof(pcount));
			sec = ntohl(sec);
			usec = ntohl(usec);
			pcount = be64toh(pcount);
			sent_time.tv_sec = sec;
			sent_time.tv_usec = usec;
		} else {
			uint32_t pc;
			memcpy(&sec, sp->buffer, sizeof(sec));
			memcpy(&usec, sp->buffer + 4, sizeof(usec));
			memcpy(&pc, sp->buffer + 8, sizeof(pc));
			sec = ntohl(sec);
			usec = ntohl(usec);
			pcount = ntohl(pc);
			sent_time.tv_sec = sec;
			sent_time.tv_usec = usec;
		}

		if (sp->test->debug) {
			fprintf(stderr, "pcount %llu packet_count %d\n", pcount, sp->packet_count);
		}

		/*
		 * Try to handle out of order packets.  The way we do this
		 * uses a constant amount of storage but might not be
		 * correct in all cases.  In particular we seem to have the
		 * assumption that packets can't be duplicated in the network,
		 * because duplicate packets will possibly cause some problems here.
		 *
		 * First figure out if the sequence numbers are going forward.
		 * Note that pcount is the sequence number read from the packet,
		 * and sp->packet_count is the highest sequence number seen so
		 * far (so we're expecting to see the packet with sequence number
		 * sp->packet_count + 1 arrive next).
		 */
		if (pcount >= (uint64_t)(sp->packet_count + 1)) {

			/* Forward, but is there a gap in sequence numbers? */
			if (pcount > (uint64_t)(sp->packet_count + 1)) {
				/* There's a gap so count that as a loss. */
				sp->cnt_error += (pcount - 1) - sp->packet_count;
			}
			/* Update the highest sequence number seen so far. */
			sp->packet_count = pcount;
		} else {

			/*
			 * Sequence number went backward (or was stationary?!?).
			 * This counts as an out-of-order packet.
			 */
			sp->outoforder_packets++;

			/*
			 * If we have lost packets, then the fact that we are now
			 * seeing an out-of-order packet offsets a prior sequence
			 * number gap that was counted as a loss.  So we can take
			 * away a loss.
			 */
			if (sp->cnt_error > 0) {
				sp->cnt_error--;
			}

			/* Log the out-of-order packet */
			if (sp->test->debug) {
				fprintf(stderr, "OUT OF ORDER - incoming packet sequence %llu but expected sequence %d on stream %d", pcount, sp->packet_count, sp->socket);
			}
		}

		/*
		 * jitter measurement
		 *
		 * This computation is based on RFC 1889 (specifically
		 * sections 6.3.1 and A.8).
		 *
		 * Note that synchronized clocks are not required since
		 * the source packet delta times are known.  Also this
		 * computation does not require knowing the round-trip
		 * time.
		 */
#if defined(CONFIG_PLATFORM_8735B)
		gettimeofday_iperf3(&arrival_time, NULL);
#else
		gettimeofday(&arrival_time, NULL);
#endif

		transit = timeval_diff(&sent_time, &arrival_time);

		/* Hack to handle the first packet by initializing prev_transit. */
		if (first_packet) {
			sp->prev_transit = transit;
		}

		d = transit - sp->prev_transit;
		if (d < 0) {
			d = -d;
		}
		sp->prev_transit = transit;
		sp->jitter += (d - sp->jitter) / 16.0;

	} else {
		if (sp->test->debug) {
			printf("Late receive, state = %d\n", sp->test->state);
		}
	}
	return r;
}


/* iperf_udp_send
 *
 * sends the data for UDP
 */
int
iperf_udp_send(struct iperf_stream *sp)
{
	int r;
	int       size = sp->settings->blksize;
	struct timeval before;

#if defined(CONFIG_PLATFORM_8735B)
	gettimeofday_iperf3(&before, 0);
#else
	gettimeofday(&before, 0);
#endif

	++sp->packet_count;

	if (sp->test->udp_counters_64bit) {

		uint32_t  sec, usec;
		uint64_t  pcount;

		sec = htonl(before.tv_sec);
		usec = htonl(before.tv_usec);
		pcount = htobe64(sp->packet_count);

		memcpy(sp->buffer, &sec, sizeof(sec));
		memcpy(sp->buffer + 4, &usec, sizeof(usec));
		memcpy(sp->buffer + 8, &pcount, sizeof(pcount));

	} else {

		uint32_t  sec, usec, pcount;

		sec = htonl(before.tv_sec);
		usec = htonl(before.tv_usec);
		pcount = htonl(sp->packet_count);

		memcpy(sp->buffer, &sec, sizeof(sec));
		memcpy(sp->buffer + 4, &usec, sizeof(usec));
		memcpy(sp->buffer + 8, &pcount, sizeof(pcount));

	}

	r = Nwrite(sp->socket, sp->buffer, size, Pudp);

	if (r <= 0) {
		-- sp->packet_count;
		return r;
	} else {
		sp->result->bytes_sent += r;
		sp->result->bytes_sent_this_interval += r;
	}

	if (sp->test->debug) {
		printf("sent %d bytes of %d, total %llu\n", r, sp->settings->blksize, sp->result->bytes_sent);
	}

	return r;
}


/**************************************************************************/

/*
 * The following functions all have to do with managing UDP data sockets.
 * UDP of course is connectionless, so there isn't really a concept of
 * setting up a connection, although connect(2) can (and is) used to
 * bind the remote end of sockets.  We need to simulate some of the
 * connection management that is built-in to TCP so that each side of the
 * connection knows about each other before the real data transfers begin.
 */

/*
 * Set and verify socket buffer sizes.
 * Return 0 if no error, -1 if an error, +1 if socket buffers are
 * potentially too small to hold a message.
 */
int
iperf_udp_buffercheck(struct iperf_test *test, int s)
{
	/* To avoid gcc warnings */
	(void) test;
	(void) s;
#if 0 /* Unimplemented: send buffer size */
	int rc = 0;
	int sndbuf_actual = 0, rcvbuf_actual = 0;

	/*
	 * Set socket buffer size if requested.  Do this for both sending and
	 * receiving so that we can cover both normal and --reverse operation.
	 */
	int opt;
	socklen_t optlen;

	if ((opt = test->settings->socket_bufsize)) {
		if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, &opt, sizeof(opt)) < 0) {
			i_errno = IESETBUF;
			return -1;
		}
		if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt)) < 0) {
			i_errno = IESETBUF;
			return -1;
		}
	}

	/* Read back and verify the sender socket buffer size */
	optlen = sizeof(sndbuf_actual);
	if (getsockopt(s, SOL_SOCKET, SO_SNDBUF, &sndbuf_actual, &optlen) < 0) {
		i_errno = IESETBUF;
		return -1;
	}

	if (test->debug) {
		printf("SNDBUF is %u, expecting %u\n", sndbuf_actual, test->settings->socket_bufsize);
	}

	if (test->settings->socket_bufsize && test->settings->socket_bufsize > sndbuf_actual) {
		i_errno = IESETBUF2;
		return -1;
	}
	if (test->settings->blksize > sndbuf_actual) {
		char str[80];
		snprintf(str, sizeof(str),
				 "Block size %d > sending socket buffer size %d",
				 test->settings->blksize, sndbuf_actual);
		warning(str);
		rc = 1;
	}

	/* Read back and verify the receiver socket buffer size */
	optlen = sizeof(rcvbuf_actual);
	if (getsockopt(s, SOL_SOCKET, SO_RCVBUF, &rcvbuf_actual, &optlen) < 0) {
		i_errno = IESETBUF;
		return -1;
	}

	if (test->debug) {
		printf("RCVBUF is %u, expecting %u\n", rcvbuf_actual, test->settings->socket_bufsize);
	}
	if (test->settings->socket_bufsize && test->settings->socket_bufsize > rcvbuf_actual) {
		i_errno = IESETBUF2;
		return -1;
	}
	if (test->settings->blksize > rcvbuf_actual) {
		char str[80];
		snprintf(str, sizeof(str),
				 "Block size %d > receiving socket buffer size %d",
				 test->settings->blksize, rcvbuf_actual);
		warning(str);
		rc = 1;
	}

	if (test->json_output) {
		cJSON_AddNumberToObject(test->json_start, "sock_bufsize", test->settings->socket_bufsize);
		cJSON_AddNumberToObject(test->json_start, "sndbuf_actual", sndbuf_actual);
		cJSON_AddNumberToObject(test->json_start, "rcvbuf_actual", rcvbuf_actual);
	}

	return rc;
#else
	return 0;
#endif
}

/*
 * iperf_udp_accept
 *
 * Accepts a new UDP "connection"
 */
int
iperf_udp_accept(struct iperf_test *test)
{
	IPERF3_DBG_INFO("iperf_udp_accept\n");

	struct sockaddr_storage sa_peer;
	int       buf;
	socklen_t len;
	int       sz, s;
	int	      rc;

	/*
	 * Get the current outstanding socket.  This socket will be used to handle
	 * data transfers and a new "listening" socket will be created.
	 */
	s = test->prot_listener;

	/*
	 * Grab the UDP packet sent by the client.  From that we can extract the
	 * client's address, and then use that information to bind the remote side
	 * of the socket to the client.
	 */
	len = sizeof(sa_peer);
	if ((sz = recvfrom(test->prot_listener, &buf, sizeof(buf), 0, (struct sockaddr *) &sa_peer, &len)) < 0) {
		i_errno = IESTREAMACCEPT;
		return -1;
	}

	if (connect(s, (struct sockaddr *) &sa_peer, len) < 0) {
		i_errno = IESTREAMACCEPT;
		return -1;
	}

	/* Check and set socket buffer sizes */
	rc = iperf_udp_buffercheck(test, s);

	if (rc < 0)
		/* error */
	{
		return rc;
	}
	/*
	 * If the socket buffer was too small, but it was the default
	 * size, then try explicitly setting it to something larger.
	 */
	if (rc > 0) {
		if (test->settings->socket_bufsize == 0) {
			int bufsize = test->settings->blksize + UDP_BUFFER_EXTRA;
			printf("Increasing socket buffer size to %d\n",
				   bufsize);
			test->settings->socket_bufsize = bufsize;
			rc = iperf_udp_buffercheck(test, s);
			if (rc < 0) {
				return rc;
			}
		}
	}

#if defined(HAVE_SO_MAX_PACING_RATE)
	/* If socket pacing is specified, try it. */
	if (test->settings->fqrate) {
		/* Convert bits per second to bytes per second */
		unsigned int fqrate = test->settings->fqrate / 8;
		if (fqrate > 0) {
			if (test->debug) {
				printf("Setting fair-queue socket pacing to %u\n", fqrate);
			}
			if (setsockopt(s, SOL_SOCKET, SO_MAX_PACING_RATE, &fqrate, sizeof(fqrate)) < 0) {
				warning("Unable to set socket pacing");
			}
		}
	}
#endif /* HAVE_SO_MAX_PACING_RATE */
	{
		unsigned int rate = test->settings->rate / 8;
		if (rate > 0) {
			if (test->debug) {
				printf("Setting application pacing to %u\n", rate);
			}
		}
	}

	/*
	 * Create a new "listening" socket to replace the one we were using before.
	 */
	test->prot_listener = netannounce(test->settings->domain, Pudp, test->bind_address, test->server_port);
	IPERF3_DBG("test->prot_listener:%d\n", test->prot_listener);

	if (test->prot_listener < 0) {
		i_errno = IESTREAMLISTEN;
		return -1;
	}

	FD_SET(test->prot_listener, &test->read_set);
	test->max_fd = (test->max_fd < test->prot_listener) ? test->prot_listener : test->max_fd;

	/* Let the client know we're ready "accept" another UDP "stream" */
	buf = 987654321;		/* any content will work here */
	if (write(s, &buf, sizeof(buf)) < 0) {
		i_errno = IESTREAMWRITE;
		return -1;
	}

	return s;
}


/*
 * iperf_udp_listen
 *
 * Start up a listener for UDP stream connections.  Unlike for TCP,
 * there is no listen(2) for UDP.  This socket will however accept
 * a UDP datagram from a client (indicating the client's presence).
 */
int
iperf_udp_listen(struct iperf_test *test)
{
	IPERF3_DBG_INFO("iperf_udp_listen \n");

	int s;

	if ((s = netannounce(test->settings->domain, Pudp, test->bind_address, test->server_port)) < 0) {
		i_errno = IESTREAMLISTEN;
		return -1;
	}

	/*
	 * The caller will put this value into test->prot_listener.
	 */
	return s;
}


/*
 * iperf_udp_connect
 *
 * "Connect" to a UDP stream listener.
 */
int
iperf_udp_connect(struct iperf_test *test)
{
	int s, buf, sz;
#ifdef SO_RCVTIMEO
	struct timeval tv;
#endif
	int rc;

	/* Create and bind our local socket. */
	if ((s = netdial(test->settings->domain, Pudp, test->bind_address, test->bind_port, test->server_hostname, test->server_port, -1)) < 0) {
		i_errno = IESTREAMCONNECT;
		return -1;
	}

	IPERF3_DBG_INFO(" iperf_udp_connect: create soket ok\n");

	/* Check and set socket buffer sizes */
	rc = iperf_udp_buffercheck(test, s);
	if (rc < 0)
		/* error */
	{
		close(s);
		return rc;
	}

	/*
	 * If the socket buffer was too small, but it was the default
	 * size, then try explicitly setting it to something larger.
	 */
	if (rc > 0) {
		if (test->settings->socket_bufsize == 0) {
			int bufsize = test->settings->blksize + UDP_BUFFER_EXTRA;
			printf("Increasing socket buffer size to %d\n",
				   bufsize);
			test->settings->socket_bufsize = bufsize;
			rc = iperf_udp_buffercheck(test, s);
			if (rc < 0) {
				close(s);
				return rc;
			}
		}
	}

#if defined(HAVE_SO_MAX_PACING_RATE)
	/* If socket pacing is available and not disabled, try it. */
	if (test->settings->fqrate) {
		/* Convert bits per second to bytes per second */
		unsigned int fqrate = test->settings->fqrate / 8;
		if (fqrate > 0) {
			if (test->debug) {
				printf("Setting fair-queue socket pacing to %u\n", fqrate);
			}
			if (setsockopt(s, SOL_SOCKET, SO_MAX_PACING_RATE, &fqrate, sizeof(fqrate)) < 0) {
				warning("Unable to set socket pacing");
			}
		}
	}
#endif /* HAVE_SO_MAX_PACING_RATE */
	{
		unsigned int rate = test->settings->rate / 8;
		if (rate > 0) {
			if (test->debug) {
				printf("Setting application pacing to %u\n", rate);
			}
		}
	}

#ifdef SO_RCVTIMEO
	/* 30 sec timeout for a case when there is a network problem. */
	tv.tv_sec = 30;
	tv.tv_usec = 0;
	setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (struct timeval *)&tv, sizeof(struct timeval));
#endif

	/*
	 * Write a datagram to the UDP stream to let the server know we're here.
	 * The server learns our address by obtaining its peer's address.
	 */
	buf = 123456789;		/* this can be pretty much anything */
	if (write(s, &buf, sizeof(buf)) < 0) {
		// XXX: Should this be changed to IESTREAMCONNECT?
		close(s);
		i_errno = IESTREAMWRITE;
		return -1;
	}

	/*
	 * Wait until the server replies back to us.
	 */
	if ((sz = recv(s, &buf, sizeof(buf), 0)) < 0) {
		close(s);
		printf("iperf_udp_connect, recv fail\n");
		i_errno = IESTREAMREAD;
		return -1;
	}

	return s;
}


/* iperf_udp_init
 *
 * initializer for UDP streams in TEST_START
 */
int
iperf_udp_init(struct iperf_test *test)
{
	/* To avoid gcc warnings */
	(void) test;
	return 0;
}
