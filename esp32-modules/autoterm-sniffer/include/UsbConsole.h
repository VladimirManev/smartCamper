// Commands on the USB serial port (UART0). They never go out to AUTOTERM.

#ifndef AUTOTERM_USB_CONSOLE_H
#define AUTOTERM_USB_CONSOLE_H

#include "Sniffer.h"

class UsbConsole {
 public:
  explicit UsbConsole(Sniffer& sniffer);

  void begin();
  void poll();

 private:
  void handleLine(char* line);
  void printHelp() const;

  Sniffer& sniffer_;
  char line_[80];
  size_t length_;
  bool overflow_;
};

#endif
