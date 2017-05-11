/* file: mr.c
 * auth: guthrie linck (burpod@gmail.com)
 * date: 12/2/2016
 * desc:

 * putty: 115200
 * data: 8
 * one stop bit
 * no parity
 * flow control xon/xoff

 * to reader: 9600
 * data 8
 * one stop
 * no parity
 * no flow control
 */

#include "mr.h"

/* list of readers for 8 or 16 port usb hub
 * last one has null string for name
 */

static reader_t usb_readers[] = {
  { "reader0", "/dev/ttyUSB0", "r0", 1 },
  { "reader1", "/dev/ttyUSB1", "r1", 1 },
  { "reader2", "/dev/ttyUSB2", "r2", 1 },
  { "reader3", "/dev/ttyUSB3", "r3", 1 },
  { "reader4", "/dev/ttyUSB4", "r4", 1 },
  { "reader5", "/dev/ttyUSB5", "r5", 1 },
  { "reader6", "/dev/ttyUSB6", "r6", 1 },
  { "reader7", "/dev/ttyUSB7", "r7", 1 },
  { "reader8", "/dev/ttyUSB8", "r8", 1 },
  { "reader9", "/dev/ttyUSB9", "r9", 1 },
  { "reader10", "/dev/ttyUSB10", "r10", 1 },
  { "reader11", "/dev/ttyUSB11", "r11", 1 },
  { "reader12", "/dev/ttyUSB12", "r12", 1 },
  { "reader13", "/dev/ttyUSB13", "r13", 1 },
  { "reader14", "/dev/ttyUSB14", "r14", 1 },
  { "reader15", "/dev/ttyUSB15", "r15", 1 },
  { "" }
};

/*
 * list of readers for 24 port combox
 */

static reader_t combox_readers[] = {
  { "reader0", "/dev/ttyT8Sy0", "r0", 1 },
  { "reader1", "/dev/ttyT8Sy1", "r1", 1 },
  { "reader2", "/dev/ttyT8Sy2", "r2", 1 },
  { "reader3", "/dev/ttyT8Sy3", "r3", 1 },
  { "reader4", "/dev/ttyT8Sy4", "r4", 1 },
  { "reader5", "/dev/ttyT8Sy5", "r5", 1 },
  { "reader6", "/dev/ttyT8Sy6", "r6", 1 },
  { "reader7", "/dev/ttyT8Sy7", "r7", 1 },
  { "reader8", "/dev/ttyT8Sy8", "r8", 1 },
  { "reader9", "/dev/ttyT8Sy9", "r9", 1 },
  { "reader10", "/dev/ttyT8Sy10", "r10", 1 },
  { "reader11", "/dev/ttyT8Sy11", "r11", 1 },
  { "reader12", "/dev/ttyT8Sy12", "r12", 1 },
  { "reader13", "/dev/ttyT8Sy13", "r13", 1 },
  { "reader14", "/dev/ttyT8Sy14", "r14", 1 },
  { "reader15", "/dev/ttyT8Sy15", "r15", 1 },
  { "reader16", "/dev/ttyT8Sy16", "r16", 1 },
  { "reader17", "/dev/ttyT8Sy17", "r17", 1 },
  { "reader18", "/dev/ttyT8Sy18", "r18", 1 },
  { "reader19", "/dev/ttyT8Sy19", "r19", 1 },
  { "reader20", "/dev/ttyT8Sy20", "r20", 1 },
  { "reader21", "/dev/ttyT8Sy21", "r21", 1 },
  { "reader22", "/dev/ttyT8Sy22", "r22", 1 },
  { "reader23", "/dev/ttyT8Sy23", "r23", 1 },
  { "" }
};

int verbose = 0;
char logfile[128];
FILE *flog;
int min_data = 5;
int max_readers = 0;

/*
 * main()
 * just parse options and load the reader threads from specified reader list
 * for now, just wait on getchar() and exits user hits return
 * program options:
 *  -v  verbose mode
 *  -o  add a prefix label to log file name
 *  -m  max number of readers to load
 *  -d  mininum number of characters read for an actual data line
 *  -c  use combox reader device list (default uses usb list)
 */

int main(int argc,  char *const *argv)
{
  int opt, i;
  reader_t *r, *readers;
  struct timeval tv;                 // time structs for timestamp
  struct tm tm;

  flog = stdout;                     // default output to stdout
  readers = usb_readers;             // default to usb hub of readers

  while ((opt = getopt(argc, argv, "cvo:m:d:")) != -1) {
    switch (opt) {
      case 'v':                      // -v verbose mode
	verbose = 1;
	break;
      case 'o':                      // -o set a prefix for log file name
        gettimeofday(&tv, 0);        // add date/timestamp to prefix
        localtime_r(&tv.tv_sec, &tm);
        (strlen(optarg) > 64) && (optarg[64] = 0);  // prevent buffer overflow
        sprintf(logfile, "%s%4d%2d%02d:%02d:%02d", optarg, tm.tm_year + 1900,
          tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min);
        if (!(flog = fopen(logfile, "a"))) {
          fprintf(stderr, "can't open logfile %s (%s)\n",
            logfile, strerror(errno));
          return 1;
        }
        break;
      case 'm':                      // -m max number of readers to open
	max_readers = atoi(optarg);
	break;
      case 'd':                      // -d min # of chars for a data line
	min_data = atoi(optarg);
	break;
      case 'c':                      // -c for combox readers (/dev/ttyT8SyXX)
	readers = combox_readers;
	break;
      default:
        fprintf(stderr, "Usage: mr [-v -o PREFIX -m MAX -d MIN -c]\n");
	return 1;
    }
  }

  // create thread for each reader; reader list must be terminated by
  // a reader with an empty name string
  // only open MAX number of readers, specified by -m
  // don't open readers if active flag is set to zero
  for (i = 0, r = readers; r->name[0]; r++, i++) {
    if (max_readers && (i == max_readers)) { // stop loading after MAX readers
      break;
    } if (!r->active) {              // skip reader if marked non-active
      continue;
    } if (verbose) {
      fprintf(flog, "creating reader thread %s (%s)\n", r->name, r->file);
    } if (pthread_create(&r->pt, 0, (void*) reader_thread, r)) {
      fprintf(flog, "failed to open reader thread %s (%s)\n",
        r->name, r->file);
    }
  }

  getchar();                         // exit when someone hits return

  return 0;
}

/*
 * reader_thread(reader_t *r)
 * opens up the reader file/data stream and reads data writing to log file
 */

void reader_thread(reader_t *r)
{
  int n;
  char buf[256];                     // read buffer; 256 chars big enough
  struct timeval tv;                 // structs for timestamp data
  struct tm tm;

  // wait until input stream is ready, sleep for 1 second intervals
  while ((r->fd = open(r->file, O_RDWR | O_NOCTTY)) <= 0) {
    if (verbose) {
      fprintf(flog, "waiting for reader file to open: %s (%s)\n",
        r->name, r->file);
    } sleep(1);
  } if (verbose) {
    fprintf(flog, "reader thread opened: name %s (%s)\n",
      r->name, r->file);
  }

  // default behavior for open/read will read in one line at a time
  // this is "canonical" mode; read will block until a newline is read
  while (1) {                          // infinite read loop
    n = read(r->fd, buf, sizeof(buf)); // read in a line
    buf[n-1] = 0;                      // null terminate - replace \r with \0
    if (n < min_data) {                // <min_data chars: unimportant non-data
      if (verbose && n > 1) {          // but >1 char got something
        fprintf(flog, "[%s] %s\n", r->name, buf); // spit it out if verbose
      }
      continue;
    }
    gettimeofday(&tv, 0);            // read actual data, get timestamp
    localtime_r(&tv.tv_sec, &tm);    // and print to log
    fprintf(flog, "[%s] %02d:%02d:%02d.%06ld: %s\n", r->name,
          tm.tm_hour, tm.tm_min, tm.tm_sec, tv.tv_usec, buf);
    if (flog == stdout) {            // flush output if writing to stdout
      fflush(flog);                  // so we see output in realtime
    }
  }
}

