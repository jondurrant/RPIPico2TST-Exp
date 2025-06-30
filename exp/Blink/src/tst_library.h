/*
Copyright (c) 2025 TrueSmartTech OÜ  
All rights reserved.  

This code is proprietary and confidential. Redistribution, modification, or commercial use without explicit permission is strictly prohibited.  

Version 5.0.0
Author: TrueSmartTech OÜ
www.truesmarttech.com
*/

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "math.h"
#include <time.h>

#ifndef TSTLIBRARY_H
#define TSTLIBRARY_H

/**
 * @section ERROR_CODES Error Codes
 * @brief Standard error codes returned by TST library functions
 */
enum tstErrors
{
    TST_OK,                      ///< Operation completed successfully
    TST_NO_STRUCT_FOUND,         ///< Requested structure not found in registry
    TST_INVALID_POINTER,         ///< Invalid pointer parameter passed
    TST_STRUCT_ALREADY_PRESENT,  ///< Structure already registered
    TST_FAIL_ALLOCATE_MEMORY,    ///< Memory allocation failed
    TST_FAIL,                    ///< General failure
    TST_FAIL_UPDATE,             ///< Firmware update operation failed
    TST_FAIL_INVALID_PARAMETER,  ///< Invalid parameter provided
    TST_FAIL_NOT_INITIALIZED,    ///< System not properly initialized
    TST_FAIL_NOT_FOUND          ///< Requested item not found
};

/**
 * @section TIMER_SYSTEM Timer Management System
 * @brief Comprehensive timer system with callbacks and state management
 */

/**
 * @brief Callback function type for timer events
 * @param userData User-defined data passed to callback
 */
typedef void (*TSTTimerCallback)(void *userData);

/**
 * @brief Timer state enumeration
 */
typedef enum {
    TST_TIMER_ACTIVE,     ///< Timer is running and counting down
    TST_TIMER_PAUSED,     ///< Timer is paused (can be resumed)
    TST_TIMER_COMPLETED,  ///< Timer has expired (one-shot timers)
    TST_TIMER_CANCELLED   ///< Timer was stopped/cancelled
} TSTTimerState;

/**
 * @brief Timer type enumeration
 */
typedef enum {
    TST_TIMER_ONESHOT,   ///< Fires once then auto-removes
    TST_TIMER_PERIODIC   ///< Repeats until explicitly stopped
} TSTTimerType;

/**
 * @section DATA_STRUCTURES Core Data Structures
 * @brief Internal data structures for managing devices, interfaces, and data
 */

/**
 * @brief Linked list structure for message queues
 * Used for storing incoming and outgoing message buffers
 */
struct TSTList
{
    uint8_t *pData;          ///< Pointer to message data
    size_t sData;            ///< Size of message data in bytes
    struct TSTList *next;    ///< Pointer to next list element
};

/**
 * @brief Hash table node structure
 * Used in hash maps for fast lookups of devices, interfaces, and structures
 */
struct TSTNode
{
    uint32_t id;             ///< Hash ID for quick lookup
    void *data;              ///< Pointer to actual data
    size_t sData;            ///< Size of data in bytes
    struct TSTNode *next;    ///< Next node in collision chain
};

/**
 * @brief Hash map structure
 * Provides O(1) average lookup time for registered items
 */
struct TSTMap
{
    uint32_t size;           ///< Number of buckets in hash table
    uint32_t count;          ///< Number of items stored
    struct TSTNode **table;  ///< Array of bucket pointers
};

/**
 * @section PROTOCOL_MESSAGES Protocol Message Structures
 * @brief Packed structures for different message types in the protocol
 * All messages use __attribute__((packed)) to ensure consistent layout across platforms
 */

/**
 * @brief Online notification message
 * Sent when a device comes online
 */
typedef struct __attribute__((packed))
{
    uint8_t mode;           ///< Message type (ONLINE)
    uint16_t deviceId;      ///< Hash ID of the device
} tstMessageOnline;

/**
 * @brief Offline notification message
 * Sent when a device goes offline
 */
typedef struct __attribute__((packed))
{
    uint8_t mode;           ///< Message type (OFFLINE)
    uint16_t deviceId;      ///< Hash ID of the device
} tstMessageOffline;

/**
 * @brief File system operation request
 * Used for all file system operations (list, read, write, delete, etc.)
 */
typedef struct __attribute__((packed))
{
    uint8_t mode;           ///< Message type (FS)
    uint16_t deviceId;      ///< Target device hash ID
    uint8_t fsOperation;    ///< File system operation type (see tstFs enum)
    uint16_t pathLen;       ///< Length of file path
    uint32_t offset;        ///< File offset for read/write operations
    uint32_t dataSize;      ///< Size of data or requested data
    char path[];            ///< Variable length file path (null-terminated)
} tstMessageFsRequest;

/**
 * @brief File system operation response
 * Response to file system requests, may contain file data or directory listings
 */
typedef struct __attribute__((packed))
{
    uint8_t mode;           ///< Message type (FS)
    uint16_t deviceId;      ///< Source device hash ID
    uint8_t fsOperation;    ///< File system operation type
    uint8_t status;         ///< Success/error code (see tstErrors)
    uint32_t dataSize;      ///< Total data size (may span multiple fragments)
    uint8_t data[];         ///< File data or directory listing
} tstMessageFsResponse;

/**
 * @brief Variable get request message
 * Requests the current value of a variable from a remote device
 */
typedef struct __attribute__((packed))
{
    uint8_t mode;              ///< Message type (VARIABLEGET)
    uint16_t deviceId;         ///< Target device hash ID
    uint8_t total_fragments;   ///< Total number of fragments for this request
    uint8_t fragment_number;   ///< Current fragment number (0-based)
    uint16_t structId;         ///< Hash ID of the structure containing the variable
    uint16_t structOffset;     ///< Byte offset of variable within structure
    uint16_t variableSize;     ///< Size of variable in bytes
} tstMessageDataGet;

/**
 * @brief Variable set message
 * Sets the value of a variable on a remote device
 */
typedef struct __attribute__((packed))
{
    uint8_t mode;              ///< Message type (VARIABLESET)
    uint16_t deviceId;         ///< Target device hash ID
    uint8_t total_fragments;   ///< Total number of fragments for this message
    uint8_t fragment_number;   ///< Current fragment number (0-based)
    uint16_t structId;         ///< Hash ID of the structure containing the variable
    uint16_t structOffset;     ///< Byte offset of variable within structure
    uint16_t variableSize;     ///< Total size of variable in bytes
    uint8_t data[];            ///< Variable data (may be fragmented)
} tstMessageDataSet;

/**
 * @brief Monitor message
 * Used for sending text messages, logs, and debug information
 */
typedef struct __attribute__((packed))
{
    uint8_t mode;              ///< Message type (MONITOR)
    uint16_t deviceId;         ///< Source device hash ID
    uint8_t total_fragments;   ///< Total number of fragments
    uint8_t fragment_number;   ///< Current fragment number (0-based)
    uint16_t msgLen;           ///< Total message length in bytes
    char message[];            ///< Text message (may be fragmented)
} tstMessageMonitorSend;

/**
 * @brief Firmware update request
 * Used for sending firmware data to devices for over-the-air updates
 */
typedef struct __attribute__((packed))
{
    uint8_t mode;              ///< Message type (UPDATE)
    uint16_t deviceId;         ///< Target device hash ID
    uint8_t updateOperation;   ///< Update operation (START, DATA, END)
    uint32_t sequenceNumber;   ///< Sequence number for packet ordering
    uint32_t crc;              ///< CRC32 checksum for data validation
    uint32_t dataSize;         ///< Size of firmware data in this packet
    uint8_t data[];            ///< Firmware binary data
} tstMessageUpdateRequest;

/**
 * @brief Firmware update response
 * Acknowledgment of firmware update operations
 */
typedef struct __attribute__((packed))
{
    uint8_t mode;              ///< Message type (UPDATE)
    uint16_t deviceId;         ///< Source device hash ID
    uint8_t updateOperation;   ///< Update operation being acknowledged
    uint8_t status;            ///< Status code (TST_OK or error)
    uint32_t crc;              ///< CRC32 of received data
    uint32_t sequenceNumber;   ///< Sequence number being acknowledged
} tstMessageUpdateResponse;

/**
 * @section REGISTRY_STRUCTURES Device Registry Structures
 * @brief Structures for managing registered devices, interfaces, and data structures
 */

/**
 * @brief Structure registry node
 * Stores information about registered data structures
 */
typedef struct StructNode
{
    char *name;                ///< Name of the structure
    uint16_t structId;         ///< Hash ID of the structure name
    uint8_t *structPointer;    ///< Pointer to the actual structure instance
    size_t structSize;         ///< Size of the structure in bytes
} StructNode;

/**
 * @brief Interface registry node
 * Stores information about communication interfaces (UART, TCP, etc.)
 */
typedef struct InterfaceNode
{
    const char *interface;         ///< Interface name (e.g., "SERIAL", "TCP")
    uint16_t interfaceId;          ///< Hash ID of interface name
    size_t maxPayloadSize;         ///< Maximum payload size for this interface
    struct TSTList *tstMessageRxList;  ///< Receive message queue
    struct TSTList *tstMessageTxList;  ///< Transmit message queue
    uint8_t *reassemblyBuffer;     ///< Buffer for assembling fragmented messages
    size_t reassemblySize;         ///< Current size of data in reassembly buffer
    uint8_t expectedFragments;     ///< Number of fragments expected for current message
    uint8_t isOnline;              ///< Online status (1 = online, 0 = offline)
} InterfaceNode;

/**
 * @brief Device registry node
 * Top-level structure representing a registered device
 */
typedef struct DeviceInfo
{
    const char *device;            ///< Device name
    uint16_t deviceId;             ///< Hash ID of device name
    struct TSTMap *interfaceMap;   ///< Map of registered interfaces
    struct TSTMap *structMap;      ///< Map of registered data structures
} DeviceInfo;

/**
 * @section PROTOCOL_ENUMS Protocol Operation Enumerations
 * @brief Enumerations for various protocol operations
 */

/**
 * @brief File system operations
 */
enum tstFs
{
    LIST,      ///< List directory contents
    WRITE,     ///< Write data to file
    APPEND,    ///< Append data to file
    READ,      ///< Read data from file
    RENAME,    ///< Rename file or directory
    DELETE,    ///< Delete file or directory
    UPLOAD,    ///< Upload file to device
    DOWNLOAD   ///< Download file from device
};

/**
 * @brief Firmware update operations
 */
enum tstUpdate
{
    START,     ///< Begin firmware update process
    DATA,      ///< Send firmware data chunk
    END        ///< Complete firmware update process
};

/**
 * @brief Protocol message modes
 */
enum tstModes
{
    ONLINE,      ///< Device online notification
    OFFLINE,     ///< Device offline notification
    VARIABLEGET, ///< Request variable value
    VARIABLESET, ///< Set variable value
    MONITOR,     ///< Send monitor/debug message
    UPDATE,      ///< Firmware update operation
    FS          ///< File system operation
};

/**
 * @section CONFIGURATION_STRUCTURES Configuration Structures
 * @brief Structures for configuring devices, interfaces, and data structures
 */

/**
 * @brief Structure configuration
 * Used to register data structures with the protocol system
 */
typedef struct
{
    const char *name;          ///< Structure name for identification
    const char *structName;    ///< Structure type name (for debugging)
    void *pStruct;             ///< Pointer to structure instance
    size_t sStruct;            ///< Size of structure in bytes
} TST_StructConfig;

/**
 * @brief Interface configuration
 * Used to configure communication interfaces
 */
typedef struct
{
    const char *interface;     ///< Interface name (e.g., "SERIAL", "TCP")
    uint32_t maxSize;          ///< Maximum payload size for this interface
} TST_InterfaceConfig;

/**
 * @brief Device configuration
 * Complete configuration for a device including all interfaces and structures
 */
typedef struct
{
    const char *name;                    ///< Device name
    const TST_InterfaceConfig *pInterfaces;  ///< Array of interface configurations
    size_t nInterfaces;                  ///< Size of interface array in bytes
    const TST_StructConfig *pStructs;    ///< Array of structure configurations
    size_t nStructs;                     ///< Size of structure array in bytes
} TST_DeviceConfig;

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @section UTILITY_FUNCTIONS Utility Functions
 * @brief Helper functions for string hashing, structure management, etc.
 */

/**
 * @brief Calculate hash value for a string
 * @param String Input string to hash
 * @return 16-bit hash value
 */
uint16_t tstHashString(const char *String);

/**
 * @brief Search for a registered structure by name
 * @param map Structure map to search in
 * @param name Name of structure to find
 * @return Pointer to StructNode if found, NULL otherwise
 */
StructNode *searchStruct(struct TSTMap *map, const char *name);

/**
 * @brief Register a new device in the system
 * @param device Device name to register
 * @return TST_OK on success, error code on failure
 */
uint8_t tstRegisterDevice(const char *device);

/**
 * @brief Register a communication interface for a device
 * @param device Device name
 * @param interface Interface name
 * @param maxPayloadSize Maximum payload size for this interface
 * @return TST_OK on success, error code on failure
 */
uint8_t tstRegisterInterface(const char *device, const char *interface, uint32_t maxPayloadSize);

/**
 * @brief Register a data structure with a device
 * @param device Device name
 * @param structName Structure name
 * @param pStruct Pointer to structure instance
 * @param sStruct Size of structure in bytes
 * @return TST_OK on success, error code on failure
 */
uint8_t tstRegisterStruct(const char *device, const char *structName, void *pStruct, size_t sStruct);

/**
 * @brief Find structure ID and offset for a variable pointer
 * @param deviceHash Hash ID of device
 * @param pVar Pointer to variable
 * @param varStruct Output: structure name containing the variable
 * @param varAddress Output: offset of variable within structure
 * @return TST_OK if found, error code otherwise
 */
uint8_t tstFindstructIdAndstructOffset(uint16_t deviceHash, uint8_t *pVar, char **varStruct, uint16_t *varAddress);

/**
 * @brief Find variable pointer by device, structure, and offset
 * @param deviceId Device hash ID
 * @param structId Structure hash ID
 * @param structOffset Offset within structure
 * @return Pointer to variable if found, NULL otherwise
 */
uint8_t *tstFindVariable(uint16_t deviceId, uint16_t structId, uint16_t structOffset);

/**
 * @brief Create variable message (internal function)
 * @param device Device name
 * @param interface Interface name
 * @param pVar Pointer to variable
 * @param sVar Size of variable
 * @param pData Data to set (for SET operations)
 * @param command Command type (GET or SET)
 * @return TST_OK on success, error code on failure
 */
uint8_t tstVariablesCreateMessage(const char *device, const char *interface, void *pVar, size_t sVar, void *pData, uint8_t command);

/**
 * @brief Send online request
 * @param device Device name
 * @param interface Interface name
 * @return TST_OK on success, error code on failure
 */
uint8_t tstOnlineRequest(const char *device, const char *interface);

/**
 * @brief Send offline request
 * @param device Device name
 * @param interface Interface name
 * @return TST_OK on success, error code on failure
 */
uint8_t tstOfflineRequest(const char *device, const char *interface);

/**
 * @section CORE_FUNCTIONS Core Library Functions
 * @brief Main functions for library initialization and message processing
 */

/**
 * @brief Initialize the TST library with device configuration
 * @param config Pointer to device configuration structure
 * @return TST_OK on success, error code on failure
 * 
 * This function must be called before using any other library functions.
 * It registers the device, its interfaces, and data structures.
 */
uint8_t tstInit(const TST_DeviceConfig *config);

/**
 * @section COMMUNICATION_FUNCTIONS Communication Functions
 * @brief Functions for sending and receiving data
 */

/**
 * @brief Main processing engine - call regularly to process messages
 * @return TST_OK on success, error code on failure
 * 
 * This function processes all pending received messages and generates
 * appropriate responses. Should be called frequently in main loop.
 */
uint8_t tstEngine(void);

/**
 * @brief Receive data from communication interface
 * @param device Device name
 * @param interface Interface name
 * @param data Received data buffer
 * @param size Size of received data
 * @return TST_OK on success, error code on failure
 * 
 * Call this function when data is received from a communication interface.
 * The data will be queued for processing by tstEngine().
 */
uint8_t tstRx(const char *device, const char *interface, void *data, size_t size);

/**
 * @brief Get data to transmit on communication interface
 * @param device Device name
 * @param interface Interface name
 * @param data Output buffer for data to transmit
 * @param size Input: buffer size, Output: actual data size
 * @return TST_OK if data available, error code otherwise
 * 
 * Call this function to get data that needs to be transmitted on the
 * communication interface. Returns TST_FAIL if no data is pending.
 */
uint8_t tstTx(const char *device, const char *interface, void *data, size_t *size);

/**
 * @section VARIABLE_FUNCTIONS Variable Synchronization Functions
 * @brief Functions for reading and writing variables across devices
 */

/**
 * @brief Request the current value of a variable from remote device
 * @param device Target device name
 * @param interface Interface to use for communication
 * @param pVar Pointer to local variable to update
 * @param sVar Size of variable in bytes
 * @return TST_OK on success, error code on failure
 * 
 * This function sends a request to read a variable from a remote device.
 * When the response is received, the local variable will be updated.
 */
uint8_t tstVariablesGet(const char *device, const char *interface, void *pVar, size_t sVar);

/**
 * @brief Set the value of a variable on a remote device
 * @param device Target device name
 * @param interface Interface to use for communication
 * @param pVar Pointer to variable structure location
 * @param sVar Size of variable in bytes
 * @param pData Pointer to new data to set
 * @return TST_OK on success, error code on failure
 * 
 * This function sends a request to update a variable on a remote device
 * with the provided data.
 */
uint8_t tstVariablesSet(const char *device, const char *interface, void *pVar, size_t sVar, void *pData);

/**
 * @section MONITOR_FUNCTIONS Monitoring and Logging Functions
 * @brief Functions for sending text messages and debug information
 */

/**
 * @brief Send a text message for monitoring/debugging
 * @param device Source device name
 * @param interface Interface to use for communication
 * @param message Text message to send
 * @return TST_OK on success, error code on failure
 * 
 * Use this function to send log messages, debug information, or status
 * updates. Messages are automatically fragmented if they exceed the
 * interface maximum payload size.
 */
uint8_t tstMonitorSend(const char *device, const char *interface, const char *message);

/**
 * @section FILESYSTEM_FUNCTIONS File System Functions
 * @brief Functions for file system operations across devices
 */

/**
 * @brief List contents of a directory
 * @param device Target device name
 * @param interface Interface to use
 * @param path Directory path to list
 * @return TST_OK on success, error code on failure
 */
uint8_t tstFsList(const char *device, const char *interface, const char *path);

/**
 * @brief Upload file data to remote device
 * @param device Target device name
 * @param interface Interface to use
 * @param path Target file path on remote device
 * @param data File data to upload
 * @param offset Offset within file to start writing
 * @param size Size of data to upload
 * @return TST_OK on success, error code on failure
 */
uint8_t tstFsUpload(const char *device, const char *interface, const char *path, const void *data, uint32_t offset, uint32_t size);

/**
 * @brief Download file data from remote device
 * @param device Target device name
 * @param interface Interface to use
 * @param path Source file path on remote device
 * @param offset Offset within file to start reading
 * @param size Size of data to download (0 = entire file)
 * @return TST_OK on success, error code on failure
 */
uint8_t tstFsDownload(const char *device, const char *interface, const char *path, uint32_t offset, uint32_t size);

/**
 * @brief Delete file on remote device
 * @param device Target device name
 * @param interface Interface to use
 * @param path File path to delete
 * @return TST_OK on success, error code on failure
 */
uint8_t tstFsDelete(const char *device, const char *interface, const char *path);

/**
 * @section FS_CALLBACKS File System Callback Types
 * @brief Callback function types for implementing file system operations
 */

/**
 * @brief Callback for directory listing operations
 * @param path Directory path to list
 * @param outData Output: allocated buffer containing directory listing
 * @param outSize Output: size of directory listing data
 * @return TST_OK on success, error code on failure
 */
typedef uint8_t (*TstFsListFunc)(const char *path, uint8_t **outData, uint32_t *outSize);

/**
 * @brief Callback for file upload operations
 * @param path Target file path
 * @param data File data to write
 * @param offset Offset within file
 * @param size Size of data
 * @return TST_OK on success, error code on failure
 */
typedef uint8_t (*TstFsUploadFunc)(const char *path, const uint8_t *data, uint32_t offset, uint32_t size);

/**
 * @brief Callback for file download operations
 * @param path Source file path
 * @param offset Offset within file
 * @param requestedSize Size of data requested
 * @param outData Output: allocated buffer containing file data
 * @param outSize Output: actual size of file data
 * @return TST_OK on success, error code on failure
 */
typedef uint8_t (*TstFsDownloadFunc)(const char *path, uint32_t offset, uint32_t requestedSize, uint8_t **outData, uint32_t *outSize);

/**
 * @brief Callback for file deletion operations
 * @param path File path to delete
 * @return TST_OK on success, error code on failure
 */
typedef uint8_t (*TstFsDeleteFunc)(const char *path);

/**
 * @brief Register file system operation callbacks
 * @param listFunc Callback for directory listing
 * @param uploadFunc Callback for file uploads
 * @param downloadFunc Callback for file downloads
 * @param deleteFunc Callback for file deletion
 * 
 * Register these callbacks to handle file system operations on the local device.
 * Set any callback to NULL to disable that operation.
 */
void tstRegisterFsHandlers(TstFsListFunc listFunc, TstFsUploadFunc uploadFunc, 
                        TstFsDownloadFunc downloadFunc, TstFsDeleteFunc deleteFunc);

/**
 * @section UPDATE_FUNCTIONS Firmware Update Functions
 * @brief Functions for over-the-air firmware updates
 */

/**
 * @brief Callback for firmware update operations
 * @param data Firmware data chunk (NULL for START/END operations)
 * @param size Size of firmware data or total size for START operation
 * @param sequenceNumber Sequence number for packet ordering
 * @param crc CRC32 checksum of the data
 * @param operation Update operation type (START, DATA, END)
 * @return TST_OK on success, error code on failure
 */
typedef uint8_t (*TstUpdateFunc)(const uint8_t *data, uint32_t size, uint32_t sequenceNumber, uint32_t crc, uint8_t operation);

/**
 * @brief Register firmware update callback
 * @param updateFunc Callback function to handle firmware updates
 * 
 * Register this callback to handle firmware update operations.
 * The callback will be called for START, DATA, and END operations.
 */
void tstRegisterUpdateHandler(TstUpdateFunc updateFunc);

/**
 * @brief Process firmware update request (internal function)
 * @param iface Interface receiving the request
 * @param deviceId Source device ID
 * @param req Update request message
 * @param reqSize Size of request message
 * @return TST_OK on success, error code on failure
 */
uint8_t tstProcessUpdateRequest(InterfaceNode *iface, uint16_t deviceId, tstMessageUpdateRequest *req, size_t reqSize);

/**
 * @brief Send firmware update response
 * @param device Source device name
 * @param interface Interface to use
 * @param operation Update operation being acknowledged
 * @param status Status code (TST_OK or error)
 * @param sequenceNumber Sequence number being acknowledged
 * @param crc CRC32 of received data
 * @return TST_OK on success, error code on failure
 */
uint8_t tstUpdateResponseSend(const char *device, const char *interface, uint8_t operation, uint8_t status, uint32_t sequenceNumber, uint32_t crc);

/**
 * @section UTILITY_FUNCTIONS_2 Additional Utility Functions
 * @brief CRC calculation and data structure management functions
 */

/**
 * @brief Calculate CRC32 checksum
 * @param data Data buffer to checksum
 * @param size Size of data buffer
 * @return 32-bit CRC checksum
 */
uint32_t tstCRC32(uint8_t *data, size_t size);

/**
 * @section LIST_FUNCTIONS List Management Functions
 * @brief Functions for managing linked lists used in message queues
 */

/**
 * @brief Push data to the end of a linked list
 * @param head_ref Pointer to head pointer of the list
 * @param pData Pointer to data to be added
 * @param sData Size of data in bytes
 * @return TST_OK on success, error code on failure
 */
uint8_t tstListPush(struct TSTList **head_ref, uint8_t *pData, size_t sData);

/**
 * @brief Get and remove the first element from a linked list
 * @param head_ref Pointer to head pointer of the list
 * @param pData Output: pointer to retrieved data
 * @param sData Output: size of retrieved data
 * @return TST_OK on success, error code on failure
 */
uint8_t tstListGet(struct TSTList **head_ref, uint8_t **pData, size_t *sData);

/**
 * @section MAP_FUNCTIONS Hash Map Management Functions
 * @brief Functions for managing hash maps used for device/interface/structure lookup
 */

/**
 * @brief Create a new hash map
 * @param map Output: pointer to created map
 * @param size Number of buckets in the hash table
 * @return TST_OK on success, error code on failure
 */
uint8_t tstMapCreate(struct TSTMap **map, uint32_t size);

/**
 * @brief Destroy a hash map and free all memory
 * @param map Map to destroy
 * @return TST_OK on success, error code on failure
 */
uint8_t tstMapDestroy(struct TSTMap *map);

/**
 * @brief Insert data into hash map with dynamic resizing
 * @param mapHandle Pointer to map pointer (may be updated if resized)
 * @param id Hash ID for the data
 * @param data Pointer to data to insert
 * @param dataSize Size of data in bytes
 * @return TST_OK on success, error code on failure
 */
uint8_t tstMapInsertDynamic(struct TSTMap **mapHandle, uint32_t id, void *data, size_t dataSize);

/**
 * @brief Search for data in hash map by ID
 * @param map Map to search in
 * @param id Hash ID to search for
 * @param data Output: pointer to found data
 * @return TST_OK if found, error code otherwise
 */
uint8_t tstMapSearch(struct TSTMap *map, uint32_t id, void **data);

/**
 * @brief Delete entry from hash map by ID
 * @param map Map to delete from
 * @param id Hash ID to delete
 * @return TST_OK on success, error code on failure
 */
uint8_t tstMapDelete(struct TSTMap *map, uint32_t id);

/**
 * @section INTERNAL_FUNCTIONS Internal Helper Functions
 * @brief Internal functions used by the library (exposed for advanced usage)
 */

/**
 * @brief Process file system request and send response
 * @param iface Interface receiving the request
 * @param deviceId Source device ID
 * @param req File system request message
 * @param reqSize Size of request message
 * @return TST_OK on success, error code on failure
 */
uint8_t tstFsProcessRequest(InterfaceNode *iface, uint16_t deviceId, tstMessageFsRequest *req, size_t reqSize);

/**
 * @section TIMER_FUNCTIONS Timer Management Functions
 * @brief Comprehensive timer system for scheduling and time-based operations
 */

/**
 * @brief Initialize the timer system
 * @return TST_OK on success, error code on failure
 * 
 * Must be called before using any timer functions.
 * Creates internal data structures for timer management.
 */
uint8_t tstTimerInit(void);

/**
 * @brief Clean up timer system and free resources
 * @return TST_OK on success, error code on failure
 * 
 * Call this function to clean up all timers and free memory.
 * All active timers will be stopped and removed.
 */
uint8_t tstTimerCleanup(void);

/**
 * @brief Process timer updates - call regularly with elapsed time
 * @param elapsedMs Milliseconds elapsed since last call
 * @return TST_OK on success, error code on failure
 * 
 * This function must be called regularly (e.g., in main loop) to update
 * timer states and trigger callbacks for expired timers.
 */
uint8_t tstTimerLoop(uint32_t elapsedMs);

/**
 * @section TIMER_CREATION Timer Creation Functions
 * @brief Functions for creating and starting timers
 */

/**
 * @brief Start a one-shot timer
 * @param timerName Unique name for the timer
 * @param durationMs Timer duration in milliseconds
 * @return TST_OK on success, error code on failure
 * 
 * Creates and starts a timer that will expire once after the specified duration.
 * The timer is automatically removed when it expires.
 */
uint8_t tstStartTimer(const char* timerName, uint32_t durationMs);

/**
 * @brief Start a one-shot timer with callback
 * @param timerName Unique name for the timer
 * @param durationMs Timer duration in milliseconds
 * @param callback Function to call when timer expires
 * @param userData User data to pass to callback
 * @return TST_OK on success, error code on failure
 */
uint8_t tstStartTimerWithCallback(const char* timerName, uint32_t durationMs, TSTTimerCallback callback, void* userData);

/**
 * @brief Start a periodic timer
 * @param timerName Unique name for the timer
 * @param intervalMs Timer interval in milliseconds
 * @return TST_OK on success, error code on failure
 * 
 * Creates and starts a timer that repeats at the specified interval
 * until explicitly stopped.
 */
uint8_t tstStartPeriodicTimer(const char* timerName, uint32_t intervalMs);

/**
 * @brief Start a periodic timer with callback
 * @param timerName Unique name for the timer
 * @param intervalMs Timer interval in milliseconds
 * @param callback Function to call each time timer expires
 * @param userData User data to pass to callback
 * @return TST_OK on success, error code on failure
 */
uint8_t tstStartPeriodicTimerWithCallback(const char* timerName, uint32_t intervalMs, TSTTimerCallback callback, void* userData);

/**
 * @section TIMER_CONTROL Timer Control Functions
 * @brief Functions for controlling existing timers
 */

/**
 * @brief Stop and remove a timer
 * @param timerName Name of timer to stop
 * @return TST_OK on success, TST_FAIL_NOT_FOUND if timer doesn't exist
 */
uint8_t tstStopTimer(const char* timerName);

/**
 * @brief Pause a running timer
 * @param timerName Name of timer to pause
 * @return TST_OK on success, TST_FAIL_NOT_FOUND if timer doesn't exist
 * 
 * Paused timers can be resumed with tstResumeTimer().
 */
uint8_t tstPauseTimer(const char* timerName);

/**
 * @brief Resume a paused timer
 * @param timerName Name of timer to resume
 * @return TST_OK on success, TST_FAIL_NOT_FOUND if timer doesn't exist
 */
uint8_t tstResumeTimer(const char* timerName);

/**
 * @brief Reset a timer to its initial duration
 * @param timerName Name of timer to reset
 * @param newDurationMs New duration (0 to keep current duration)
 * @return TST_OK on success, TST_FAIL_NOT_FOUND if timer doesn't exist
 * 
 * Resets the timer countdown and optionally changes the duration.
 * Timer will be set to active state.
 */
uint8_t tstResetTimer(const char* timerName, uint32_t newDurationMs);

/**
 * @section TIMER_QUERY Timer Query Functions
 * @brief Functions for checking timer status and information
 */

/**
 * @brief Check if a timer exists
 * @param timerName Name of timer to check
 * @return true if timer exists, false otherwise
 */
bool tstTimerExists(const char* timerName);

/**
 * @brief Check if a timer has expired
 * @param timerName Name of timer to check
 * @return true if timer has expired, false otherwise
 * 
 * For one-shot timers, this returns true once the timer expires.
 * For periodic timers, this is momentarily true each time it expires.
 */
bool tstTimerExpired(const char* timerName);

/**
 * @brief Get remaining time for a timer
 * @param timerName Name of timer to query
 * @return Remaining time in milliseconds, 0 if timer doesn't exist or has expired
 */
uint32_t tstGetRemainingTime(const char* timerName);

/**
 * @brief Get current state of a timer
 * @param timerName Name of timer to query
 * @return Timer state (see TSTTimerState enum)
 */
TSTTimerState tstGetTimerState(const char* timerName);

/**
 * @brief Get count of currently active timers
 * @return Number of active timers in the system
 */
uint32_t tstGetActiveTimerCount(void);

#ifdef __cplusplus
}
#endif

#endif // TSTLIBRARY_H