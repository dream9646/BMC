// Topic: GPIO sysfs basic read
// Note: First practice from Device-Tree/GPIO Week 1

#include <stdio.h>
#include <fcntl.h>   // open(), O_RDONLY
#include <unistd.h>  // read(), close()

// Read a GPIO value (very simple practice)
int SimpleGpioRead(int gpio) {
  char path[64];
  snprintf(path, sizeof(path),
           "/sys/class/gpio/gpio%d/value", gpio);

  int fd = open(path, O_RDONLY);
  if (fd < 0) {
    perror("open");
    return -1;
  }

  char ch;
  if (read(fd, &ch, 1) < 0) {
    perror("read");
    close(fd);
    return -1;
  }

  close(fd);
  return ch == '1' ? 1 : 0;
}

int main(void) {
  int gpio = 37;
  int value = SimpleGpioRead(gpio);
  if (value < 0) {
    // TODO: error handling
    return 1;
  }

  printf("GPIO %d = %d\n", gpio, value);
  return 0;
}
