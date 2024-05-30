/*
 *  Unidirectional Layer2 (Ethernet) Air-Gap -- tested on Raspberry Pi
 *
 *      You have to bridge eth0-tap0 and bridge tap1-eth1; then run this as root
 *      for tap0 --> tap1 unidirectional link
 *
 *      (C) 2024 Alin-Adrian Anton <alin.anton@cs.upt.ro>
 * 
 *      This program is free software: you can redistribute it and/or modify it under
 *      the terms of the GNU General Public License as published by the Free Software
 *      Foundation, either version 3 of the License, or (at your option) any later 
 *      version.
 *
 *      This program is distributed in the hope that it will be useful, but WITHOUT
 *      ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *      FOR A PARTICULAR PURPOSE.
 *
 *      See the GNU General Public License for more details.
 *      You should have received a copy of the GNU General Public License along with
 *      this program. If not, see <https://www.gnu.org/licenses/>. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/if.h>
#include <linux/if_tun.h>

#define max(a,b) ((a)>(b) ? (a):(b))

int tun_alloc(char *dev)
{
    struct ifreq ifr;
    int fd, err;

    if( (fd = open("/dev/net/tun", O_RDWR)) < 0 )
       return -1;

    memset(&ifr, 0, sizeof(ifr));

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    ifr.ifr_flags = IFF_TAP;
    if( *dev )
       strncpy(ifr.ifr_name, dev, IFNAMSIZ);

    if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ){
       close(fd);
       return err;
    }
    strcpy(dev, ifr.ifr_name);
    return fd;
}


int main(int argc, char *argv[])
{
   char buf[1600];
   char dev1, dev2;
   int f1,f2,l,fm, err;
   fd_set fds;
   struct ifreq ifr;
   char *pdev1, *pdev2;

   pdev1 = &dev1;
   pdev2 = &dev2;

   if( (f1 = open("/dev/net/tun", O_RDWR)) < 0 )
   return -1;

   memset(&ifr, 0, sizeof(ifr));

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    ifr.ifr_flags = IFF_TAP;
    if( pdev1 )
       strncpy(ifr.ifr_name, pdev1, IFNAMSIZ);

    if( (err = ioctl(f1, TUNSETIFF, (void *) &ifr)) < 0 ){
       close(f1);
       return err;
    }
    strcpy(pdev1, ifr.ifr_name);

   if( (f2 = open("/dev/net/tun", O_RDWR)) < 0 )
   return -1;

   memset(&ifr, 0, sizeof(ifr));

    /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
     *        IFF_TAP   - TAP device
     *
     *        IFF_NO_PI - Do not provide packet information
     */
    ifr.ifr_flags = IFF_TAP;
    if( pdev2 )
       strncpy(ifr.ifr_name, pdev2, IFNAMSIZ);

    if( (err = ioctl(f2, TUNSETIFF, (void *) &ifr)) < 0 ){
       close(f2);
       return err;
    }
    strcpy(pdev2, ifr.ifr_name);


   fm = max(f1, f2) + 1;

 //  ioctl(f1, TUNSETNOCSUM, 1); 
 //  ioctl(f2, TUNSETNOCSUM, 1); 

   while(1){
	FD_ZERO(&fds);
        FD_SET(f1, &fds);
        FD_SET(f2, &fds);

	select(fm, &fds, NULL, NULL, NULL);

	if( FD_ISSET(f1, &fds) ) {
	   l = read(f1,buf,sizeof(buf));
           write(f2,buf,l);
	   printf("Wrote %d bytes from tap0 to tap1\n", l);
	}
	/* only copy in one direction from f1 to f2
	 *
	 * bridge the f1 and f2 taps (tap0 and tap1) to real ethernet ports
	 *
	 * what you see back are ethernet frames so you can copy back
	 * tcp acknowledgements to allow feedback loop if you want that
	 * 
	 */
//	if( FD_ISSET(f2, &fds) ) {
//	   l = read(f2,buf,sizeof(buf));
//         write(f1,buf,l);
//	   printf("Wrote %d bytes from tap1 to tap0\n", l);
//	}
   }
}
