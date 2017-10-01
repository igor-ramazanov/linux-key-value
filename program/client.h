#pragma once

enum error {
    SUCCESS,
    NETLINK_INIT_ERROR
};

int client_init(void);

void client_close(void);

void client_set(char *key, char *data);

int client_get(char *key, char** data, size_t *data_size);