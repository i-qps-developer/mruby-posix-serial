#include <stdio.h>
#include <string.h> 
#include <stdlib.h>
#include <termios.h>
#include <sys/signal.h>
#include <unistd.h>
#include <fcntl.h>

#include "mruby.h"
#include "mruby/data.h"
#include "mruby/class.h"
#include "mruby/value.h"
#include "mruby/string.h"

typedef struct {
  int fd;
  struct termios new_term_io;
  struct termios old_term_io;
} serial_params;

void _createSerial(mrb_value, int, int, int, int, serial_params *); 
static void mrb_posix_serial_free(mrb_state *, void *);

const static mrb_data_type mrb_posix_serial_type = { "PosixSerial", mrb_posix_serial_free };

//
// Baud rate
//
unsigned int get_baud_rate(int rate) {
  switch(rate) {
    case   4800 : return B4800;
    case   9600 : return B9600;
    case  19200 : return B19200;
    case  38400 : return B38400;
    case  57600 : return B57600;
    case 115200 : return B115200;
    case 230400 : return B230400;
  }
  return B9600;
} 

//
// Setting the Character Size
//
void setting_character_size(struct termios *term, int bits) {
  term->c_cflag &= ~CSIZE;
  switch(bits) {
    case 5 : term->c_cflag |= CS5;
    case 6 : term->c_cflag |= CS6;
    case 7 : term->c_cflag |= CS7;
    case 8 : term->c_cflag |= CS8;
  }
}

//
// Setting Stop bit
//
void setting_stop_bits(struct termios *term, int stop) {
  switch(stop) {
//    case 1 : term->c_cflag &= ~CSTOPB;
    case 2 : term->c_cflag |= CSTOPB;
  }
}

//
// Setting Parity Checking
//
void setting_parity_checking(struct termios *term, int parity) {
  // パリティデータエラーは無視に設定。
  if (parity == 0) {
//    term->c_iflag |= IGNPAR;
    term->c_cflag &= ~PARENB;
  }
  // 偶数パリティ
  else if (parity == 1) {
    term->c_cflag |= PARENB;
    term->c_cflag &= ~PARODD;
  }
  // 奇数パリティ
  else if (parity == 2) {
    term->c_cflag |= PARENB;
    term->c_cflag |= PARODD;
  }
}

//
//
//
int create_serial(mrb_value fnm, int rate, int bits, int stop, int parity, serial_params *upp)
{
  int fd = open(RSTRING_PTR(fnm), O_RDWR | O_NOCTTY | O_NDELAY);
  if (fd == -1)
  {
    return -1;
   }
   else{
     fcntl(fd, F_SETFL, 0);
   }
  upp->fd = fd;

  tcgetattr(fd, &upp->old_term_io);
  upp->new_term_io.c_iflag = 0;
  upp->new_term_io.c_oflag = 0;
  upp->new_term_io.c_cflag = 0;
  upp->new_term_io.c_lflag = 0;

  memset(upp->new_term_io.c_cc, '\0', sizeof(upp->new_term_io.c_cc));
 
  cfsetispeed(&upp->new_term_io, get_baud_rate(rate));
  cfsetospeed(&upp->new_term_io, get_baud_rate(rate));
  
  upp->new_term_io.c_cflag |= (CLOCAL | CREAD);

  setting_character_size(&upp->new_term_io, bits);
  setting_stop_bits(&upp->new_term_io, stop);
  setting_parity_checking(&upp->new_term_io, parity);

  // Rawモード出力。
  //upp->new_term_io.c_oflag = 0;
  upp->new_term_io.c_oflag &= ~OPOST;
  // 入力モード設定。non-canonical, no echo,
  //upp->new_term_io.c_lflag = 0;
  upp->new_term_io.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
  // inter-character timer
  upp->new_term_io.c_cc[VTIME] = 10;
  // 1文字来るまで読み込みをブロック。
  upp->new_term_io.c_cc[VMIN] = 0;
  // モデムラインのクリア。
  tcflush(fd, TCIFLUSH);
  // 新しい設定を適用。
  tcsetattr(fd, TCSANOW, &upp->new_term_io);
  return 1;
}

//
// initialize
//
mrb_value mrb_posix_serial_init(mrb_state *mrb, mrb_value self)
{
  int ret;
  serial_params *upp;

  mrb_value fnm; 
  mrb_int   rate = 9600;
  mrb_int   bits = 8;
  mrb_int   stop = 1;
  mrb_int   parity = 0;

  mrb_get_args(mrb, "S|iiii", &fnm, &rate, &bits, &stop, &parity);
//  int argc = mrb_get_args(mrb, "S|i|i|i|i", &fnm, &rate, &bits, &stop, &parity);

  printf("name : %s rate : %d bits : %d stop : %d parity : %d\n", RSTRING_PTR(fnm), rate, bits, stop, parity);

  upp = (serial_params *) mrb_malloc(mrb, sizeof(serial_params));
  upp->fd = -1;
  ret = create_serial(fnm, rate, bits, stop, parity, upp);
  if (ret == -1) {
    mrb_raisef(mrb, E_RUNTIME_ERROR, "Failed to open the serial device");
  }
  printf("created serial\n");

  DATA_TYPE(self) = &mrb_posix_serial_type;
  DATA_PTR(self) = upp;
  return self;
}

//
// Close
//
mrb_value mrb_posix_serial_close(mrb_state *mrb, mrb_value self)
{
//  mrb_full_gc(mrb);

  serial_params *upp;

  upp = DATA_PTR(self);
  if (upp != NULL) {
    if (upp->fd > 0) {
      tcsetattr(upp->fd, TCSANOW, &upp->old_term_io);
      close(upp->fd);
      upp->fd = -1;
      mrb_free(mrb, upp);
      DATA_PTR(self) = NULL;
    }
  }
  
  return mrb_fixnum_value(0);
}

//
// putc
//
mrb_value mrb_posix_serial_putc(mrb_state *mrb, mrb_value self)
{
  serial_params *upp;
  mrb_value ch;
  unsigned char nc;

  mrb_get_args(mrb, "S", &ch);
  nc = (unsigned char)RSTRING_PTR(ch)[0];
  printf("char : %c\n", nc);
  upp = DATA_PTR(self);
  if (write(upp->fd, &nc, 1) != 1) {
    return mrb_fixnum_value(-1);
  }
  return mrb_fixnum_value(0);
}

//
// puts
//
mrb_value mrb_posix_serial_puts(mrb_state *mrb, mrb_value self)
{
  serial_params *upp;
  mrb_value str;

  mrb_get_args(mrb, "S", &str);
  upp = DATA_PTR(self);
  if (write(upp->fd, RSTRING_PTR(str), strlen(RSTRING_PTR(str))) == -1) {
    return mrb_fixnum_value(-1);
  }
  return mrb_fixnum_value(0);
}

//
// getc
//
mrb_value mrb_posix_serial_getc(mrb_state *mrb, mrb_value self)
{
  serial_params *upp;
  unsigned char buf[2];
  int nn;

  memset(buf, '\0', sizeof(buf));
  upp = DATA_PTR(self);
  nn = read(upp->fd, (unsigned char *)&buf[0], 1);
  if (nn == 1) {
    return mrb_str_new_cstr(mrb, (const char *)buf);
  }
  return mrb_nil_value();
//  return mrb_fixnum_value(cc);
}

//
// gets
//
mrb_value mrb_posix_serial_gets(mrb_state *mrb, mrb_value self)
{
  serial_params *upp;
  unsigned char buf[1024];
  int nn, pp;

  memset(buf, '\0', sizeof(buf));
  pp = 0;
  upp = DATA_PTR(self);
  while(1) {
    nn = read(upp->fd, (unsigned char *)&buf[pp], 1);
    if (nn == 1) {
      if (buf[pp] == '\n') {
        break;
      }
      pp++;
      if (pp + 1 > sizeof(buf)) {
        break;
      }
    }
  }
  return mrb_str_new_cstr(mrb, (const char *)buf);
}

//
// read time out
//
mrb_value mrb_posix_serial_read_timeout(mrb_state *mrb, mrb_value self)
{
  mrb_int       time_out;
  serial_params *upp;
  upp = DATA_PTR(self);

  mrb_get_args(mrb, "i", &time_out);

  // inter-character timer
  upp->new_term_io.c_cc[VTIME] = time_out;
  // 新しい設定を適用。
  tcsetattr(upp->fd, TCSANOW, &upp->new_term_io);
  return mrb_fixnum_value(0);
}

//
//
//
static void mrb_posix_serial_free(mrb_state *mrb, void *ptr) {
  serial_params *upp = ptr;
  if (upp != NULL) {
    if (upp->fd > 0) {
      tcsetattr(upp->fd, TCSANOW, &upp->old_term_io);
      close(upp->fd);
    }
    mrb_free(mrb, upp);
  }
}

//
// gem initialize
//
void mrb_mruby_posix_serial_gem_init(mrb_state *mrb)
{
  struct RClass *cls;

  cls = mrb_define_class(mrb, "PosixSerial", mrb->object_class);
  MRB_SET_INSTANCE_TT(cls, MRB_TT_DATA); 
  mrb_define_method(mrb, cls, "initialize",   mrb_posix_serial_init, MRB_ARGS_ARG(1,4));
  mrb_define_method(mrb, cls, "close",        mrb_posix_serial_close, MRB_ARGS_NONE());
  mrb_define_method(mrb, cls, "putc",         mrb_posix_serial_putc, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, cls, "puts",         mrb_posix_serial_puts, MRB_ARGS_REQ(1));
  mrb_define_method(mrb, cls, "getc",         mrb_posix_serial_getc, MRB_ARGS_NONE());
  mrb_define_method(mrb, cls, "gets",         mrb_posix_serial_gets, MRB_ARGS_NONE());
  mrb_define_method(mrb, cls, "read_timeout", mrb_posix_serial_read_timeout, MRB_ARGS_REQ(1));
}

//
//
//
void mrb_mruby_posix_serial_gem_final(mrb_state *mrb)
{
}


