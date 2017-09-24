#pragma once

enum error {
    SUCCESS,
    NETLINK_INIT_ERROR
};

int client_init(void);

void client_close(void);

void client_set(char *key, char *data);

void client_get(char *key);