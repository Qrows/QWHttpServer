#include "config.h"

static int split_line(char *cfgline, size_t size, char **name, char **value)
{
	char *ptr = NULL;
	*name = cfgline;
	for (ptr = cfgline; *ptr != '\0' && *ptr != ' '; ++ptr);
	*ptr = '\0';
	// trim whitespace 
	for (++ptr; *ptr != '\0' && (*ptr == ' ' || *ptr == '\t'); ++ptr);
	*value = ptr;
	// trim ending whitespace
	for (; *ptr != '\0' && *ptr != ' ' && *ptr != '\n'; ++ptr);
	*ptr = '\0';
	return 0;
}

static int parse_value(struct config *cfg, char *name, char *value)
{
	char *errptr = NULL;
	if (strcmp(name, "connection_write_buffer") == 0) {
		cfg->connection_write_buffer = strtoul(value, &errptr, 10);
		if (*errptr != '\0')
			return -1;
	} else if (strcmp(name, "connection_read_buffer") == 0) {
		cfg->connection_read_buffer = strtoul(value, &errptr, 10);
		if (*errptr != '\0')
			return -1;
	} else if (strcmp(name, "server_root") == 0) {
		cfg->server_root = strdup(value);
		if (cfg->server_root == NULL)
			return -1;
	} else if (strcmp(name, "server_cache") == 0) {
		cfg->server_cache = strdup(value);
		if (cfg->server_cache == NULL)
			return -1;
	} else if (strcmp(name, "thread_number") == 0) {
		cfg->thread_number = strtol(value, &errptr, 10);
		if (*errptr != '\0' || cfg->thread_number < 0)
			return -1;
	} else if (strcmp(name, "timeout") == 0) {
		cfg->timeout = strtol(value, &errptr, 10);
		if (*errptr != '\0' || cfg->thread_number < 0)
			return -1;
	} else if (strcmp(name, "port") == 0) {
		cfg->port = strdup(value);
		if (cfg->port == NULL)
			return -1;
	} else if (strcmp(name, "backlog") == 0) {
		cfg->backlog = strtol(value, &errptr, 10);
		if (*errptr != '\0' || cfg->backlog < 0)
			return -1;
	} else if (strcmp(name, "http_max_request_size") == 0) {
		cfg->http_max_request_size = strtol(value, &errptr, 10);
		if (*errptr != '\0') 
			return -1;
	} else if (strcmp(name, "http_max_response_size") == 0) {
		cfg->http_max_response_size = strtol(value, &errptr, 10);
		if (*errptr != '\0')
			return -1;
	} else {
		return -1;
	}
	return 0;
}


static void clean_cfg(struct config *cfg)
{
	if (cfg == NULL)
		return;
	if (cfg->server_root)
		free(cfg->server_root);
	if (cfg->server_cache)
		free(cfg->server_cache);
	if (cfg->port)
		free(cfg->port);
}

static int cfg_set_default(struct config *cfg)
{
	if (cfg == NULL)
		return -1;
	if (cfg->connection_write_buffer == 0)
		cfg->connection_write_buffer = 8192;
	if (cfg->connection_read_buffer == 0)
		cfg->connection_read_buffer = 8192;
	if (cfg->thread_number == 0)
		cfg->thread_number = 1;
	if (cfg->backlog == 0)
		cfg->backlog = 10;
	if (cfg->http_max_request_size == 0)
		cfg->http_max_request_size = 8192;
	if (cfg->http_max_response_size == 0)
		cfg->http_max_response_size = 8192;
	if (cfg->timeout == 0)
		cfg->timeout = 5;
	if (cfg->server_root == NULL) {
		cfg->server_root = strdup("root");
		if (cfg->server_root == NULL) {
			return -1;
		}
	}
	if (cfg->server_cache == NULL) {
		cfg->server_cache = strdup("cache");
		if (cfg->server_cache == NULL) {
			free(cfg->server_root);
			return -1;
		}
	}
	if (cfg->port == NULL) {
		cfg->port = strdup("80");
		if (cfg->port == NULL) {
			free(cfg->server_root);
			free(cfg->server_cache);
			return -1;
		}
	}
	return 0;
}

static int parse_cfg_file(FILE *fcfg, struct config *cfg)
{
	char *line = NULL;
	size_t line_size = 0;
	int res = 0;
	ssize_t rd = 0;
	char *name = NULL;
	char *value = NULL;
	int nline = 1;
	memset(cfg, 0, sizeof(*cfg));
	while ((rd = getline(&line, &line_size, fcfg)) >= 0) {
		// parse line: name value
		// skip for comments and newline
		if (*line == '#' || *line == '\n') {
			free(line);
			line_size = 0;
			continue;
		}
		res = split_line(line, line_size, &name, &value);
		if (res < 0) {
			syslog(LOG_ERR, "%s %d\n",
			       "invalid line in config file at line",
			       nline);
			free(line);
			clean_cfg(cfg);
			return -1;
		}
		res = parse_value(cfg, name, value);
		if (res < 0) {
			syslog(LOG_ERR, "%s %d\n",
			       "invalid line in config file at line",
			       nline);
			free(line);
			clean_cfg(cfg);
			return -1;
		}
		++nline;
	}
	free(line);
	res = cfg_set_default(cfg);
	if (res < 0)
		return -1;
	return 0;
}


struct config *create_config(char *cfg_file)
{
	struct config *cfg = NULL;
	FILE *fcfg  = NULL;
	int res = 0;
	if (cfg_file == NULL)
		return NULL;
	cfg = malloc(sizeof(*cfg));
	if (cfg == NULL)
		return NULL;
	fcfg = fopen(cfg_file, "r");
	if (fcfg == NULL) {
		free(cfg);
		return NULL;
	}
	res = parse_cfg_file(fcfg, cfg);
	if (res < 0) {
		free(cfg);
		fclose(fcfg);
		return NULL;
	}
	fclose(fcfg);
	return cfg;
		
}
