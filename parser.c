//work in progresso


include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <cjson/cJSON.h>

transport_type_t parse_transport(const char *s) {
    if (strcmp(s, "tcp") == 0 || strcmp(s, "TRANSPORT_TCP") == 0)
        return TRANSPORT_TCP;

    if (strcmp(s, "udp") == 0 || strcmp(s, "TRANSPORT_UDP") == 0)
        return TRANSPORT_UDP;

    return TRANSPORT_INVALID; //se restituisce null allora ERRORE
}

encoding_type_t parse_encoding(const char *s) {
    if (strcmp(s, "binary") == 0 || strcmp(s, "ENCODING_BINARY") == 0)
        return ENCODING_BINARY;

    return ENCODING_INVALID;
}

encoding_endianness_t parse_endianness(const char *s) {
    if (strcmp(s, "big") == 0 || strcmp(s, "ENDIAN_BIG") == 0)
        return ENDIAN_BIG;

    if (strcmp(s, "little") == 0 || strcmp(s, "ENDIAN_LITTLE") == 0) //SCONSIGLIATA NEL NETWORKING FANCULO I PICCOLI INDIANI
        return ENDIAN_LITTLE;

    return ENDIAN_INVALID;
}

payload_type_t parse_payload(const char *s) {
    if (strcmp(s, "string") == 0 || strcmp(s, "PAYLOAD_STRING") == 0)
        return PAYLOAD_STRING;

    if (strcmp(s, "raw") == 0 || strcmp(s, "PAYLOAD_RAW") == 0)
        return PAYLOAD_RAW;

    return PAYLOAD_INVALID;
}

header_field_t* parse_fields(cJSON *array, int *count) {
    if (!array || !count) return FIELD_INVALID;

    int n = cJSON_GetArraySize(array);
    *count = n;

    header_field_t *fields = calloc(n, sizeof(header_field_t));
    if (!fields) return FIELD_INVALID;

    for (int i = 0; i < n; i++) {
        cJSON *f = cJSON_GetArrayItem(array, i);
        if (!f) goto error;

        cJSON *name_item = cJSON_GetObjectItem(f, "name");
        cJSON *type_item = cJSON_GetObjectItem(f, "type");

        if (!name_item || !type_item) goto error;
        if (!name_item->valuestring || !type_item->valuestring) goto error;

        strncpy(fields[i].name, name_item->valuestring, sizeof(fields[i].name) - 1);

        fields[i].name[sizeof(fields[i].name) - 1] = '\0';

        const char *type = type_item->valuestring;

        if (strcmp(type, "uint8") == 0 || strcmp(type, "FIELD_UINT8") == 0)
            fields[i].type = FIELD_UINT8;
        else if (strcmp(type, "uint16") == 0 || strcmp(type, "FIELD_UINT16") == 0)
            fields[i].type = FIELD_UINT16;
        else if (strcmp(type, "uint32") == 0 || strcmp(type, "FIELD_UINT32") == 0)
            fields[i].type = FIELD_UINT32;
        else if (strcmp(type, "uint64") == 0 || strcmp(type, "FIELD_UINT64") == 0)
            fields[i].type = FIELD_UINT64;
        else
            goto error;
    }

    return fields;

    error:
    free(fields);
    return FIELD_INVALID;
}

version_t parse_version(const char *s) {
    version_t v = {1, 0};

    if (!s) return v;

    char *dot = strchr(s, '.');
    if (!dot) return v;

    v.major = (uint8_t)atoi(s);
    v.minor = (uint8_t)atoi(dot + 1);

    return v;
}

protocol_t* parse_protocol(const char *json_str) {
    if (!json_str) return NULL;

    cJSON *root = cJSON_Parse(json_str);
    if (!root) return NULL;

    protocol_t *p = calloc(1, sizeof(protocol_t));
    if (!p) {
        cJSON_Delete(root);
        return NULL;
    }

    // ---------------- NAME ----------------
    cJSON *name = cJSON_GetObjectItem(root, "name");
    if (!name || !name->valuestring) goto error;
    strncpy(p->name, name->valuestring, sizeof(p->name)-1);

    // ---------------- VERSION ----------------
    cJSON *version = cJSON_GetObjectItem(root, "version");
    if (!version || !version->valuestring) goto error;
    p->version = parse_version(version->valuestring);

    // ---------------- TRANSPORT ----------------
    cJSON *transport = cJSON_GetObjectItem(root, "transport");
    if (!transport) goto error;

    cJSON *t_type = cJSON_GetObjectItem(transport, "type");
    cJSON *t_port = cJSON_GetObjectItem(transport, "default_port");

    if (!t_type || !t_type->valuestring) goto error;

    p->transport.type = parse_transport(t_type->valuestring);
    if (p->transport.type == TRANSPORT_INVALID) goto error;

    if (t_port) p->transport.default_port = t_port->valueint;

    // ---------------- ENCODING ----------------
    cJSON *encoding = cJSON_GetObjectItem(root, "encoding");
    if (!encoding) goto error;

    cJSON *e_type = cJSON_GetObjectItem(encoding, "type");
    cJSON *e_endian = cJSON_GetObjectItem(encoding, "endianness");

    if (!e_type || !e_endian) goto error;

    p->encoding.type = parse_encoding(e_type->valuestring);
    p->encoding.endianness = parse_endianness(e_endian->valuestring);

    if (p->encoding.type == ENCODING_INVALID ||
        p->encoding.endianness == ENDIAN_INVALID)
        goto error;

    // ---------------- HEADER ----------------
    cJSON *header = cJSON_GetObjectItem(root, "header");
    if (!header) goto error;

    cJSON *size_bytes = cJSON_GetObjectItem(header, "size_bytes");
    cJSON *fields = cJSON_GetObjectItem(header, "fields");

    if (!size_bytes || !fields) goto error;

    p->header.size_bytes = size_bytes->valueint;

    p->header.fields = parse_fields(fields, &p->header.field_count);
    if (!p->header.fields) goto error;

    // ---------------- PAYLOAD ----------------
    cJSON *payload = cJSON_GetObjectItem(root, "payload");
    if (!payload) goto error;

    cJSON *p_type = cJSON_GetObjectItem(payload, "type");
    cJSON *p_max = cJSON_GetObjectItem(payload, "max_size_bytes");

    if (!p_type || !p_type->valuestring) goto error;

    p->payload.type = parse_payload(p_type->valuestring);
    if (p->payload.type == PAYLOAD_INVALID) goto error;

    if (p_max) p->payload.max_size_bytes = p_max->valueint;

    // ---------------- CLEAN ----------------
    cJSON_Delete(root);
    return p;

    error:
    if (p) {
        if (p->header.fields)
            free(p->header.fields);
        free(p);
    }
    cJSON_Delete(root);
    return NULL;
}
