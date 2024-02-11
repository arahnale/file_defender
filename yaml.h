#include <stdio.h>
#include <yaml.h>
#include <string.h>

struct Conf_t {
  char** files;                 // список файлов за которыми требуется следить
  unsigned int files_count;     // количество файлов за которыми требуется следить
  char*  log;                   // файл лога куда буду писать информацию по программам которые попытались получить доступ к файлам
  char** exclude;               // исключение программ которым можно получать доступ к файлам
  unsigned int exclude_count;   // количество файлов для исключения за слежением
};

struct Conf_t* new_conf_t();

int conf_t_files_append(struct Conf_t* conf, char * file);

int conf_t_exclude_append(struct Conf_t* conf, char * file);

struct Conf_t* read_config(char *filename );
