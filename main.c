/** https://stackoverflow.com/questions/21955446/how-to-use-fan-deny-fanotify */

#define _GNU_SOURCE
#define _ATFILE_SOURCE
#include <linux/fanotify.h>
#include <sys/fanotify.h>
#include <errno.h>
#include <inttypes.h>
#include <fcntl.h>
#include <linux/limits.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


#include "yaml.h"
#include "process.h"

//#define FANOTIFY_ARGUMENTS "cdfhmnp"
int fan_fd;

/* Mark a object */
int mark_object(int fan_fd, const char *path, int fd, uint64_t mask, unsigned int flags) {
  return fanotify_mark(fan_fd, flags, mask, fd, path);
}

/* Usage */
int usage() {
  fprintf(stdout, "Application for deny acess to your files.\nUsage: [file]\n");
}


/* Main */
int main() {
  struct Conf_t *conf = read_config("./config.yaml");
  if (conf == NULL) {
      fprintf (stderr, "не могу открыть файл с конфигурацией ./config.yaml\n");
      return 1;
  }

  /* Fanotify options */
  uint64_t fan_mask = FAN_OPEN | FAN_CLOSE | FAN_ACCESS | FAN_MODIFY | FAN_EVENT_ON_CHILD | FAN_ALL_PERM_EVENTS;
  unsigned int mark_flags = FAN_MARK_ADD, init_flags = 0;

  /* Other declarations */
  ssize_t len;
  char buf[4096];
  fd_set rfds;

  /* Initalize Fanotify */
  if(fan_mask & FAN_ALL_PERM_EVENTS)
    init_flags |= FAN_CLASS_CONTENT;
  else
    init_flags |= FAN_CLASS_NOTIF;

  fan_fd = fanotify_init(init_flags, O_RDONLY | O_LARGEFILE);
  if (fan_fd < 0) {
    fprintf(stderr, "%s\n", strerror(errno));
    return 1;
  }

  /* Mark object/Initalize object control with Fanotify */
  for (int i = 0 ; i < conf->files_count ; i++) {
    if (mark_object(fan_fd, conf->files[i], AT_FDCWD, fan_mask, mark_flags) != 0) {
      fprintf(stderr, "%s\n", strerror(errno));
      return 1;
    }
  }

  /** for (; optind < argc; optind++) { */
  /**   if (mark_object(fan_fd, argv[optind], AT_FDCWD, fan_mask, mark_flags) != 0) { */
  /**     fprintf(stderr, "%s\n", strerror(errno)); */
  /**     return 1; */
  /**   } */
  /** } */

  /* Restore fan_fd variable */
  FD_ZERO(&rfds);
  FD_SET(fan_fd, &rfds);

  /* Loop for start another loop for check fanotify events */
  while ((len = read(fan_fd, buf, sizeof(buf))) > 0) {
    /* Declarations */
    struct fanotify_event_metadata *metadata;
    struct fanotify_response response;
    char path[PATH_MAX];
    int path_len;
    metadata = (void *)buf;

    /* New loop for check fanotify events */ 
    while(FAN_EVENT_OK(metadata, len)) {
      /* Check if fanotify version are too old */
      if(metadata->vers < 2) {
        fprintf(stderr, "Kernel fanotify version too old\n");
        return 1;
      }

      /* Получение пути к файлу по его дескриптору */
      if (metadata->fd >= 0) {
        sprintf(path, "/proc/self/fd/%d", metadata->fd);

        path_len = readlink(path, path, sizeof(path)-1);
        if (path_len < 0) {
          fprintf(stderr, "%s\n", strerror(errno));
          return 1;
        }

        path[path_len] = '\0';
        fprintf(stdout, "%s:", path);
      }

      /** if(!strcmp(path, argv[1])) {  */
      /**   response.fd = metadata->fd; */
      /**   response.response = FAN_DENY; */
      /**   write(fan_fd, &response, sizeof(struct fanotify_response)); */
      /** } */

      /* Pid of acesser */
      fprintf(stdout, " pid=%ld\n", (long)metadata->pid);

      // получаю имя программы которая запросила доступ к файлу
      char * process_name = get_process_name(metadata->pid);
      char access = 0;
      for (int i = 0; i < conf->exclude_count; i++) {
        if (strcmp(process_name,conf->exclude[i]) == 0 ) {
          access = 1;
        }
      }

      free(process_name);

      write_process_info(metadata->pid);

      /** kill(metadata->pid, SIGKILL); */

      if (access) { 
        fprintf(stdout, "access accept\n");
        response.fd = metadata->fd;
        response.response = FAN_ACCESS;
        write(fan_fd, &response, sizeof(struct fanotify_response));
      } else {
        fprintf(stdout, "access forbiden\n");
        response.fd = metadata->fd;
        response.response = FAN_DENY;
        write(fan_fd, &response, sizeof(struct fanotify_response));
      } 

      /* Check actions by acesser and kill acesser */
      if(metadata->mask & FAN_ACCESS) {
        fprintf(stdout, " access");
        /** if(!strcmp(path, argv[1])) {  */
        /**   response.fd = metadata->fd; */
        /**   response.response = FAN_DENY; */
        /**   write(fan_fd, &response, sizeof(struct fanotify_response)); */
        /** } */
      }

      if(metadata->mask & FAN_OPEN) {
        fprintf(stdout, " open");
        /** if(!strcmp(path, argv[1])) {  */
        /**   response.fd = metadata->fd; */
        /**   response.response = FAN_DENY; */
        /**   write(fan_fd, &response, sizeof(struct fanotify_response)); */
        /** } */
      }

      if(metadata->mask & FAN_MODIFY) {
        fprintf(stdout, " modify");
        /** if(!strcmp(path, argv[1])) {  */
        /**   response.fd = metadata->fd; */
        /**   response.response = FAN_DENY; */
        /**   write(fan_fd, &response, sizeof(struct fanotify_response)); */
        /** } */
      }

      if(metadata->mask & FAN_CLOSE) {
        if(metadata->mask & FAN_CLOSE_WRITE)
          fprintf(stdout, " close(writable)");
      }

      /* Setup the output */
      fprintf(stdout, "\n");
      fflush(stdout);

      /* Check for internal error */
      if(metadata->fd >= 0 && close(metadata->fd) != 0) {
        fprintf(stderr, "%s\n", strerror(errno));
        return 1;
      }

      /* Next fanotify event.. */
      metadata = FAN_EVENT_NEXT(metadata, len);
    } // end of loop 2
  } // end of loop 1

  /* Check for len error */
  if(len < 0) {
    fprintf(stderr, "%s\n", strerror(errno));
    return 1;
  }

  return 0;
}

