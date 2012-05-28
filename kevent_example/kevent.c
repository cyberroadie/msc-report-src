#include <sys/event.h>
#include <sys/time.h> 
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h> 

int
main(void)
{
   int f, kq, nev;
   struct kevent change;
   struct kevent event;

   kq = kqueue();
   if (kq == -1)
       perror("kqueue");

   f = open("/tmp/foo", O_RDONLY);
   if (f == -1)
       perror("open");

   EV_SET(&change, f, EVFILT_VNODE,
          EV_ADD | EV_ENABLE | EV_ONESHOT,
          NOTE_DELETE | NOTE_EXTEND | NOTE_WRITE | NOTE_ATTRIB,
          0, 0);

   for (;;) {
       nev = kevent(kq, &change, 1, &event, 1, NULL);
       if (nev == -1)
           perror("kevent");
       else if (nev > 0) {
           if (event.fflags & NOTE_DELETE) {
               printf("File deleted\n");
               break;
           }
           if (event.fflags & NOTE_EXTEND ||
               event.fflags & NOTE_WRITE)
               printf("File modified\n");
           if (event.fflags & NOTE_ATTRIB)
               printf("File attributes modified\n");
       }
   }

   close(kq);
   close(f);
   return EXIT_SUCCESS;
}

