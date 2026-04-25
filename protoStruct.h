//work in progresso



#ifndef PROTOSTRUCT_H
#define PROTOSTRUCT_H

// ------------------ ENUMS ------------------

typedef enum {
    TRANSPORT_TCP,
    TRANSPORT_UDP,

    TRANSPORT_INVALID
} transport_type_t;

typedef enum {
    ENCODING_BINARY,

    ENCODING_INVALID
} encoding_type_t;

typedef enum {
    ENDIAN_BIG,
    ENDIAN_LITTLE,

    ENDIAN_INVALID
} encoding_endianness_t;

typedef enum {
    PAYLOAD_RAW,      // bytes grezzi
    PAYLOAD_STRING,   // UTF-8 string

    PAYLOAD_INVALID
} payload_type_t;

typedef enum {
    FIELD_UINT8,
    FIELD_UINT16,
    FIELD_UINT32,
    FIELD_UINT64,

    FIELD_INT8,
    FIELD_INT16,
    FIELD_INT32,
    FIELD_INT64,

    FIELD_INVALID
} field_type_t;

// ------------------ DEFINITION ------------------
typedef struct {
    transport_type_t type;
    int default_port;
} transport_t;

typedef struct {
    encoding_type_t type;
    encoding_endianness_t endianness;
} encoding_t;
// ------------------ HEADER ------------------
typedef struct {
    char name[16];
    field_type_t type;
} header_field_t;

typedef struct {
    int size_bytes;
    header_field_t *fields;
    int field_count;
} header_t;
// ------------------ PAYLOAD ------------------
typedef struct {
    payload_type_t type;
    int max_size_bytes;
} payload_t;
// ------------------ MAIN PROTOCOL ------------------
typedef struct {
    uint8_t major;
    uint8_t minor; //es 2.1 2major 1minor
} version_t;

typedef struct {
    char name[16];
    version_t version;
    transport_t transport;
    encoding_t encoding;
    header_t header;
    payload_t payload;
} protocol_t;

#endif
