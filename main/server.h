/**
 * @file server.h
 * @brief Web server
 * @author Milan Vodák <xvodak07@stud.fit.vut.cz>
 */

#define SERVER_BASE_PATH "/www"
#define SERVER_CHUNK_SIZE 1024

#define IS_EXTENSION(filename, ext) strcasecmp(&filename[strlen(filename) - strlen(ext)], ext) == 0

void server_init();
