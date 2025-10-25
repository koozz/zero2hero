#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#include "common.h"
#include "parse.h"

void list_employees(struct dbheader_t *dbhdr, struct employee_t *employees) {

}

int add_employee(struct dbheader_t *dbhdr, struct employee_t *employees, char *addstring) {

}

int read_employees(int fd, struct dbheader_t *dbhdr, struct employee_t **employeesOut) {

}

int output_file(int fd, struct dbheader_t *dbhdr, struct employee_t *employees) {
  if (fd < 0) {
    fprintf(stderr, "Error: Invalid file descriptor.\n");
    return STATUS_ERROR;
  }
  dbhdr->magic = htonl(dbhdr->magic);
  dbhdr->version = htons(dbhdr->version);
  dbhdr->count = htons(dbhdr->count);
  dbhdr->filesize = htonl(dbhdr->filesize);
  if (lseek(fd, 0, SEEK_SET) == -1) {
    perror("lseek");
    return STATUS_ERROR;
  }
  if (write(fd, dbhdr, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
    perror("write");
    return STATUS_ERROR;
  }
  return STATUS_SUCCESS;
}

int validate_db_header(int fd, struct dbheader_t **headerOut) {
  if (fd < 0) {
    fprintf(stderr, "Error: Invalid file descriptor.\n");
    return STATUS_ERROR;
  }
  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == -1) {
    perror("calloc");
    return STATUS_ERROR;
  }
  if (read(fd, header, sizeof(struct dbheader_t)) != sizeof(struct dbheader_t)) {
    perror("read");
    free(header);
    return STATUS_ERROR;
  }
  header->magic = ntohl(header->magic);
  header->version = ntohs(header->version);
  header->count = ntohs(header->count);
  header->filesize = ntohl(header->filesize);

  if (header->magic != HEADER_MAGIC) {
    fprintf(stderr, "Error: Invalid database file (bad magic number).\n");
    free(header);
    return STATUS_ERROR;
  }
  if (header->version != 1) {
    printf("Improper header version\n");
    free(header);
    return STATUS_ERROR;
  }
  // if (header->count < 0 || header->count > MAX_EMPLOYEES)

  // if (header->filesize < sizeof(struct dbheader_t) || header->filesize > MAX_FILESIZE)

  *headerOut = header;

  return STATUS_SUCCESS;
}

int create_db_header(int fd, struct dbheader_t **headerOut) {
  struct dbheader_t *header = calloc(1, sizeof(struct dbheader_t));
  if (header == -1) {
    perror("calloc");
    return STATUS_ERROR;
  }
  header->version = 0x1;
  header->count = 0;
  header->magic = HEADER_MAGIC;
  header->filesize = sizeof(struct dbheader_t);

  *headerOut = header;

  return STATUS_SUCCESS;
}


