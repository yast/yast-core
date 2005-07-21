#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>
#define MD_MAJOR 9
#include <linux/raid/md_u.h>

#ifndef RAID_AUTORUN
#define RAID_AUTORUN           _IO (MD_MAJOR, 0x14)
#endif

int main ()
{
  int result;

  int fd = open ("/dev/md0", O_RDWR, 0);

  result = ioctl(fd , RAID_AUTORUN, 0);
  close(fd);

  return result;
}
