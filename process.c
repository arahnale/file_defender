#include "process.h"

void write_process_info(int pid) {
    char proc_file[256];
    snprintf(proc_file, sizeof(proc_file), "/proc/%d/cmdline", pid);

    // Открываем файл /proc/<pid>/stat для чтения
    FILE* proc_cmdline = fopen(proc_file, "r");
    if (proc_cmdline == NULL) {
        fprintf(stderr, "Ошибка при открытии файла %s\n", proc_file);
        return;
    }

    // Считываем имя и PID процесса из файла /proc/<pid>/stat
    char process_name[256];
    int nbytesread = fread(process_name, 1, 256, proc_cmdline);
    process_name[255] = '\0';

    /** fscanf(proc_cmdline, "%s", process_name); */
    printf("Имя процесса: %s\n", process_name);
    printf("PID: %d\n", pid);
    /** fprintf(file, "Имя процесса: %s\n", process_name); */
    /** fprintf(file, "PID: %d\n", pid); */

    fclose(proc_cmdline);

    // Открываем файл /proc/<pid>/stat для чтения
    snprintf(proc_file, sizeof(proc_file), "/proc/%d/status", pid);
    FILE* proc_status = fopen(proc_file, "r");
    if (proc_status == NULL) {
        fprintf(stderr, "Ошибка при открытии файла %s\n", proc_file);
        return;
    }

    // Считываем PPID родительского процесса
    int ppid = -1;
    char line[256];
    while (fgets(line, sizeof(line), proc_status) != NULL) {
        if (strncmp(line, "PPid:", 5) == 0) {
            sscanf(line + 5, "%d", &ppid);
            break;
        }
    }
    if (ppid == -1) {
        fprintf(stderr, "Не удалось найти PPid в файле %s\n", proc_file);
        return;
    }

    fclose(proc_status);

    // Рекурсивно записываем информацию о родительском процессе
    if (ppid != 0) {
        write_process_info(ppid);
    }
}

char * get_process_name(int pid) {
    char proc_file[256];
    snprintf(proc_file, sizeof(proc_file), "/proc/%d/cmdline", pid);

    // Открываем файл /proc/<pid>/stat для чтения
    FILE* proc_cmdline = fopen(proc_file, "r");
    if (proc_cmdline == NULL) {
        fprintf(stderr, "Ошибка при открытии файла %s\n", proc_file);
        return NULL;
    }

    // Считываем имя и PID процесса из файла /proc/<pid>/stat
    char process_name[256];
    int nbytesread = fread(process_name, 1, 256, proc_cmdline);
    process_name[255] = '\0';

    char * ret = malloc( sizeof(char) * strlen(process_name) );

    ret = strdup(process_name);

    return ret;
}

#ifdef QA
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s <PID>\n", argv[0]);
        return 1;
    }

    int pid = atoi(argv[1]);

    // Открываем файл для записи
    FILE* file = fopen("process_info.txt", "a");
    if (file == NULL) {
        fprintf(stderr, "Ошибка при создании файла\n");
        return 1;
    }

    write_process_info(pid);

    // Закрываем файл
    fclose(file);

    printf("Информация о процессе успешно записана в файл process_info.txt\n");

    return 0;
}
#endif
