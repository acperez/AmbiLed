#include "arduino.h"

Arduino::Arduino(size_t aLedsLen, int aLedType): mFd(0),
                                                 mLedsLen(aLedsLen),
                                                 mLedType(aLedType) {
}

Arduino::~Arduino() {
  if (mLedsLen > 0) {
    uint8_t data[mLedsLen * 3];
    memset (&data, 0, mLedsLen * 3);

    write(data, mLedsLen * 3);
  }

  close(mFd);
}

int Arduino::open(const char *aDevPath) {
  if (mFd != 0) {
    error("Connection with arduino already stablished\n");
    return 0;
  }

  mFd = initSerial(aDevPath, B115200);
  if (mFd < 0) {
    error("Can't open serial connection with Arduino\n");
    return 0;
  }

  // Wait for arduino ready
  int bufSize = 40;
  uint8_t buffer[40];
  int size = read(buffer, bufSize);
  if (size < 0 || memcmp(buffer, "ready\0", size) != 0) {
    error("Invalid Arduino initialization\n");
    return 0;
  }

  // Configure arduino
  uint8_t data[2];
  data[0] = mLedsLen;
  data[1] = mLedType;

  writeInternal(data, 2);

  size = read(buffer, bufSize);
  if (size < 0 || memcmp(buffer, "ready\0", size) != 0) {
    error("Invalid Arduino initialization\n");
    return 0;
  }

  return 1;
}

int Arduino::initSerial(const char* aDevPath, int aSpeed) {
  int fd;
  struct termios tty;
  memset (&tty, 0, sizeof tty);

  fd = ::open(aDevPath, O_RDWR | O_NOCTTY);
  if (fd == -1) {
    fprintf(stderr, "Error %d opening %s: %s\n", errno, aDevPath, strerror (errno));
    return -1;
  }

  if (tcgetattr (fd, &tty) != 0) {
    error("Error %d from tcgetattr\n", errno);
    return -1;
  }

  // Set baudrate
  cfsetospeed (&tty, aSpeed);
  cfsetispeed (&tty, aSpeed);

  // 8N1
  tty.c_cflag &= ~PARENB;
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;
  // no flow control
  tty.c_cflag &= ~CRTSCTS;

  tty.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

  tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
  tty.c_oflag &= ~OPOST; // make raw

  tty.c_cc[VMIN]  = 1;
  tty.c_cc[VTIME] = 0;

  if (tcsetattr (fd, TCSANOW, &tty) != 0)
  {
    error("error %d from tcsetattr\n", errno);
    return -1;
  }

  return fd;
}

int Arduino::writeInternal(const uint8_t* aData, size_t aBytes) {
#ifdef DEBUG
  print("W -----");
  for(size_t i = 0; i < aBytes; i++) {
    print(" %d ", aData[i]);
  }
  print("-----\n");
#endif

  size_t n = ::write(mFd, aData, aBytes);
  if( n != aBytes ) {
    error("serial_write: couldn't write whole string\n");
    return 0;
  }

  return 1;
}

int Arduino::write(const uint8_t* aData, size_t aBytes) {
  writeInternal(aData, aBytes);

  // wait for ack
  int bufSize = 40;
  uint8_t buffer[40];
  size_t n = read(buffer, bufSize);
  if (n < 0 || memcmp(buffer, "ack\0", n) != 0) {
    error("Arduino protocol error\n");
    return 0;
  }

  return 1;
}

int Arduino::read(uint8_t* aData, size_t aBytes) {
  uint8_t b[1];  // read expects an array, so we give it a 1-byte array
  int i = 0;
  aData[0] = 0;

  do {
    int n = ::read(mFd, b, 1);  // read a char at a time
    if(n == -1) return -1;    // couldn't read

    aData[i] = b[0];
    i++;
  } while (b[0] != '\n');
  i--;

  aData[i] = 0;  // null terminate the string

#ifdef DEBUG
  int x;
  print("R ----- ");
  for(x = 0; x <i; x++) {
    print("%d ", aData[x]);
  }
  print("-----\n");
#endif

  return i;
}
