#include "yaml.h"


struct Conf_t* new_conf_t() {
    struct Conf_t* conf = malloc(sizeof(struct Conf_t));
    conf->files = NULL;
    conf->files_count = 0;
    conf->log = NULL;
    conf->exclude = NULL;
    conf->exclude_count = 0;

    return conf;
}

int conf_t_files_append(struct Conf_t* conf, char * file) {
  conf->files_count++;
  conf->files = realloc(conf->files, conf->files_count * sizeof(char*));
  if (conf->files == NULL) {
    return 1;
  }
  size_t size = strlen(file) + 1;
  conf->files[conf->files_count - 1] = malloc(size);
  if (strcpy(conf->files[conf->files_count - 1], file) == NULL) {
    return 1;
  }

  return 0;
}

int conf_t_exclude_append(struct Conf_t* conf, char * file) {
  conf->exclude_count++;
  conf->exclude = realloc(conf->exclude, conf->exclude_count * sizeof(char*));
  if (conf->exclude == NULL) {
    return 1;
  }
  size_t size = strlen(file) + 1;
  conf->exclude[conf->exclude_count - 1] = malloc(size);
  if (strcpy(conf->exclude[conf->exclude_count - 1], file) == NULL) {
    return 1;
  }

  return 0;
}


struct Conf_t* read_config(char *filename ) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        printf("Не удалось открыть файл с конфигурацией\n");
        return NULL;
    }

    // Инициализируем парсер YAML
    yaml_parser_t parser;
    yaml_event_t event;
    if (!yaml_parser_initialize(&parser)) {
        printf("Не удалось инициализировать парсер YAML\n");
        return NULL;
    }

    // Устанавливаем источник данных для парсера YAML
    yaml_parser_set_input_file(&parser, file);
    int yaml_list = 0;
    char * yaml_key = NULL;

    struct Conf_t* conf = new_conf_t();

    // Читаем события из YAML файла
    do {
        // Получаем следующее событие
        if (!yaml_parser_parse(&parser, &event)) {
            printf("Ошибка при чтении события YAML\n");
            return NULL;
        }

        // Обрабатываем событие в зависимости от его типа
        switch (event.type) {
            case YAML_SCALAR_EVENT:
              if (yaml_list == 0) {
                if (strcmp(event.data.scalar.value, "files") == 0) {
                  yaml_key = "files";
                  break;
                } else if (strcmp(event.data.scalar.value, "log") == 0) {
                  yaml_key = "log";
                  break;
                } else if (strcmp(event.data.scalar.value, "exclude") == 0) {
                  yaml_key = "exclude";
                  break;
                }
              }

              if (strcmp(yaml_key, "files") == 0 && yaml_list == 1) {
                if (conf_t_files_append(conf, event.data.scalar.value) != 0) {
                  return NULL;
                }
              }

              if (strcmp(yaml_key, "exclude") == 0 && yaml_list == 1) {
                if (conf_t_exclude_append(conf, event.data.scalar.value) != 0) {
                  return NULL;
                }
              }

              /** printf("YAML_SCALAR_EVENT %s %s\n", yaml_key, event.data.scalar.value ); */
              break;
            case YAML_SEQUENCE_START_EVENT:
              yaml_list = 1;
              /** printf("Начало списка\n"); */
              break;
            case YAML_SEQUENCE_END_EVENT:
              yaml_list = 0;
              /** printf("Конец списка\n"); */
              break;
        }

        if(event.type != YAML_STREAM_END_TOKEN)
          yaml_event_delete(&event);
        // Освобождаем ресурсы, выделенные под событие
    } while (event.type != YAML_STREAM_END_EVENT);

    // Завершаем работу с парсером YAML
    yaml_parser_delete(&parser);

    // Закрываем файл
    fclose(file);

    return conf;
}

#ifdef QA
int main() {
    // Открываем файл с YAML конфигурацией
    /** struct Conf_t *c = readConf("./config.yaml"); */
    struct Conf_t *conf = read_config("./config.yaml");

    if (conf == NULL) {
      return 1;
    }

    for (int i = 0 ; i < conf->files_count; i++) {
      printf("file %s\n" , conf->files[i]);
    }

    for (int i = 0 ; i < conf->exclude_count; i++) {
      printf("exclude %s\n" , conf->exclude[i]);
    }
}
#endif
