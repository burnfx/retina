#include "edvs.h"
#include "edvs_impl.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

//#define VERBOSE_DEBUG_PRINTING

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <limits.h>

int edvs_net_open(const char* address, int port)
{
	// open socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd <= 0) {
		printf("edvs_net_open: socket error %d\n", sockfd);
		return -1;
	}
	// prepare address
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	// inet pton
	int i = inet_pton(AF_INET, address, &addr.sin_addr);
	if(i <= 0) {
		printf("edvs_net_open: inet p_ton error %d\n", i);
		return -1;
	}
	// connect
	if(connect(sockfd, (struct sockaddr*)&addr, sizeof(addr))) {
		printf("edvs_net_open: connect error\n");
		return -1;
	}
	// return handle
	return sockfd;
}

ssize_t edvs_net_read(int sockfd, unsigned char* data, size_t n)
{
	ssize_t m = recv(sockfd, data, n, 0);
	if(m < 0) {
		printf("edvs_net_read: recv error %zd\n", m);
		return -1;
	}
	return m;
}

ssize_t edvs_net_write(int sockfd, const char* data, size_t n)
{
	ssize_t m = send(sockfd, data, n, 0);
	if(m != n) {
		printf("edvs_net_send: send error %zd\n", m);
	}
	return m;
}

int edvs_net_close(int sockfd)
{
	int r = shutdown(sockfd, SHUT_RDWR);
	if(r != 0) {
		printf("edvs_net_close: socket shutdown error %d\n", r);
		return -1;
	}
	r = close(sockfd);
	if(r != 0) {
		printf("edvs_net_close: socket close error %d\n", r);
		return -1;
	}
	return 0;				
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

#include <termios.h>
#include <stdio.h>
#include <fcntl.h>

int edvs_serial_open(const char* path, int baudrate)
{
	int baudrate_enum;
	switch(baudrate) {

		case 2000000: baudrate_enum = B2000000; break;
		case 4000000: baudrate_enum = B4000000; break;
		default:
			printf("edvs_open: invalid baudrate '%d'!", baudrate);
			return -1;
	}

	int port = open(path, O_RDWR /*| O_NOCTTY/ * | O_NDELAY*/);
	if(port < 0) {
		printf("edvs_serial_open: open error %d\n", port);
		return -1;
	}
//	fcntl(fd, F_SETFL, 0);
	// set baud rates and other options
	struct termios settings;
	if(tcgetattr(port, &settings) != 0) {
		printf("edvs_serial_open: tcgetattr error\n");
		return -1;
	}
	if(cfsetispeed(&settings, baudrate_enum) != 0) {
		printf("edvs_serial_open: cfsetispeed error\n");
		return -1;
	}
	if(cfsetospeed(&settings, baudrate_enum)) {
		printf("edvs_serial_open: cfsetospeed error\n");
		return -1;
	}
	settings.c_cflag = (settings.c_cflag & ~CSIZE) | CS8; // 8 bits
	settings.c_cflag |= CLOCAL | CREAD;
	settings.c_cflag |= CRTSCTS; // use hardware handshaking
	settings.c_iflag = IGNBRK;
	settings.c_oflag = 0;
	settings.c_lflag = 0;
	settings.c_cc[VMIN] = 1; // minimum number of characters to receive before satisfying the read.
	settings.c_cc[VTIME] = 5; // time between characters before satisfying the read.
	// write modified record of parameters to port
	if(tcsetattr(port, TCSANOW, &settings) != 0) {
		printf("edvs_serial_open: tcsetattr error\n");
		return -1;
	}
	return port;
}

ssize_t edvs_serial_read(int port, unsigned char* data, size_t n)
{
	ssize_t m = read(port, data, n);
	if(m < 0) {
		printf("edvs_serial_read: read error %zd\n", m);
		return -1;
	}
	return m;
}

ssize_t edvs_serial_write(int port, const char* data, size_t n)
{
	ssize_t m = write(port, data, n);
	if(m != n) {
		printf("edvs_serial_send: write error %zd\n", m);
		return -1;
	}
	return m;
}

int edvs_serial_close(int port)
{
	int r = close(port);
	if(r != 0) {
		printf("edvs_serial_close: close error %d\n", r);
		return -1;
	}
	return r;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

/** Reads data from an edvs device */
ssize_t edvs_device_read(edvs_device_t* dh, unsigned char* data, size_t n)
{
	switch(dh->type) {
	case EDVS_NETWORK_DEVICE:
		return edvs_net_read(dh->handle, data, n);
	case EDVS_SERIAL_DEVICE:
		return edvs_serial_read(dh->handle, data, n);
	default:
		return -1;
	}
}

/** Writes data to an edvs device */
ssize_t edvs_device_write(edvs_device_t* dh, const char* data, size_t n)
{
	switch(dh->type) {
	case EDVS_NETWORK_DEVICE:
		return edvs_net_write(dh->handle, data, n);
	case EDVS_SERIAL_DEVICE:
		return edvs_serial_write(dh->handle, data, n);
	default:
		return -1;
	}
}

/** Closes an edvs device connection */
int edvs_device_close(edvs_device_t* dh)
{
	switch(dh->type) {
	case EDVS_NETWORK_DEVICE:
		return edvs_net_close(dh->handle);
	case EDVS_SERIAL_DEVICE:
		return edvs_serial_close(dh->handle);
	default:
		return -1;
	}
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

uint64_t timestamp_limit(int mode)
{
	switch(mode) {
		default: return 0; // no timestamps
		case 1: return (1ull<<16); // 16 bit
		case 2: return (1ull<<24); // 24 bit
		case 3: return (1ull<<32); // 32 bit
	}
}

edvs_device_streaming_t* edvs_device_streaming_start(edvs_device_t* dh)
{
	edvs_device_streaming_t *s = (edvs_device_streaming_t*)malloc(sizeof(edvs_device_streaming_t));
	if(s == 0) {
		return 0;
	}
	s->device = dh;
	s->timestamp_mode = 0; // 0
	s->use_system_time = 1;// 0;
	s->length = 8192;
	s->buffer = (unsigned char*)malloc(s->length);
	s->offset = 0;
	s->current_time = 0;
	s->last_timestamp = timestamp_limit(s->timestamp_mode);
	if(s->timestamp_mode == 1) {
		if(edvs_device_write(dh, "!E1\n", 4) != 4)
			return 0;
	}
	else if(s->timestamp_mode == 2) {
		if(edvs_device_write(dh, "!E2\n", 4) != 4)
			return 0;
	}
	else if(s->timestamp_mode == 3) {
		if(edvs_device_write(dh, "!E3\n", 4) != 4)
			return 0;
	}
	else {
		if(edvs_device_write(dh, "!E0\n", 4) != 4)
			return 0;
	}
	if(edvs_device_write(dh, "E+\n", 3) != 3)
		return 0;
	return s;
}

uint64_t get_micro_time() {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return 1000000ull*(uint64_t)(t.tv_sec) + (uint64_t)(t.tv_nsec)/1000ull;
}

ssize_t edvs_device_streaming_read(edvs_device_streaming_t* s, edvs_event_t* events, size_t n, edvs_special_t* special, size_t* ns)
{
	// realtime timer
	uint64_t system_clock_time = 0;
	if(s->use_system_time == 1) {
		system_clock_time = get_micro_time();
#ifdef VERBOSE_DEBUG_PRINTING
		printf("Time: %ld\n", system_clock_time);
#endif
	}
	// constants
	const int timestamp_mode = (s->timestamp_mode > 3) ? 0 : s->timestamp_mode;
	const unsigned char cHighBitMask = 0x80; // 10000000
	const unsigned char cLowerBitsMask = 0x7F; // 01111111
	const unsigned int cNumBytesTimestamp = (s->timestamp_mode == 0) ? 0 : (s->timestamp_mode + 1);
	const uint64_t cTimestampLimit = timestamp_limit(timestamp_mode);
	const unsigned int cNumBytesPerEvent = 2 + cNumBytesTimestamp;
	const unsigned int cNumBytesPerSpecial = 2 + cNumBytesTimestamp + 1 + 16;
	const unsigned int cNumBytesAhead = (cNumBytesPerEvent > cNumBytesPerSpecial) ? cNumBytesPerEvent : cNumBytesPerSpecial;
	// read bytes
	unsigned char* buffer_begin = s->buffer;
	unsigned char* buffer = buffer_begin;
	size_t num_bytes_events = n*cNumBytesPerEvent;
	size_t num_bytes_buffer = s->length - s->offset;
	size_t num_read = (num_bytes_buffer < num_bytes_events ? num_bytes_buffer : num_bytes_events);
	ssize_t bytes_read = edvs_device_read(s->device, buffer + s->offset, num_read);
	size_t num_special = 0;
	if(bytes_read < 0) {
		return bytes_read;
	}
	bytes_read += s->offset;
	// parse events
	ssize_t i = 0; // index of current byte
	edvs_event_t* event_it = events;
	edvs_special_t* special_it = special;
#ifdef VERBOSE_DEBUG_PRINTING
	printf("START\n");
#endif
	while(i+cNumBytesAhead < bytes_read) {
		// break if no more room for special
		if(special != 0 && ns != 0 && num_special >= *ns) {
			break;
		}
		// get to bytes
		unsigned char a = buffer[i];
		unsigned char b = buffer[i + 1];
#ifdef VERBOSE_DEBUG_PRINTING
		printf("e: %d %d\n", a, b);
#endif
		i += 2;
		// check for and parse 1yyyyyyy pxxxxxxx
		if(!(a & cHighBitMask)) { // check that the high bit o first byte is 0
			// the serial port missed a byte somewhere ...
			// skip one byte to jump to the next event
			i --;
			continue;
		}
		// check for special data
		special = 0;
		size_t special_data_len = 0;
		if(special != 0 && a == 0 && b == 0) {
			// get special data length
			special_data_len = (buffer[i] & 0x0F);
			// HACK assuming special data always sends timestamp!
			if(special_data_len >= cNumBytesTimestamp) {
				special_data_len -= cNumBytesTimestamp;
			}
			else {
				printf("ERROR parsing special data length!\n");
			}
			i ++;
#ifdef VERBOSE_DEBUG_PRINTING
			printf("s: len=%ld\n", special_data_len);
#endif
		}
		// read timestamp
		uint64_t timestamp;
		if(timestamp_mode == 1) {
			timestamp =
				  ((uint64_t)(buffer[i  ]) <<  8)
				|  (uint64_t)(buffer[i+1]);
		}
		else if(timestamp_mode == 2) {
			timestamp =
				  ((uint64_t)(buffer[i  ]) << 16)
				| ((uint64_t)(buffer[i+1]) <<  8)
				|  (uint64_t)(buffer[i+2]);
			// printf("%d %d %d %d %d\n", a, b, buffer[i+0], buffer[i+1], buffer[i+2]);
#ifdef VERBOSE_DEBUG_PRINTING
				printf("t: %d %d %d -> %ld\n", buffer[i], buffer[i+1], buffer[i+2], timestamp);
#endif
		}
		else if(timestamp_mode == 3) {
			timestamp =
				  ((uint64_t)(buffer[i  ]) << 24)
				| ((uint64_t)(buffer[i+1]) << 16)
				| ((uint64_t)(buffer[i+2]) <<  8)
				|  (uint64_t)(buffer[i+3]);
		}
		else {
			timestamp = 0;
		}
		// advance byte count
		i += cNumBytesTimestamp;
		// compute event time
		if(s->use_system_time == 1) {
			// compute time since last
			// FIXME this does not assure that timestamps are increasing!!!
			uint64_t dt;
			if(timestamp < s->last_timestamp) {
				// we have a wrap
				dt = cTimestampLimit + timestamp - s->last_timestamp;
			}
			else {
				// we do not have a wrap
				// OR long time no event => ignore
				dt = s->last_timestamp - timestamp;
			}
			s->current_time = system_clock_time + dt;
			s->last_timestamp = timestamp;
			// // OLD
			// if(s->last_timestamp == cTimestampLimit) {
			// // 	// start event time at zero
			// // 	// FIXME this is problematic with multiple event streams
			// // 	//       as they will have different offsets
			// // 	s->last_timestamp = system_clock_time;
			// 	s->last_timestamp = 0; // use system clock time zero point
			// }
			// s->current_time = system_clock_time;
			// s->last_timestamp = system_clock_time;
		}
		else {
			if(timestamp_mode != 0) {
				if(s->current_time < 8) { // ignore timestamps of first 8 events
					s->current_time ++;
				}
				else {
					if(s->last_timestamp != cTimestampLimit) {
						// FIXME possible errors for 16 bit timestamps if no event for more than 65 ms
						// FIXME possible errors for 24/32 bit timestamps if timestamp is wrong
						if(timestamp >= s->last_timestamp) {
							s->current_time += (timestamp - s->last_timestamp);
						}
						else {
							// s->current_time += 2 * timestamp;
							s->current_time += timestamp + (cTimestampLimit - s->last_timestamp);
						}
					}
				}
			}
//			printf("old=%lu \tnew=%lu \tt=%lu\n", s->last_timestamp, timestamp, s->current_time);
			s->last_timestamp = timestamp;
		}

		if(special != 0 && ns != 0 && a == 0 && b == 0) {
			// create special
			special_it->t = s->current_time;
			special_it->n = special_data_len;
			// read special data
#ifdef VERBOSE_DEBUG_PRINTING
			printf("SPECIAL DATA:");
#endif
			size_t k;
			for(k=0; k<special_it->n; k++) {
				special_it->data[k] = buffer[i+k];
#ifdef VERBOSE_DEBUG_PRINTING
				printf(" %d", special_it->data[k]);
#endif
			}
#ifdef VERBOSE_DEBUG_PRINTING
			printf("\n");
#endif
			i += special_it->n;
			special_it++;
			num_special++;
		}
		else {
			// create event
			event_it->t = s->current_time;
			event_it->x = (uint16_t)(b & cLowerBitsMask);
			event_it->y = (uint16_t)(a & cLowerBitsMask);
			event_it->parity = ((b & cHighBitMask) ? 1 : 0);
			event_it->id = 0;
			event_it++;
		}
	}
	// i is now the number of processed bytes
	s->offset = bytes_read - i;
	if(s->offset > 0) {
		size_t j;
		for(j=0; j<s->offset; j++) {
			buffer[j] = buffer[i + j];
		}
	}
	// return
	if(ns != 0) {
		if(special != 0) {
			*ns = special_it - special;
		}
		else {
			*ns = 0;
		}
	}
	return event_it - events;
}

int edvs_device_streaming_write(edvs_device_streaming_t* s, const char* cmd, size_t n)
{
	if(edvs_device_write(s->device, cmd, n) != n)
		return -1;
	return 0;
}

int edvs_device_streaming_stop(edvs_device_streaming_t* s)
{
	int r = edvs_device_streaming_write(s, "E-\n", 3);
	if(r != 0) return r;
	free(s->buffer);
	free(s);
	return 0;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

ssize_t edvs_file_read(FILE* fh, edvs_event_t* events, size_t n)
{
	return fread((void*)events, sizeof(edvs_event_t), n, fh);
}

ssize_t edvs_file_write(FILE* fh, const edvs_event_t* events, size_t n)
{
	size_t m = fwrite((const void*)events, sizeof(edvs_event_t), n, fh);
	if(m != n) {
		printf("edvs_file_write: could not write to file\n");
		return -1;
	}
	return m;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

#include <time.h>

edvs_file_streaming_t* edvs_file_streaming_start(const char* filename, uint64_t dt, float ts)
{
	edvs_file_streaming_t *s = (edvs_file_streaming_t*)malloc(sizeof(edvs_file_streaming_t));
	if(s == 0) {
		return 0;
	}
	s->fh = fopen(filename, "rb");
	s->is_eof = 0;
	s->dt = dt;
	s->timescale = ts;
	s->num_max = 1024;
	s->unprocessed = (edvs_event_t*)malloc(s->num_max*sizeof(edvs_event_t));
	s->num_curr = 0;
	s->is_first = 1;
	s->start_time = clock();
	s->start_event_time = 0;
	s->current_event_time = 0;
	return s;
}

ssize_t edvs_file_streaming_read(edvs_file_streaming_t* s, edvs_event_t* events, size_t events_max)
{
	// get time
	if(s->dt == 0) {
		uint64_t nt = ((clock() - s->start_time)*1000000)/CLOCKS_PER_SEC;
		s->current_event_time = s->start_event_time + (uint64_t)(s->timescale*(float)(nt));
	}
	else {
		s->current_event_time += s->dt;
	}
	size_t num_total = 0;
	do {
		// read more from stream
		if(s->num_curr == 0) {
			s->num_curr = edvs_file_read(s->fh, s->unprocessed, s->num_max);
			if(s->num_curr == 0) s->is_eof = 1;
			return 0;
		}
		if(s->is_first) {
			s->start_event_time = s->unprocessed[0].t;
			s->current_event_time = s->start_event_time;
			s->is_first = 0;
		}
		// find first event with time greater equal to desires time
		size_t n = 0;
		//printf("ti=%ld\n",s->unprocessed[n].t);
		while( n < s->num_curr
			&& num_total < events_max
			&& s->unprocessed[n].t < s->current_event_time
		) {
			n++;
			num_total++;
		}
		// copy events to output buffer
		memcpy(
			(void*)events,
			(const void*)s->unprocessed,
			n*sizeof(edvs_event_t));
		events += n;
		// move remaining events in unprocessed buffer to start
		memmove(
			(void*)s->unprocessed,
			(const void*)(s->unprocessed + n),
			(s->num_curr - n)*sizeof(edvs_event_t));
		s->num_curr -= n;
	}
	while(s->num_curr == 0);
	//printf("num_total=%zd\n",num_total);
	return num_total;
}

int edvs_file_streaming_stop(edvs_file_streaming_t* s)
{
	fclose(s->fh);
	free(s->unprocessed);
	free(s);
	return 0;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //

#include <stdint.h>
#include <string.h>

edvs_stream_handle edvs_open(const char* uri)
{
	// check for a ':' -> network socket
	const char *pcolon = strstr(uri, ":");
	if(pcolon != NULL) {
		// get ip from uri
		size_t ip_len = pcolon-uri;
		char *ip = (char*)malloc(ip_len+1);
		memcpy(ip, uri, ip_len);
		ip[ip_len] = '\0';
		// get port from uri
		int port = atoi(pcolon+1);
		// open device
		printf("Opening network socket: ip=%s, port=%d\n", ip, port);
		int dev = edvs_net_open(ip, port);
		if(dev < 0) {
			printf("edvs_open: URI seems to point to a network socket, but connection failed\n");
			return 0;
		}
		free(ip);
		// start streaming
		edvs_device_t* dh = (edvs_device_t*)malloc(sizeof(edvs_device_t));
		dh->type = EDVS_NETWORK_DEVICE;
		dh->handle = dev;
		edvs_device_streaming_t* ds = edvs_device_streaming_start(dh);
		struct edvs_stream_t* s = (struct edvs_stream_t*)malloc(sizeof(struct edvs_stream_t));
		s->type = EDVS_DEVICE_STREAM;
		s->handle = (uintptr_t)ds;
		return s;
	}
	// check for 'baudrate' -> serial
	char *pbaudrate = strstr(uri, "baudrate");
	if(pbaudrate != NULL) {
		// parse uri for port
		int port_len = pbaudrate - uri - 1;
		char* port = (char*)malloc(port_len+1);
		memcpy(port, uri, port_len);
		port[port_len] = '\0';
		// parse uri for baudrate
		// FIXME find end of baudrate
		int baudrate = atoi(pbaudrate+9);
		// open device
		printf("Opening serial port: port=%s, baudrate=%d\n", port, baudrate);
		int dev = edvs_serial_open(port, baudrate);
		if(dev < 0) {
			printf("edvs_open: URI seems to point to a serial port, but connection failed\n");
			return 0;
		}
		// start streaming
		edvs_device_t* dh = (edvs_device_t*)malloc(sizeof(edvs_device_t));
		dh->type = EDVS_SERIAL_DEVICE;
		dh->handle = dev;
		edvs_device_streaming_t* ds = edvs_device_streaming_start(dh);
		struct edvs_stream_t* s = (struct edvs_stream_t*)malloc(sizeof(struct edvs_stream_t));
		s->type = EDVS_DEVICE_STREAM;
		s->handle = (uintptr_t)ds;
		return s;
	}
	// else -> file
	{
		const char *pquest = strstr(uri, "?");
		// parse filename
		size_t fn_len = strlen(uri);
		if(pquest != NULL) {
			fn_len = pquest - uri;
		}
		char *fn = malloc(fn_len+1);
		memcpy(fn, uri, fn_len);
		fn[fn_len] = '\0';
		// parse arguments
		uint64_t dt = 0;
		float ts = 1.0f;
		if(pquest != NULL) {
			// create copy of token string
			size_t len = strlen(pquest+1);
			char* tmp = malloc(len+1);
			memcpy(tmp, pquest+1, len+1);
			// parse tokens by &
			char *token = strtok(tmp, "&");
			while(token != NULL) {
				// find = in token and delete
				char* val = strstr(token, "=");
				val[0] = '\0';
				val++;
				// check which token we are parsing
				if(strcmp(token,"dt")==0) {
					dt = atoi(val);
				}
				if(strcmp(token,"ts")==0) {
					ts = atof(val);
				}
				// next token
				token = strtok(NULL, "&");
			}
		}
		// open
		printf("Opening event file '%s', using dt=%lu, ts=%f\n", fn, dt, ts);
		edvs_file_streaming_t* ds = edvs_file_streaming_start(fn, dt, ts);
		free(fn);
		struct edvs_stream_t* s = (struct edvs_stream_t*)malloc(sizeof(struct edvs_stream_t));
		s->type = EDVS_FILE_STREAM;
		s->handle = (uintptr_t)ds;
		return s;
	}
	// printf("edvs_open: did not recognize URI\n");
	// return 0;
}

int edvs_close(edvs_stream_handle s)
{
	if(s->type == EDVS_DEVICE_STREAM) {
		edvs_device_streaming_t* ds = (edvs_device_streaming_t*)s->handle;
		edvs_device_t* dh = ds->device;
		edvs_device_streaming_stop(ds);
		edvs_device_close(dh);
		free(s);
		return 0;
	}
	if(s->type == EDVS_FILE_STREAM) {
		edvs_file_streaming_t* ds = (edvs_file_streaming_t*)s->handle;
		edvs_file_streaming_stop(ds);
		free(s);
		return 0;
	}
	printf("edvs_close: unknown stream type\n");
	return -1;
}

int edvs_eos(edvs_stream_handle s)
{
	if(s->type == EDVS_DEVICE_STREAM) {
		// TODO can device streams reach end of stream?
		return 0;
	}
	if(s->type == EDVS_FILE_STREAM) {
		edvs_file_streaming_t* ds = (edvs_file_streaming_t*)s->handle;
		return ds->is_eof;
	}
	printf("edvs_eos: unknown stream type\n");
	return -1;
}

ssize_t edvs_read(edvs_stream_handle s, edvs_event_t* events, size_t n)
{
	return edvs_read_ext(s, events, n, 0, 0);
}

ssize_t edvs_read_ext(edvs_stream_handle s, edvs_event_t* events, size_t n, edvs_special_t* special, size_t* ns)
{
	if(s->type == EDVS_DEVICE_STREAM) {
		edvs_device_streaming_t* ds = (edvs_device_streaming_t*)s->handle;
		return edvs_device_streaming_read(ds, events, n, special, ns);
	}
	if(s->type == EDVS_FILE_STREAM) {
		edvs_file_streaming_t* ds = (edvs_file_streaming_t*)s->handle;
		if(ns != 0) {
			*ns = 0;
		}
		return edvs_file_streaming_read(ds, events, n);
	}
	printf("edvs_read: unknown stream type\n");
	return -1;
}

ssize_t edvs_write(edvs_stream_handle s, const char* cmd, size_t n)
{
	if(s->type == EDVS_DEVICE_STREAM) {
		edvs_device_streaming_t* ds = (edvs_device_streaming_t*)s->handle;
		return edvs_device_streaming_write(ds, cmd, n);
	}
	if(s->type == EDVS_FILE_STREAM) {
		printf("edvs_write: ERROR can not write to file stream!\n");
		return -1;
	}
	printf("edvs_write: unknown stream type\n");
	return -1;
}

// ----- ----- ----- ----- ----- ----- ----- ----- ----- //
